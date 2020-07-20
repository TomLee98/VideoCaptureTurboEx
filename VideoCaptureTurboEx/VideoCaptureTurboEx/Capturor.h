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
	//�ֶ�����Capturor
	Capturor(
		const string&	mainPath, 
		const double&	fps, 
		const double&	recordTime, 
		unsigned		cameraCount = 6u,
		CaptureMode		captureMode = CaptureMode::Turbo_Off
	);
	//ʹ��Helper�ṩ�Ĳ����Զ�����Capturor
	Capturor(const RunningParams& params) :
		Capturor(
			params.mainPath, 
			params.fps, 
			params.recordTime, 
			params.cameraCount, 
			params.captureMode
		){}

	//��������ڵ�
	const bool Run() {
		if (CaptureMode::Task_Invalid == this->mode)
			return false;
		return CaptureMode::Turbo_On == this->mode ?
			RunWithTurboOn() : RunWithTurboOff();
	}

	//����JPEG�ļ�����
	void SetJPEGQuality(const int quality) { this->jpegQuality = quality; }

	//����FPS����
	const bool SaveFpsSeries(const char* fpsFileName);

	//��ȡƽ��֡��
	double GetAverageFPS() { return this->frameN * 1.0 / this->recordTime; }

	//�˺���������ղ�������Ӧ������ͷ������
	static void CleanBuffer(VideoCapture* VCs, unsigned count, bool* isRunning);

	//
	~Capturor();

private:
	/******************* private member variables *********************/
	unsigned cameraCount;		//��ǰ����������ͷ��Ŀ
	double fps;					//Ŀ��֡��
	double recordTime;			//Ŀ���¼ʱ��,��
	unsigned frameN;			//��ǰ�Ѽ�¼֡��
	int jpegQuality;			//jpg��ʽ�ļ�ѹ������ \in[0,100]
	double dfFrequency;			//cpuʱ��Ƶ��
	bool runningBufferCleaner;	//�����������������
	string saveDisc;			//�ļ������̷�
	vector<string> savePath;	//ÿ������ͷ��Ӧ���ļ�����·������
	VideoCapture* VCs;			//����ͷ��Ӧ�Ĳ�������
	//Mat* frames;				//����ÿ������ͷ��һ֡����
	CaptureMode mode;			//��ǰ����������ģʽ

	double* vFPS;				//ʵ��֡������
	unsigned vFPS_len;			//ʵ��֡�����м�¼��С

	/******************* private member functions *********************/
	
	//��CaptureTurbo���в�����
	const bool RunWithTurboOn();

	//��Normalģʽ���в�����
	const bool RunWithTurboOff();

	//����д���ļ��Թ̶���ʽ��д�����
	void ImageRewriter(const vector<int>& imgFormat);

};

#endif // !_CAPTUROR_H
