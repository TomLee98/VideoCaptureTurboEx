#include"CapturorHelper.h"
#include"userio.h"
#include<thread>

CapturorHelper::CapturorHelper(const string& mainPath, const double& fps, const double& recordTime, const unsigned& maxCameraCount)
{
	//==================== CAPTUROR HELPER CONSTRUCTOR BEGIN =========================
	//================================================================================
	/**************************** 0.��������Ƿ�Ϸ� ********************************/
	//================================================================================
	if (!IsValidPathName(mainPath.c_str())) {
		throw exception("ERROR(-1):Invalid File Folder");
	}
	else if (fps < MIN_FPS || fps > MAX_TURBO_FPS) {
		throw exception("ERROR(-2):Invalid Recording FPS");
	}
	else if (0.0 == recordTime || recordTime > 3600.0 * 24.0 * 7.0) {
		throw exception("ERROR(-3):Invalid Time For Recording");
	}
	else if (0u == maxCameraCount) {
		throw exception("ERROR(-4):Invalid Camera Count Limited");
	}
	auto hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	//================================================================================
	/******************************* 1.��ȡϵͳ���� *********************************/
	//================================================================================
	SendMsgToConsole(
		hConsoleOutput,
		TIPS,
		GB_FB,
		"���ڴ���������Э������...", true
	);

	GetCameraCount(maxCameraCount);

	CString disc(mainPath.substr(0, 3).c_str());
	GetSysInfo(&this->sysInfo, (wstring)disc);
	
	//================================================================================
	/************************** 2.������Capture����ģʽ ***************************/
	//================================================================================
	GetCaptureRunMode(mainPath, fps, recordTime);

	//===================== CAPTUROR HELPER CONSTRUCTOR END ==========================
}

const unsigned CapturorHelper::GetCameraCount(unsigned maxCameraNumber)
{
	unsigned camCount = 0u;

	for (unsigned k = 0; k < maxCameraNumber; k++)
	{
		//˳��������ͷ�����޷��򿪣�����ϵͳδע�������ͷ
		cv::VideoCapture vcp = cv::VideoCapture(k,cv::CAP_DSHOW);
		if (!vcp.isOpened()) {
			break;
		}
		camCount++;
	}
	this->cameraCount = camCount;
	return camCount;
}

void CapturorHelper::AskUserForTurbo()
{
	char choice;
	bool retry = true;
	auto hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	while (retry)
	{
		SendMsgToConsole(
			hConsoleOutput,
			TIPS,
			GB_FB,
			"��ǰ����FPS����ʵʱģʽ����FPS���Ƿ�����Turboģʽ[y/n]��"
		);
		cin >> choice;
		if (cin.good() && (choice=='y' || choice=='n')) {
			retry = false;
		}
		else {
			SendMsgToConsole(
				hConsoleOutput,
				ERR,
				RB_FB,
				"����ѡ�񲻷��Ϲ淶������������.", true
			);
			cin.clear();
			while (cin.get() != '\n')continue;
		}
	}
	if (choice == 'y') {
		this->isUserUseTurbo = true;
	}
	else {
		this->isUserUseTurbo = false;
	}
}

void CapturorHelper::AdviseUserForFPS()
{
	char choice;
	bool retry = true;
	auto hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	while (retry)
	{
		SendMsgToConsole(
			hConsoleOutput,
			TIPS,
			GB_FB,
			"��ǰ����FPS����Turboģʽ����FPS���Ƿ񽵵�Ϊ " + DoubleToString(this->turboStableFPS, 4) + " FPS[y/n]��"
		);
		cin >> choice;
		if (cin.good() && (choice == 'y' || choice == 'n')) {
			retry = false;
		}
		else {
			SendMsgToConsole(
				hConsoleOutput,
				ERR,
				RB_FB,
				"����ѡ�񲻷��Ϲ淶������������.", true
			);
			cin.clear();
			while (cin.get() != '\n')continue;
		}
	}
	if (choice == 'y') {
		this->isUserFixedFPS = true;
	}
	else {
		this->isUserFixedFPS = false;
	}
}

