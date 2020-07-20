/******************************===== _SYSTEM_INFO_H =====************************************/
/*��ͷ�ļ���װWIN API �ѻ�ȡϵͳ����(��������ǰ�����ڴ�������ָ���̷���������������д������)*/
/*Version: 1.0.0                                                                            */
/*Author: Tom                                                                     */
/*Last Update: 2020/7/10																	*/	
/*																							*/
/*�����ṹ��: struct _SYS_INFO_{rom_free_space;ram_free_space;rom_zipwrite_rate;rom_write_rate}*/
/*�����ڴ��ȡ���� GlobalMemoryStatusEx <Windows.h>-->API									*/
/*ָ���̷������������� GetDiskFreeSpaceEx <Windows.h>-->API									*/
/*����д������ ����imwrite <opencv.hpp>-->opencv 4.3.0 API									*/
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
	unsigned cpu_core_counts;	//cpu��������
	unsigned cpu_lcore_counts;	//cpul�߼���������
}SysInfo;

//�����̷�����������ÿռ�(KBytes)
unsigned GetROMFreeSpace(wstring disc = L"C:\\");

//��ȡ�̷�����
const vector<wstring> GetDisc();

//��ȡϵͳ��ǰ�����ڴ�(KBytes)
unsigned GetRAMFreeSpace();

//��ȡϵͳ��ǰcpu��Ϣ�����������߳�����
CPU_STATUS GetCPUInfo(unsigned& coreCount, unsigned& lcoreCount);


DWORD CountSetBits(ULONG_PTR bitMask);

//��ô���д����
unsigned GetROMWriteRate(string filePath, bool isZip = true, unsigned checkT = 1u, int jpgQuality = 95);

//ֱ�ӻ�ȡ��ص�ϵͳ����
bool GetSysInfo(SysInfo* pSysInfo, wstring disc = L"C:\\");

#endif // !_SYS_INFO_H
