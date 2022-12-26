/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Main.java to edit this template
 */
package main;

import com.sun.management.OperatingSystemMXBean;
import java.io.IOException;
import java.lang.management.ManagementFactory;
import java.lang.management.RuntimeMXBean;
import java.nio.file.Files;
import java.nio.file.Paths;



/**
 *
 * @author Administrator
 */
public class main {

    static int dataLength = 1 * 1024 * 1024 * 1024;//1GB Data
    
    public static class MyData
    {
        public long beginIndex;
        public long counts;
        public byte[] numbers;
        public int[] results;
    }
    
    public static class ThreadFunc implements Runnable {
        private MyData _data;
        
        ThreadFunc(MyData data)
        {
            _data = data;
        }

        @Override
        public void run() {

            long i = _data.beginIndex;
            long end = _data.beginIndex + _data.counts;
            
            while (i < end)
            {
                byte number = _data.numbers[(int)i];

                _data.results[number] = (_data.results[number] + 1);

                i++;
            }
        }
    }
    
    private static void ProcessData(byte[] buffer, int maxThreads)
    {
        int[] result = new int[10];

        long partLength = (dataLength) / (maxThreads);

        /***Execution Time Performance***/
        long startingTime = 0;
        long endingTime = 0;
        long elapsedTime = 0;
        /*******************************/
        
        long repeat = 10;

        Thread[] threads = new Thread[maxThreads];
        MyData[] dataList = new MyData[maxThreads];
        
        /*CPU Performance*/
        double cpuUsage = 0;
        OperatingSystemMXBean bean = (com.sun.management.OperatingSystemMXBean) ManagementFactory.getOperatingSystemMXBean();
        RuntimeMXBean runtimeMXBean = ManagementFactory.getRuntimeMXBean();
        int availableProcessors = bean.getAvailableProcessors();
        long prevUpTime = runtimeMXBean.getUptime();
        long prevCpuTime = bean.getProcessCpuTime();
        /**************/
        
        /*MEM Performance*/
        Runtime rt = Runtime.getRuntime();
        long memUsage = 0;
        /****************************/
        
        for (int r = 0; r < repeat; r++)
        {
            startingTime = System.nanoTime();

            for (int i = 0; i < 10; i++)
                result[i] = 0;

            for (int i = 0; i < maxThreads; i++)
            {
                MyData data = new MyData();
                data.beginIndex = i * partLength;
                data.counts = partLength;
                data.numbers = buffer;
                data.results = new int[10];

                dataList[i] = data;

                threads[i] = new Thread(new ThreadFunc(data));

                threads[i].start();
            }

            for (int i = 0; i < maxThreads; i++)
            {
                try
                {
                    threads[i].join();
                }
                catch(InterruptedException ex)
                {
                }
            }

            for (int i = 0; i < maxThreads; i++)
            {
                for (int j = 0; j < 10; j++)
                    result[j] += dataList[i].results[j];
            }
 
            /*CPU Performance*/
            long cpuTime = bean.getProcessCpuTime();
            long upTime = runtimeMXBean.getUptime();
            long elapsedCpuTime = cpuTime - prevCpuTime;
            long elapsedUpTime = upTime - prevUpTime;
    
            cpuUsage += (elapsedCpuTime  / (elapsedUpTime * 10000F * availableProcessors));
            prevCpuTime = cpuTime;
            prevUpTime = upTime;
            /******************************/
            
            /*MEM Performance*/
            memUsage += (rt.totalMemory()-rt.freeMemory());
            /****************************/
            
            /***Execution Time Performance***/
            endingTime = System.nanoTime();
            elapsedTime += (endingTime - startingTime);
            /*******************************/
        }        

        elapsedTime /= repeat;
        cpuUsage /= repeat;
        memUsage /=repeat;
        
        System.out.println(String.format("Threads: %d ElapsedTime : %f CPU Usage: %f Mem Usage: %f", maxThreads,elapsedTime/(double)1000000000,cpuUsage,memUsage/(double)(1024*1024)));

        for (int i = 0; i < 10; i++)
        {
            System.out.println(String.format("Number %d : %d", i, result[i]));
        }
    }
    
    /**
     * @param args the command line arguments
     */
    public static void main(String[] args){

        String filePath = "d:/data.dat";

        try
        {
            byte[] buffer = Files.readAllBytes(Paths.get(filePath));
            
            for (int i = 1; i <= 32; i*=2)
            {
                ProcessData(buffer, i);
            }
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }
    }
}


