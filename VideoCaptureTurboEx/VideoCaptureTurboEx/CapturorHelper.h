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

//���岶��������ģʽ
typedef enum class _CAPTURE_MODE { Task_Invalid = -1, Turbo_Off, Turbo_On }CaptureMode;

//���岶�������в���
typedef struct _RUNNING_PARAMETERS_
{
	string mainPath;		//����д����Ŀ¼
	double fps;				//���ݱ���֡��
	double recordTime;		//���ݼ�¼ʱ��,��
	unsigned cameraCount;	//��ǰ�豸����ͷ����
	CaptureMode captureMode;//����������ģʽ
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

	//��ȡ���������в���
	const RunningParams GetRunningParams()const {
		return this->runParam;
	}

	~CapturorHelper() {}

private:
	/******************* private member variables *********************/
	unsigned maxFrameCount;		//���ɼ�¼֡��(ѹ����)
	double normalStableFPS;		//Normal�ȶ���¼ģʽ��֡��
	unsigned cameraCount;		//��ǰ�豸����ͷ����
	double turboStableFPS;		//����Turboģʽ���ȶ�֡��
	bool isUserUseTurbo;		//����û��Ƿ�����Turboģʽ
	bool isUserFixedFPS;		//����û��Ƿ�ͬ�����FPS
	SysInfo sysInfo;			//��¼ϵͳ��Ϣ
	RunningParams runParam;		//������������в���

	/******************* private member functions *********************/
	//��ȡ��ǰ�豸���ӵ�����ͷ��Ŀ
	const unsigned GetCameraCount(unsigned maxCameraNumber);

	//��ȡ�û���Turbo������
	void AskUserForTurbo();

	//�����û�������֡�ʵ���Ϊ����ֵ
	void AdviseUserForFPS();

	//������Capture����ģʽ���������û�������
	void GetCaptureRunMode(
		const string&	mainPath,
		const double&	fps,
		const double&	recordTime
	);
};

#endif // !_CAPTUROR_HELPER_H
