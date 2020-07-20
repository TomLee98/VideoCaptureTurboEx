#include"CapturorHelper.h"
#include"userio.h"
#include<thread>

CapturorHelper::CapturorHelper(const string& mainPath, const double& fps, const double& recordTime, const unsigned& maxCameraCount)
{
	//==================== CAPTUROR HELPER CONSTRUCTOR BEGIN =========================
	//================================================================================
	/**************************** 0.检查输入是否合法 ********************************/
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
	/******************************* 1.获取系统参数 *********************************/
	//================================================================================
	SendMsgToConsole(
		hConsoleOutput,
		TIPS,
		GB_FB,
		"正在创建捕获器协助进程...", true
	);

	GetCameraCount(maxCameraCount);

	CString disc(mainPath.substr(0, 3).c_str());
	GetSysInfo(&this->sysInfo, (wstring)disc);
	
	//================================================================================
	/************************** 2.计算获得Capture运行模式 ***************************/
	//================================================================================
	GetCaptureRunMode(mainPath, fps, recordTime);

	//===================== CAPTUROR HELPER CONSTRUCTOR END ==========================
}

const unsigned CapturorHelper::GetCameraCount(unsigned maxCameraNumber)
{
	unsigned camCount = 0u;

	for (unsigned k = 0; k < maxCameraNumber; k++)
	{
		//顺序开启摄像头，若无法打开，表明系统未注册此摄像头
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
			"当前请求FPS高于实时模式可行FPS，是否启用Turbo模式[y/n]："
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
				"输入选择不符合规范，请重新输入.", true
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
			"当前请求FPS高于Turbo模式可行FPS，是否降低为 " + DoubleToString(this->turboStableFPS, 4) + " FPS[y/n]："
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
				"输入选择不符合规范，请重新输入.", true
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
	//Try Formular(实验公式):使每个线程工作在满负载的60%,避免系统调用引起的波动
	//默认超线程技术可以提高单核1.5倍处理能力
	this->normalStableFPS = 0.6 * (22.0 - NORMAL_FPS_COEFF * this->cameraCount)
		/ 8 * (this->sysInfo.cpu_core_counts + this->sysInfo.cpu_lcore_counts);
	//Turbo的最大帧率为空间限制、读写与显示限制的瓶颈值
	//此处忽略内存交换、CPU频率对写入速率的影响
	//磁盘平均速率为最大读写速率的50% (//TEST)
	//使每个线程工作在满负载的60%,避免系统调用引起的波动
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
			"存储空间不足，请更换主目录.", true
		);
		return;
	}
	else if (!this->cameraCount) {
		this->runParam.captureMode = CaptureMode::Task_Invalid;
		SendMsgToConsole(
			hConsoleOutput,
			ERR,
			RB_FB,
			"未发现摄像头，请检查连接.", true
		);
		return;
	}
	else {
		//设置记录公共参数
		this->runParam.cameraCount = this->cameraCount;
		this->runParam.mainPath = mainPath;
		this->runParam.recordTime = recordTime;

		if ((fps > this->normalStableFPS) && (this->turboStableFPS>this->normalStableFPS)) {
			//如果请求fps高于实时运行可支持的稳定帧率
			AskUserForTurbo();
			if (this->isUserUseTurbo) {
				if (this->turboStableFPS >= fps) {
					//请求帧率低于Turbo可行帧率，启用Turbo模式
					this->runParam.captureMode = CaptureMode::Turbo_On;
					this->runParam.fps = fps;
					SendMsgToConsole(
						hConsoleOutput,
						TIPS,
						GB_FB,
						"已启用Turbo模式(With " + DoubleToString(this->runParam.fps, 4) + " FPS)", true
					);
					return;
				}
				else {
					//帧率高于Turbo模式帧率，建议用户调整帧率
					AdviseUserForFPS();
					if (this->isUserFixedFPS) {
						this->runParam.captureMode = CaptureMode::Turbo_On;
						this->runParam.fps = this->turboStableFPS;
						SendMsgToConsole(
							hConsoleOutput,
							TIPS,
							GB_FB,
							"已启用Turbo模式(With " + DoubleToString(this->runParam.fps, 4) + " FPS)", true
						);
						return;
					}
					else {
						this->runParam.captureMode = CaptureMode::Turbo_On;
						this->runParam.fps = fps;
						//用户坚持自己的帧率，只能给出警告，概不负责
						SendMsgToConsole(
							hConsoleOutput,
							WARN,
							YB_FB,
							"Turbo模式自动帧率解锁(可能发生帧率不稳定).", true
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
					"未启用Turbo模式，自动启用通用模式(With " + DoubleToString(this->runParam.fps, 4) + " FPS).", true
				);
				return;
			}
		}
		else {
			//此时选择普通模式下稳定帧率与请求帧率中的最小值
			this->runParam.captureMode = CaptureMode::Turbo_Off;
			this->runParam.fps = min(this->normalStableFPS, fps);
			SendMsgToConsole(
				hConsoleOutput,
				TIPS,
				GB_FB,
				"已启用通用模式(With " + DoubleToString(this->runParam.fps, 4) + " FPS)", true
			);
			return;
		}
	}
}