void CapturorHelper::GetCaptureRunMode(const string& mainPath, const double& fps, const double& recordTime)
{
	auto hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	this->maxFrameCount = this->sysInfo.rom_free_space
		/ (unsigned)(ZIP_IMG_SIZE_EXCEPT + 2 * ZIP_IMG_SIZE_STDEV);
	unsigned frameCountWanted = (unsigned)(recordTime * fps * this->cameraCount);
	//Try Formular(ʵ�鹫ʽ):ʹÿ���̹߳����������ص�60%,����ϵͳ��������Ĳ���
	//Ĭ�ϳ��̼߳���������ߵ���1.5����������
	this->normalStableFPS = 0.6 * (22.0 - NORMAL_FPS_COEFF * this->cameraCount)
		/ 8 * (this->sysInfo.cpu_core_counts + this->sysInfo.cpu_lcore_counts);
	//Turbo�����֡��Ϊ�ռ����ơ���д����ʾ���Ƶ�ƿ��ֵ
	//�˴������ڴ潻����CPUƵ�ʶ�д�����ʵ�Ӱ��
	//����ƽ������Ϊ����д���ʵ�50% (//TEST)
	//ʹÿ���̹߳����������ص�60%,����ϵͳ��������Ĳ���
	this->turboStableFPS = 0.6 * min(25.0,
		0.5 * min(
			this->sysInfo.rom_free_space / (recordTime * this->cameraCount * IMG_SIZE_EXCEPT),
			this->sysInfo.rom_write_rate / (IMG_SIZE_EXCEPT * this->cameraCount)
		) + TURBO_FPS_OFFSET) / 8 * (this->sysInfo.cpu_core_counts + this->sysInfo.cpu_lcore_counts);

	if (this->maxFrameCount <= frameCountWanted) {
		this->runParam.captureMode = CaptureMode::Task_Invalid;
		SendMsgToConsole(
			hConsoleOutput,
			ERR,
			RB_FB,
			"�洢�ռ䲻�㣬�������Ŀ¼.", true
		);
		return;
	}
	else if (!this->cameraCount) {
		this->runParam.captureMode = CaptureMode::Task_Invalid;
		SendMsgToConsole(
			hConsoleOutput,
			ERR,
			RB_FB,
			"δ��������ͷ����������.", true
		);
		return;
	}
	else {
		//���ü�¼��������
		this->runParam.cameraCount = this->cameraCount;
		this->runParam.mainPath = mainPath;
		this->runParam.recordTime = recordTime;

		if ((fps > this->normalStableFPS) && (this->turboStableFPS>this->normalStableFPS)) {
			//�������fps����ʵʱ���п�֧�ֵ��ȶ�֡��
			AskUserForTurbo();
			if (this->isUserUseTurbo) {
				if (this->turboStableFPS >= fps) {
					//����֡�ʵ���Turbo����֡�ʣ�����Turboģʽ
					this->runParam.captureMode = CaptureMode::Turbo_On;
					this->runParam.fps = fps;
					SendMsgToConsole(
						hConsoleOutput,
						TIPS,
						GB_FB,
						"������Turboģʽ(With " + DoubleToString(this->runParam.fps, 4) + " FPS)", true
					);
					return;
				}
				else {
					//֡�ʸ���Turboģʽ֡�ʣ������û�����֡��
					AdviseUserForFPS();
					if (this->isUserFixedFPS) {
						this->runParam.captureMode = CaptureMode::Turbo_On;
						this->runParam.fps = this->turboStableFPS;
						SendMsgToConsole(
							hConsoleOutput,
							TIPS,
							GB_FB,
							"������Turboģʽ(With " + DoubleToString(this->runParam.fps, 4) + " FPS)", true
						);
						return;
					}
					else {
						this->runParam.captureMode = CaptureMode::Turbo_On;
						this->runParam.fps = fps;
						//�û�����Լ���֡�ʣ�ֻ�ܸ������棬�Ų�����
						SendMsgToConsole(
							hConsoleOutput,
							WARN,
							YB_FB,
							"Turboģʽ�Զ�֡�ʽ���(���ܷ���֡�ʲ��ȶ�).", true
						);
						return;
					}
				}
			}
			else {
				this->runParam.captureMode = CaptureMode::Turbo_Off;
				this->runParam.fps = this->normalStableFPS;
				SendMsgToConsole(
					hConsoleOutput,
					TIPS,
					GB_FB,
					"δ����Turboģʽ���Զ�����ͨ��ģʽ(With " + DoubleToString(this->runParam.fps, 4) + " FPS).", true
				);
				return;
			}
		}
		else {
			//��ʱѡ����ͨģʽ���ȶ�֡��������֡���е���Сֵ
			this->runParam.captureMode = CaptureMode::Turbo_Off;
			this->runParam.fps = min(this->normalStableFPS, fps);
			SendMsgToConsole(
				hConsoleOutput,
				TIPS,
				GB_FB,
				"������ͨ��ģʽ(With " + DoubleToString(this->runParam.fps, 4) + " FPS)", true
			);
			return;
		}
	}
}
