// MtPerfTestInC.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include<Windows.h>
#include "psapi.h"

#define DATA_LENGTH 1 * 1024 * 1024 * 1024 

//WORD numberCounts[10] = { 0 };


// Sample custom data structure for threads to use.
// This is passed by void pointer so it can be any data type
// that can be passed using a single void pointer (LPVOID).
typedef struct MyData {
    DWORD beginIndex;
    DWORD counts;
    BYTE* numbers;
    INT numberCounts[10] = { 0 };
} MYDATA, * PMYDATA;

DWORD WINAPI MyThreadFunc(LPVOID lpParam)
{
    PMYDATA data = (PMYDATA)lpParam;

    DWORD i = data->beginIndex;
    DWORD end = data->beginIndex + data->counts;

    DWORD dwWaitResult;

    while (i < end)
    {

        BYTE number = *(data->numbers + i);

        data->numberCounts[number] = (data->numberCounts[number] + 1);

        i++;
    }

    return 0;
}

DWORD LoadData(const TCHAR* fileName, LPVOID buffer)
{
    HANDLE hFile;
    DWORD dwNumberOfBytesRead = 0;

    hFile = CreateFile(fileName,    // file to open
        GENERIC_READ,               // open for reading
        FILE_SHARE_READ,            // share for reading
        NULL,                       // default security
        OPEN_EXISTING,              // existing file only
        FILE_ATTRIBUTE_NORMAL,      // normal file
        NULL);                      // no attr. template

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return ERROR;
    }

    if (FALSE == ReadFile(hFile, buffer, DATA_LENGTH, &dwNumberOfBytesRead, NULL))
    {
        printf("ReadFile Error!");
        printf("Terminal failure: Unable to read from file.\n GetLastError=%08x\n", GetLastError());

        CloseHandle(hFile);

        return ERROR;
    }

    CloseHandle(hFile);

    return dwNumberOfBytesRead;
}

