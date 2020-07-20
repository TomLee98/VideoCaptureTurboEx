/******************************===== _CAPTUROR_HELPER_H =====*********************************/

/*********************************************************************************************/
#pragma once
#ifndef _CAPTUROR_HELPER_H
#define _CAPTUROR_HELPER_H
#include<iostream>
#include<string>
#include<opencv.hpp>
#include"sysinfo.h"
#include<atlstr.h>

constexpr auto ZIP_IMG_SIZE_EXCEPT = 27.25;	//KB
constexpr auto ZIP_IMG_SIZE_STDEV = 2.45;	//KB
constexpr auto IMG_SIZE_EXCEPT = 640 * 480 * 3 / 1024;	//KB
constexpr auto NORMAL_FPS_COEFF = 2.0;
constexpr auto TURBO_FPS_OFFSET = -2.0;
constexpr auto MAX_CAMERA_COUNT = 10U;

using namespace std;

//定义捕获器运行模式
typedef enum class _CAPTURE_MODE { Task_Invalid = -1, Turbo_Off, Turbo_On }CaptureMode;

//定义捕获器运行参数
typedef struct _RUNNING_PARAMETERS_
{
	string mainPath;		//数据写入主目录
	double fps;				//数据保存帧率
	double recordTime;		//数据记录时长,秒
	unsigned cameraCount;	//当前设备摄像头数量
	CaptureMode captureMode;//捕获器运行模式
}RunningParams;

class CapturorHelper
{
public:
	//
	CapturorHelper(
		const string&	mainPath, 
		const double&	fps, 
		const double&	recordTime, 
		const unsigned& maxCameraCount = MAX_CAMERA_COUNT
	);

	//获取捕获器运行参数
	const RunningParams GetRunningParams()const {
		return this->runParam;
	}

	~CapturorHelper() {}

private:
	/******************* private member variables *********************/
	unsigned maxFrameCount;		//最大可记录帧数(压缩后)
	double normalStableFPS;		//Normal稳定记录模式下帧率
	unsigned cameraCount;		//当前设备摄像头数量
	double turboStableFPS;		//启用Turbo模式下稳定帧率
	bool isUserUseTurbo;		//标记用户是否启用Turbo模式
	bool isUserFixedFPS;		//标记用户是否同意调整FPS
	SysInfo sysInfo;			//记录系统信息
	RunningParams runParam;		//捕获器最佳运行参数

	/******************* private member functions *********************/
	//获取当前设备连接的摄像头数目
	const unsigned GetCameraCount(unsigned maxCameraNumber);

	//获取用户对Turbo的请求
	void AskUserForTurbo();

	//建议用户将请求帧率调整为更低值
	void AdviseUserForFPS();

	//计算获得Capture运行模式（包括与用户交互）
	void GetCaptureRunMode(
		const string&	mainPath,
		const double&	fps,
		const double&	recordTime
	);
};

#endif // !_CAPTUROR_HELPER_H
