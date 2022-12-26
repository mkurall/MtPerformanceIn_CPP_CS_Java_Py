import time
from threading import Thread
from time import perf_counter
import psutil
import os

data_length = 1 * 1024 * 1024 * 1024    #1GB Data

class my_data():
    begin_index = 0
    length = 0
    buffer = []
    results = []

def thread_func(data):
    for i in data.buffer[data.begin_index:data.begin_index+data.length]:
        data.results[i] += 1

def process_data(buffer, max_threads):
    threadList = []
    dataList = []
    results = [0]*10
    part_length = data_length//max_threads


    my_process = psutil.Process(os.getpid())

    prev_cpu_time = time.process_time_ns()
    prev_up_time = time.time_ns()

    cpu_usage = 0
    elapsed_time = 0;

    repeat = 100
    for r in range(repeat):
        start_time = perf_counter()
        results = [0] * 10
        dataList.clear()
        threadList.clear()

        for i in range(max_threads):
            data = my_data()
            data.begin_index = i * part_length
            data.length = part_length
            data.buffer = buffer
            data.results = [0] * 10
            thread = Thread(target=thread_func, args=(data,))
            dataList.append(data)
            threadList.append(thread)
            thread.start()

        for thread in threadList:
            thread.join()

        for data in dataList:
            for i in range(10):
                results[i] += data.results[i]

        end_time = perf_counter()
        cpu_time = time.process_time_ns()
        up_time = time.time_ns()
        cpu_usage += ((cpu_time - prev_cpu_time) / ((up_time - prev_up_time) * psutil.cpu_count())) * 100
        elapsed_time += (end_time - start_time)
        prev_up_time = up_time
        prev_cpu_time = cpu_time

    elapsed_time/=repeat
    cpu_usage/=repeat


    print("Threads:", max_threads, " Elapsed Time:", elapsed_time," Cpu Usage:", cpu_usage," Mem Usage:",my_process.memory_info().rss/(1024*1024))

    for i in range(10):
        print("Number", i, ":", results[i])

if __name__ == "__main__":

    file_path = "d:/data1.dat"
    with open(file_path, "rb") as file:
        buffer = file.read()

    i = 1
    while (i <= 32):
        process_data(buffer, i)
        i *= 2

    input()