VOID ProcessData(BYTE* Data, INT MaxThreads)
{
    PMYDATA* pDataArray = (PMYDATA*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(MYDATA) * MaxThreads);
    HANDLE* hThreadArray = (HANDLE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(HANDLE) * MaxThreads);
    DWORD* dwThreadIdArray = (DWORD*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DWORD) * MaxThreads);

    DWORD partLength = (DATA_LENGTH) / (MaxThreads);
    INT result[10] = { 0 };

    INT repeat = 100;

    /*******************Execution Time Performance**********/
    LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroSeconds;
    LARGE_INTEGER Frequency;
    ElapsedMicroSeconds.QuadPart = 0;
    /*******************************************************/

    /******************CPU Performance*********************/
    DOUBLE cpuVal=0;
    ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
    int numProcessors;
    HANDLE self;
    SYSTEM_INFO sysInfo;
    FILETIME ftime, fsys, fuser;

    GetSystemInfo(&sysInfo);
    numProcessors = sysInfo.dwNumberOfProcessors;

    GetSystemTimeAsFileTime(&ftime);
    memcpy(&lastCPU, &ftime, sizeof(FILETIME));

    self = GetCurrentProcess();
    GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
    memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
    memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
    //*****************************************************

    /***********************MEM Performance***************/
    SIZE_T memVal = 0;
    SIZE_T lastMemVal = 0;
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(self, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    lastMemVal = pmc.PrivateUsage;
    /*****************************************************/

    for (INT r = 0; r < repeat; r++)
    {
        for (int j = 0; j < 10; j++)
            result[j] = 0;

        QueryPerformanceFrequency(&Frequency);
        QueryPerformanceCounter(&StartingTime);

        for (int i = 0; i < MaxThreads; i++)
        {
            pDataArray[i] = (PMYDATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(MYDATA));

            if (pDataArray[i] == NULL)
            {
                // If the array allocation fails, the system is out of memory
                // so there is no point in trying to print an error message.
                // Just terminate execution.
                ExitProcess(2);
            }

            // Generate unique data for each thread to work with.

            pDataArray[i]->beginIndex = i * partLength;
            pDataArray[i]->counts = partLength;
            pDataArray[i]->numbers = Data;

            // Create the thread to begin execution on its own.
            hThreadArray[i] = CreateThread(
                NULL,                   // default security attributes
                0,                      // use default stack size  
                MyThreadFunc,           // thread function name
                pDataArray[i],          // argument to thread function 
                0,                      // use default creation flags 
                &dwThreadIdArray[i]);   // returns the thread identifier 


            // Check the return value for success.
            // If CreateThread fails, terminate execution. 
            // This will automatically clean up threads and memory. 

            if (hThreadArray[i] == NULL)
            {
                printf("CreateThread Error");
                ExitProcess(3);
            }
        }

        // Wait until all threads have terminated.
        WaitForMultipleObjects(MaxThreads, hThreadArray, TRUE, INFINITE);


        // Close all thread handles and free memory allocations.
        for (int i = 0; i < MaxThreads; i++)
        {
            for (int j = 0; j < 10; j++)
                result[j] += pDataArray[i]->numberCounts[j];

            CloseHandle(hThreadArray[i]);

            HeapFree(GetProcessHeap(), 0, pDataArray[i]);
            pDataArray[i] = NULL;    // Ensure address is not reused.
        }

        /**************CPU Performance*************/
        FILETIME ftime, fsys, fuser;
        ULARGE_INTEGER now, sys, user;
        double percent;

        GetSystemTimeAsFileTime(&ftime);
        memcpy(&now, &ftime, sizeof(FILETIME));

        GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
        memcpy(&sys, &fsys, sizeof(FILETIME));
        memcpy(&user, &fuser, sizeof(FILETIME));
        percent = (sys.QuadPart - lastSysCPU.QuadPart) +
            (user.QuadPart - lastUserCPU.QuadPart);
        percent /= (now.QuadPart - lastCPU.QuadPart);
        percent /= numProcessors;
        lastCPU = now;
        lastUserCPU = user;
        lastSysCPU = sys;

        cpuVal += (percent * 100.0);
        /****************************************/

        /*************MEM Performance************/
        GetProcessMemoryInfo(self, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
        memVal += pmc.PrivateUsage;
        lastMemVal = pmc.PrivateUsage;
        /****************************************/

        /***********Execution Time Performance*****/
        QueryPerformanceCounter(&EndingTime);
        ElapsedMicroSeconds.QuadPart += EndingTime.QuadPart - StartingTime.QuadPart;
        /******************************************/
    }

    ElapsedMicroSeconds.QuadPart /= repeat;
    cpuVal /= repeat;
    memVal /= repeat;

    HeapFree(GetProcessHeap(), 0, pDataArray);
    HeapFree(GetProcessHeap(), 0, hThreadArray);
    HeapFree(GetProcessHeap(), 0, dwThreadIdArray);


    printf("Threads:%d Elapsed Time: %lf Cpu Usage: %lf Mem Usage: %lf\n", MaxThreads, ElapsedMicroSeconds.QuadPart / (DOUBLE)Frequency.QuadPart, cpuVal, memVal/(DOUBLE)(1024*1024));

    for (int i = 0; i < 10; i++)
    {
        printf("Number %d : %d\n", i, result[i]);
    }
}

int main()
{
    BYTE* data = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, DATA_LENGTH);
    DWORD numberOfBytesReaded;

    numberOfBytesReaded = LoadData(TEXT("d:/data.dat"), data);

    if (numberOfBytesReaded == ERROR)
    {
        HeapFree(GetProcessHeap(), 0, data);
        return -1;
    }

    for (INT i = 1; i <= 32; i *= 2)
    {
        ProcessData(data, i);
    }

    HeapFree(GetProcessHeap(), 0, data);

    getchar();

    return 0;
}

