using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;

namespace CSharpPerf
{
    internal class Program
    {
        static int dataLength = 1 * 1024 * 1024 * 1024;//1GB Data
        struct MyData
        {
            public long beginIndex;
            public long counts;
            public byte[] numbers;
            public int[] results;
        };

        static void MyThreadFunc(object p)
        {
            MyData data = (MyData)p;

            long i = data.beginIndex;
            long end = data.beginIndex + data.counts;

            while (i < end)
            {

                byte number = data.numbers[i];

                data.results[number] = (data.results[number] + 1);

                i++;
            }
        }

        static void ProcessData(byte[] buffer, int maxThreads)
        {
            int[] result = new int[10];

            long partLength = (dataLength) / (maxThreads);

            long startingTime = 0;
            long endingTime = 0;
            long elapsedTime = 0;

            long repeat = 100;
            double cpuUsage = 0;
            double ramUsage = 0;

            for (int r = 0; r < repeat; r++)
            {
                startingTime = Stopwatch.GetTimestamp();
                PerformanceCounter cpuCounter = new PerformanceCounter("Process", "% Processor Time", Process.GetCurrentProcess().ProcessName /*"_Total"*/);
                PerformanceCounter ramCounter = new PerformanceCounter("Process", "Private Bytes", Process.GetCurrentProcess().ProcessName, true);
                cpuCounter.NextValue();

                List<Thread> threads = new List<Thread>();
                List<MyData> dataList = new List<MyData>();


                for (int i = 0; i < 10; i++)
                    result[i] = 0;

                for (int i = 0; i < maxThreads; i++)
                {
                    MyData data = new MyData();
                    data.beginIndex = i * partLength;
                    data.counts = partLength;
                    data.numbers = buffer;
                    data.results = new int[10];

                    dataList.Add(data);


                    threads.Add(new Thread(new ParameterizedThreadStart(MyThreadFunc)));

                    threads[i].Start(data);
                }

                for (int i = 0; i < maxThreads; i++)
                {
                    threads[i].Join();
                }

                for (int i = 0; i < maxThreads; i++)
                {
                    for (int j = 0; j < 10; j++)
                        result[j] += dataList[i].results[j];
                }

                endingTime = Stopwatch.GetTimestamp();
                
                float cpuVal = cpuCounter.NextValue() / Environment.ProcessorCount;
                float ramVal = ramCounter.NextValue();
                cpuUsage += cpuVal;
                ramUsage += ramVal;

                elapsedTime += (endingTime - startingTime);
            }

            elapsedTime /= repeat;
            cpuUsage /= repeat;
            ramUsage /= repeat;

            Console.WriteLine(string.Format("Threads: {0} ElapsedTime {1} Cpu Usage {2} Ram Usage {3}", maxThreads, elapsedTime*(1.0/Stopwatch.Frequency), cpuUsage, ramUsage/(double)(1024*1024)));

            for (int i = 0; i < 10; i++)
            {
                Console.WriteLine(string.Format("Number {0} : {1}", i, result[i]));
            }
        }

        //Bu metodu veri seti oluşturmak için main içerisinde çağırabilirsiniz
        static void CreateData()
        {
            string filePath = "d://data.dat";
            
            byte[] buffer = new byte[dataLength];

            Random random = new Random();

            for (int i = 0; i< dataLength; i++)
            {
                buffer[i] = (byte)random.Next(10);
            }

            System.IO.File.WriteAllBytes(filePath, buffer);

            Console.WriteLine("Veri seti oluşturuldu...");
        }

        static void Main(string[] args)
        {
            string filePath = "d:/data.dat";

            byte[] buffer = System.IO.File.ReadAllBytes(filePath);

            for (int i = 1; i <= 32; i *= 2)
            {
                ProcessData(buffer, i);
            }

            Console.ReadKey();
        }
    }
}
