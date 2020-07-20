/*********************************===== _CAPTUROR_H =====*************************************/

/*********************************************************************************************/
#pragma once
#ifndef _CAPTUROR_H
#define _CAPTUROR_H
#include<iostream>
#include<string>
#include<vector>
#include<queue>
#include<opencv.hpp>
#include"CapturorHelper.h"

constexpr auto RESERVE_RAM_RATIO = 0.5;
constexpr auto BUFFER_FILENAME = "RawImg.bin";

using namespace std;
using namespace cv;

class Capturor
{
public:
	//手动构造Capturor
	Capturor(
		const string&	mainPath, 
		const double&	fps, 
		const double&	recordTime, 
		unsigned		cameraCount = 6u,
		CaptureMode		captureMode = CaptureMode::Turbo_Off
	);
	//使用Helper提供的参数自动构造Capturor
	Capturor(const RunningParams& params) :
		Capturor(
			params.mainPath, 
			params.fps, 
			params.recordTime, 
			params.cameraCount, 
			params.captureMode
		){}

	//主进程入口点
	const bool Run() {
		if (CaptureMode::Task_Invalid == this->mode)
			return false;
		return CaptureMode::Turbo_On == this->mode ?
			RunWithTurboOn() : RunWithTurboOff();
	}

	//设置JPEG文件质量
	void SetJPEGQuality(const int quality) { this->jpegQuality = quality; }

	//保存FPS序列
	const bool SaveFpsSeries(const char* fpsFileName);

	//获取平均帧率
	double GetAverageFPS() { return this->frameN * 1.0 / this->recordTime; }

	//此函数用于清空捕获器对应的摄像头缓冲区
	static void CleanBuffer(VideoCapture* VCs, unsigned count, bool* isRunning);

	//
	~Capturor();

private:
	/******************* private member variables *********************/
	unsigned cameraCount;		//当前已连接摄像头数目
	double fps;					//目标帧率
	double recordTime;			//目标记录时长,秒
	unsigned frameN;			//当前已记录帧数
	int jpegQuality;			//jpg格式文件压缩质量 \in[0,100]
	double dfFrequency;			//cpu时钟频率
	bool runningBufferCleaner;	//启动缓冲清理器标记
	string saveDisc;			//文件保存盘符
	vector<string> savePath;	//每个摄像头对应的文件保存路径向量
	VideoCapture* VCs;			//摄像头对应的捕获器组
	//Mat* frames;				//保存每个摄像头的一帧序列
	CaptureMode mode;			//当前捕获器运行模式

	double* vFPS;				//实际帧率序列
	unsigned vFPS_len;			//实际帧率序列记录大小

	/******************* private member functions *********************/
	
	//以CaptureTurbo运行捕获器
	const bool RunWithTurboOn();

	//以Normal模式运行捕获器
	const bool RunWithTurboOff();

	//将已写入文件以固定格式重写入磁盘
	void ImageRewriter(const vector<int>& imgFormat);

};

#endif // !_CAPTUROR_H
