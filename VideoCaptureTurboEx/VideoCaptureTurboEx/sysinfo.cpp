#pragma comment(lib,"winmm.lib")
#include "sysinfo.h"
#include<opencv.hpp>
#include<ctime>
#include<sys/stat.h>
using namespace cv;
typedef BOOL(WINAPI* LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

//此函数用与根据输入盘符确定其可用空间
//disc: 待获取容量盘符
//code visit: https://blog.csdn.net/aixiaodeshushu/article/details/99541752
//copy right:CC 4.0 BY-SA
unsigned GetROMFreeSpace(wstring disc)
{
    bool findFlag = false, fResult = false;
    unsigned _int64 i64FreeBytesToCaller;
    unsigned _int64 i64TotalBytes;
    unsigned _int64 i64FreeBytes;
    //首先获取系统中存在的盘符
    vector<wstring> fixedDisc = GetDisc();
    for (size_t k = 0; k < fixedDisc.size(); k++) {
        if (fixedDisc[k] == disc) {
            findFlag = true;
            break;
        }
    }
    if (!findFlag) {
        //如果未找到要求盘符，直接报告无可用空间
        return 0u;
    }
    //
    fResult = GetDiskFreeSpaceEx(
        (LPCWSTR)disc.c_str(),
        (PULARGE_INTEGER)&i64FreeBytesToCaller,
        (PULARGE_INTEGER)&i64TotalBytes,
        (PULARGE_INTEGER)&i64FreeBytes);
    if (fResult) {
        return (unsigned)(i64FreeBytesToCaller / KBYTES);
    }
    else
        return 0u;   //磁盘未准备完成
}

//此函数用于获取当前系统中硬盘盘符
const vector<wstring> GetDisc()
{
    vector<wstring> v;
    //在驱动器已分配盘符中循环
    for (char id = 'A'; id <= 'Z'; id++) {
        
        WCHAR ws[5];
        char* prec = new char[4]{ id,':','\\','\0' };
        swprintf_s(ws, L"%hs", prec);
        //如果此盘符为固定驱动器
        if (DRIVE_FIXED == GetDriveTypeW(ws)) {
            v.push_back(ws);
        }
        delete[] prec;
    }
    return v;
}

//此函数用于获取当前系统可用内存
unsigned GetRAMFreeSpace()
{
    MEMORYSTATUSEX statusex;
    statusex.dwLength = sizeof(statusex);
    if (GlobalMemoryStatusEx(&statusex))
    {
        return (unsigned)(statusex.ullAvailPhys / KBYTES);
    }
    else {
        return 0u;
    }
}

//此函数用于获取cpu信息
//coreCount: CPU核心数量
//lcoreCount：CPU线程数量
//code visit:https://blog.csdn.net/tobacco5648/article/details/22201169
//copy right:CC 4.0 BY-SA
CPU_STATUS GetCPUInfo(unsigned& coreCount, unsigned& lcoreCount)
{
    LPFN_GLPI glpi;
    glpi = (LPFN_GLPI)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "GetLogicalProcessorInformation");
    if (NULL == glpi)
    {
        printf("GetLogicalProcessorInformation is not supported.\n");
    }
    BOOL done = FALSE;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
    DWORD returnLength = 0;
    while (!done)
    {
        DWORD rc = glpi(buffer, &returnLength);
        if (FALSE == rc)
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                if (buffer)
                    free(buffer);
                buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(returnLength);
                if (NULL == buffer)
                {
                    printf("Error: Allocation failure\n");
                    return (2);
                }
            }
            else
            {
                printf("Error %d\n", GetLastError());
                return (3);
            }
        }
        else
        {
            done = TRUE;
        }
    }
    ptr = buffer;
    DWORD byteOffset = 0;
    DWORD logicalProcessorCount = 0;
    DWORD processorCoreCount = 0;
    while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength)
    {
        switch (ptr->Relationship)
        {
        case RelationProcessorCore:
            processorCoreCount++;
            logicalProcessorCount += CountSetBits(ptr->ProcessorMask);
            break;
        }
        byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        ptr++;
    }
    coreCount = processorCoreCount;
    lcoreCount = logicalProcessorCount;
    return (0);
}

//code visit:https://blog.csdn.net/tobacco5648/article/details/22201169
//copy right:CC 4.0 BY-SA
DWORD CountSetBits(ULONG_PTR bitMask)
{
    DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
    DWORD bitSetCount = 0;
    ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;
    DWORD i;

    for (i = 0; i <= LSHIFT; ++i)
    {
        bitSetCount += ((bitMask & bitTest) ? 1 : 0);
        bitTest /= 2;
    }
    return bitSetCount;
}

//此函数用于获取当前进程对磁盘写入速率(取平均值),KB/s
//file_path: 测试写入路径
//checkT: 测试总时长,s
//jpg_quality: 用于测试写入的jpg文件质量
unsigned GetROMWriteRate(string filePath, bool isZip, unsigned checkT, int jpgQuality)
{
    unsigned frame = 0u;
    double runTime = 0., allFileSize = 0., freq = getTickFrequency();
    struct _stat info;
    Mat data;
    int64 ts, te;
    string fileName;
    jpgQuality > 100 || jpgQuality < 1 ? jpgQuality = 75 : NULL;
    vector<int> imgFormat = { IMWRITE_JPEG_QUALITY,jpgQuality };
    data.create(480, 640, CV_8UC3);

    while (runTime < checkT) {
        //首先生成用于写入的随机数据
        randu(data, Scalar::all(0), Scalar::all(256));
        //测量写入时间(返回毫秒数)
        ts = getTickCount();
        if (isZip) {
            //jpg 编码器压缩+写入
            fileName = filePath + "0.jpg";
            imwrite(fileName, data, imgFormat);
        }
        else {
            //bmp无压缩+写入
            fileName = filePath + "0.bmp";
            imwrite(fileName, data);
        }
        te = getTickCount();
        runTime += (te - ts) / freq; //转换为秒数
        //获取写入文件的大小
        _stat(fileName.c_str(), &info);
        allFileSize += info.st_size / 1024.0;   //转换为KB
        //清理写入的文件
        if (remove(fileName.c_str()))
            break;
    }

    return (unsigned)(allFileSize / runTime);
}

//获取系统相关参数
bool GetSysInfo(SysInfo* pSysInfo, wstring disc)
{
    if (pSysInfo == nullptr)
        return false;
    string str;
    str.assign(disc.begin(), disc.end());
    GetCPUInfo(pSysInfo->cpu_core_counts, pSysInfo->cpu_lcore_counts);
    pSysInfo->ram_free_space = GetRAMFreeSpace();
    pSysInfo->rom_free_space = GetROMFreeSpace(disc);
    pSysInfo->rom_write_rate = GetROMWriteRate(str, false);
    pSysInfo->rom_zipwrite_rate = GetROMWriteRate(str);
    return true;
}
