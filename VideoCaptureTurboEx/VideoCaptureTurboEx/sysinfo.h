/******************************===== _SYSTEM_INFO_H =====************************************/
/*此头文件封装WIN API 已获取系统参数(包括：当前可用内存容量、指定盘符可用容量、磁盘写入速率)*/
/*Version: 1.0.0                                                                            */
/*Author: Tom                                                                     */
/*Last Update: 2020/7/10																	*/	
/*																							*/
/*包含结构体: struct _SYS_INFO_{rom_free_space;ram_free_space;rom_zipwrite_rate;rom_write_rate}*/
/*可用内存获取调用 GlobalMemoryStatusEx <Windows.h>-->API									*/
/*指定盘符可用容量调用 GetDiskFreeSpaceEx <Windows.h>-->API									*/
/*磁盘写入速率 调用imwrite <opencv.hpp>-->opencv 4.3.0 API									*/
/********************************************************************************************/

#pragma once
#ifndef _SYSTEM_INFO_H
#define _SYSTEM_INFO_H
#include<Windows.h>
#include<iostream>
#include<vector>
#include<string>

#define  GBYTES  1073741824  
#define  MBYTES  1048576  
#define  KBYTES  1024  
#define  DKBYTES 1024.0  

using namespace std;

typedef int CPU_STATUS;

typedef struct _SYSTEM_INFO_
{
	unsigned rom_free_space;	//~static,KB
	unsigned ram_free_space;	//dynamic,KB
	unsigned rom_zipwrite_rate;	//KB/s
	unsigned rom_write_rate;	//KB/s
	unsigned cpu_core_counts;	//cpu核心数量
	unsigned cpu_lcore_counts;	//cpul逻辑核心数量
}SysInfo;

//输入盘符，计算其可用空间(KBytes)
unsigned GetROMFreeSpace(wstring disc = L"C:\\");

//获取盘符向量
const vector<wstring> GetDisc();

//获取系统当前可用内存(KBytes)
unsigned GetRAMFreeSpace();

//获取系统当前cpu信息（核心数、线程数）
CPU_STATUS GetCPUInfo(unsigned& coreCount, unsigned& lcoreCount);


DWORD CountSetBits(ULONG_PTR bitMask);

//获得磁盘写速率
unsigned GetROMWriteRate(string filePath, bool isZip = true, unsigned checkT = 1u, int jpgQuality = 95);

//直接获取相关的系统参数
bool GetSysInfo(SysInfo* pSysInfo, wstring disc = L"C:\\");

#endif // !_SYS_INFO_H
