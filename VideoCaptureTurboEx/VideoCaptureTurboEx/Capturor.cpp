#include"Capturor.h"
#include"CapturorHelper.h"
#include"userio.h"
#include<iomanip>
#include<fstream>
#include<Windows.h>
#include<thread>

Capturor::Capturor(const string& mainPath, const double& fps, const double& recordTime, unsigned cameraCount, CaptureMode captureMode)
{
	//======================== CAPTUROR CONSTRUCTOR BEGIN ============================
	//================================================================================
	/**************************** 0.检查输入是否合法 ********************************/
	//================================================================================
	if (!IsValidPathName(mainPath.c_str())) {
		throw exception("ERROR(-1):Invalid File Folder");
	}
	else if (fps < MIN_FPS || fps>MAX_TURBO_FPS) {
		throw exception("ERROR(-2):Invalid Recording FPS");
	}
	else if (0.0==recordTime || recordTime > 3600 * 24 * 7) {
		throw exception("ERROR(-3):Invalid Time For Recording");
	}
	else if (0u==cameraCount) {
		throw exception("ERROR(-4):Invalid Camera Count Limited");
	}
	else if (CaptureMode::Task_Invalid == captureMode) {
		throw exception("ERROR(-5):Invalid Task");
	}
	auto hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	//================================================================================
	/*********************** 1.根据主目录初始化摄像头写入路径 ***********************/
	//================================================================================
	this->cameraCount = cameraCount;
	if (!this->cameraCount) {
		throw exception("ERROR(-6):Not Found Camera In this Device");
	}
	//根据摄像头数量初始化每个摄像头的捕获流(分配内存)
	this->VCs = new cv::VideoCapture[this->cameraCount];
	//this->frames = new cv::Mat[this->cameraCount];
	this->vFPS = nullptr;
	this->vFPS_len = 0;
	this->saveDisc = mainPath.substr(0, 3);	//format as: "C:\\"
	this->runningBufferCleaner = false;

	//初始化检测摄像头
	cv::Mat tmpFrame = Mat(480, 640, CV_8UC3);
	SendMsgToConsole(
		hConsoleOutput,
		TIPS,
		GB_FB,
		"摄像头设置初始化...", true
	);
	
	for (unsigned k = 0; k < this->cameraCount; k++) {
		this->savePath.push_back(mainPath + "\\" + GetYMD() + "\\Camera_" + to_string(k) + "\\");

		//设置每个捕获流的参数
		this->VCs[k].set(CAP_PROP_FPS, 30.0);
		this->VCs[k].set(CAP_PROP_FOURCC, VideoWriter::fourcc('M', 'J', 'P', 'G'));
		this->VCs[k].set(CAP_PROP_FRAME_WIDTH, 640);
		this->VCs[k].set(CAP_PROP_FRAME_HEIGHT, 480);
		//依次打开摄像头(以 DSHOW 模式)
		if (!this->VCs[k].open(k, CAP_DSHOW)) {
			SendMsgToConsole(
				hConsoleOutput,
				ERR,
				RB_FB,
				"Camera_" + to_string(k) + "初始化失败...", true
			);
			throw exception("ERROR(-7):Camera Initial Failed");
		}
		else {
			//读取第一帧舍弃（内部分配内存，占用~750ms）
			if (!VCs[k].read(tmpFrame)) {
				SendMsgToConsole(
					hConsoleOutput,
					ERR,
					RB_FB,
					"Camera_" + to_string(k) + "数据读取失败...", true
				);
				throw exception("ERROR(-8):Bad Camera Input Stream");
			}
			else {
				//生成路径
				RecursiveMkdir(this->savePath[k].c_str());
			}
		}
	}
	//==============================================================================
	/***************************** 2.设置捕获器关键信息****************************/
	//==============================================================================
	this->fps = fps;
	this->recordTime = recordTime;
	this->jpegQuality = 75;
	this->frameN = 0u;
	//默认采用Turbo_Off模式运行捕获器
	this->mode = captureMode;
	//设置精准定时器时钟频率
	LARGE_INTEGER litmp;
	QueryPerformanceFrequency(&litmp);
	this->dfFrequency = (double)litmp.QuadPart;

	//======================== CAPTUROR CONSTRUCTOR END =============================
}

const bool Capturor::RunWithTurboOn()
{
	double recordingTime = 0.0, fr_time = 0.0, delay = 1000.0 / this->fps;
	unsigned cnt = 0u;
	LARGE_INTEGER litmp;
	LONGLONG QPart1, QPart2;
	COORD cursorOldPosition;
	ofstream outFile;
	cv::Mat tmpFrame = cv::Mat(480, 640, CV_8UC3);	//设置为640*480视频宽度
	CString fileName_CStr((this->saveDisc + (string)BUFFER_FILENAME).c_str()); 
	vector<int> imgFormat = { cv::IMWRITE_JPEG_QUALITY, this->jpegQuality };
	auto hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	cout << setiosflags(ios::fixed);
	//分配内存空间
	this->vFPS = new double[(size_t)round(this->recordTime * this->fps)];

	SendMsgToConsole(
		hConsoleOutput,
		TIPS,
		GB_FB,
		"开始记录(With Capture Turbo On)...", true
	);
	//在当前选定盘符下创建一个隐藏文件
	//用于保存图像的二进制数据
	outFile.open(this->saveDisc + (string)BUFFER_FILENAME, ios::binary | ios::app);
	if (!outFile.is_open()) {
		throw exception("ERROR(-9) Can't Open Buffer.");
	}
	SetFileAttributes(fileName_CStr, FILE_ATTRIBUTE_HIDDEN);

	//记录当前光标所在位置
	WhereXY(cursorOldPosition);
	//启动清空缓冲区子线程
	thread subth(CleanBuffer, this->VCs, this->cameraCount, &this->runningBufferCleaner);
	subth.detach();	//分离子线程
	this->runningBufferCleaner = true;
	//开始主循环记录数据
	while (recordingTime < this->recordTime) {
		//获取高精度定时
		QueryPerformanceCounter(&litmp);
		QPart1 = litmp.QuadPart;

		//依次访问每一个摄像头,写入数据
		for (unsigned k = 0; k < this->cameraCount; k++) {
			//如果没读到数据，直接退出循环
			if (!VCs[k].read(tmpFrame)) {
				//清理磁盘缓存
				outFile.close();
				remove((this->saveDisc + BUFFER_FILENAME).c_str());
				//抛出异常
				throw exception("ERROR(-8):Bad Camera Input Stream");
			}

			//显示当前帧
			cv::imshow("Camera_" + to_string(k), tmpFrame);
			cv::waitKey(1);

			//将当前帧写入磁盘缓冲文件
			for (int r = 0; r < tmpFrame.rows; r++) {
				outFile.write(
					reinterpret_cast<const char*>(tmpFrame.ptr(r)),
					tmpFrame.cols * tmpFrame.elemSize()
				);
			}
		}
		//帧数新增
		cnt++;

		if (fr_time >= 1000.0) {
			recordingTime += fr_time / 1000.0;
			//还原光标位置
			GotoXY(cursorOldPosition);
			//显示实时帧率
			SetConsoleTextAttribute(hConsoleOutput, GB_FB);
			cout << "[提示] ";
			SetConsoleTextAttribute(hConsoleOutput, WB_FB);
			cout << "当前帧率：" << setprecision(2) << setw(5) << cnt * 1000.0 / fr_time << " FPS, "
				<< "已记录时长：" << GetHMSFrom(recordingTime);
			this->vFPS[this->vFPS_len++] = cnt * 1000.0 / fr_time;
			cnt = 0;
			fr_time = 0;
		}
		this->frameN++;
		QueryPerformanceCounter(&litmp);
		QPart2 = litmp.QuadPart;
		//使用高精度计时器补足时长
		while ((QPart2 - QPart1) * 1000.0 / this->dfFrequency < delay) {
			QueryPerformanceCounter(&litmp);
			QPart2 = litmp.QuadPart;
		}
		fr_time += (QPart2 - QPart1) * 1000.0 / this->dfFrequency;
	}
	
	this->runningBufferCleaner = false;
	cout << endl;
	//关闭所有已分配的窗体
	cv::destroyAllWindows();

	outFile.close();
	ImageRewriter(imgFormat);

	return true;
}

const bool Capturor::RunWithTurboOff()
{
	//将输出格式设置为jpeg
	vector<int> imgFormat = { cv::IMWRITE_JPEG_QUALITY, this->jpegQuality };
	double recordingTime = 0.0, fr_time = 0.0, delay = 1000.0 / this->fps;
	unsigned cnt = 0u;
	LARGE_INTEGER litmp;
	LONGLONG QPart1, QPart2;
	COORD cursorOldPosition;
	cv::Mat tmpFrame = cv::Mat(480, 640, CV_8UC3);	//设置为640*480视频宽度
	auto hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	cout << setiosflags(ios::fixed);

	this->vFPS = new double[(size_t)round(this->recordTime * this->fps)];

	SendMsgToConsole(
		hConsoleOutput,
		TIPS,
		GB_FB,
		"开始记录(With Capture Turbo Off)...", true
	);

	//记录当前光标所在位置
	WhereXY(cursorOldPosition);
	//启动清空缓冲区子线程
	thread subth(CleanBuffer, this->VCs, this->cameraCount,&this->runningBufferCleaner);
	subth.detach();
	this->runningBufferCleaner = true;
	//开始主循环记录数据
	while (recordingTime < this->recordTime) {
		//获取高精度定时
		QueryPerformanceCounter(&litmp);
		QPart1 = litmp.QuadPart;
		//单线程依次访问每一个摄像头,并压缩写入数据
		for (unsigned k = 0; k < this->cameraCount; k++) {
			//如果没读到数据，直接退出循环
			if (!this->VCs[k].read(tmpFrame)) {
				throw exception("ERROR(-8):Bad Camera Input Stream");
			}
			//显示当前帧
			cv::imshow("Camera_" + to_string(k), tmpFrame);
			cv::waitKey(1);	//刷新当前图像
			//将当前帧写入磁盘
			cv::imwrite(this->savePath[k] + to_string(this->frameN) + ".jpg", tmpFrame, imgFormat);
		}
		//帧数新增
		cnt++;

		if (fr_time >= 1000.0) {
			recordingTime += fr_time / 1000.0;
			//还原光标位置
			GotoXY(cursorOldPosition);
			//显示实时帧率
			SetConsoleTextAttribute(hConsoleOutput, GB_FB);
			cout << "[提示] ";
			SetConsoleTextAttribute(hConsoleOutput, WB_FB);
			cout << "当前帧率：" << setprecision(2) << setw(5) << cnt * 1000.0 / fr_time << " FPS, "
				<< "已记录时长：" << GetHMSFrom(recordingTime);
			this->vFPS[this->vFPS_len++] = cnt * 1000.0 / fr_time;
			cnt = 0;
			fr_time = 0;
		}
		this->frameN++;
		QueryPerformanceCounter(&litmp);
		QPart2 = litmp.QuadPart;
		//使用高精度计时器补足时长
		while ((QPart2 - QPart1) * 1000.0 / this->dfFrequency < delay) {
			QueryPerformanceCounter(&litmp);
			QPart2 = litmp.QuadPart;
		}
		fr_time += (QPart2 - QPart1) * 1000.0 / this->dfFrequency;
	}

	this->runningBufferCleaner = false;
	cout << endl;
	//关闭所有已分配的窗体
	cv::destroyAllWindows();

	return true;
}

void Capturor::ImageRewriter(const vector<int>& imgFormat)
{
	ifstream inFile;
	unsigned* savedNums = new unsigned[this->cameraCount]{ 0u };
	unsigned idx = 0u, saveN = 0u;
	Mat img = Mat(480, 640, CV_8UC3);
	auto hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	vector<Mat> frames;
	COORD r;	//记录光标位置

	unsigned oneLoadCount = GetRAMFreeSpace() / IMG_SIZE_EXCEPT / 2u; //使用当前可用物理内存的一半作为buffer
	oneLoadCount -= oneLoadCount % this->cameraCount; //将一次的加载数设置为cameraCount的整数倍
	unsigned leftCount = this->frameN * this->cameraCount % oneLoadCount;	//余下的加载次数
	
	//预先分配一次读入数目的内存空间
	//但不需要初始化对象！！！
	if (leftCount < this->frameN * this->cameraCount) {
		frames.reserve((size_t)(oneLoadCount));
	}
	else {
		frames.reserve((size_t)(leftCount));
	}
	
	inFile.open(this->saveDisc + (string)BUFFER_FILENAME, ios::in | ios::binary);
	if (!inFile.is_open()) {
		throw exception("ERROR(-9) Can't Open Buffer.");
	}

	SendMsgToConsole(
		hConsoleOutput,
		TIPS,
		GB_FB,
		"离线压缩程序启动，已完成 "
	);
	WhereXY(r);

	for (unsigned img_all_c = 0; img_all_c < this->frameN * this->cameraCount; img_all_c++) {
		//读入一幅图像
		for (unsigned r = img_all_c; r < img_all_c + img.rows; r++) {
			inFile.read(
				reinterpret_cast<char*>(img.ptr(r - img_all_c)),
				img.cols * img.elemSize()
			);
		}
		//将读入的img入队列
		frames.push_back(img.clone());

		//如果queue中读入的数据量达到一次读入的限定
		if (frames.size() == oneLoadCount) {
			//将这一批数据使用imwrite导出至对应目录
			for (unsigned k = 0; k < this->cameraCount; k++) {
				for (idx = k + savedNums[k]; idx < savedNums[k] + oneLoadCount; idx += this->cameraCount) {
					imwrite(
						this->savePath[k] + to_string(idx / this->cameraCount) + ".jpg",
						frames[idx-savedNums[k]], imgFormat
					);
					cout << setprecision(2) << setw(6)
						<< saveN++ * 100.0 / (this->frameN * this->cameraCount - 1)
						<< " %" << endl;
					GotoXY(r);
				}
				savedNums[k] = idx;
			}
			//清空当前存储文件
			frames.clear();
		}
		else {
			//最后一次读入的数据量必然不超过oneLoadCount
		}
	}
	inFile.close();

	//如果最后一次读入了数据
	if (frames.size()) {
		//将这一批数据使用imwrite导出至对应目录
		for (unsigned k = 0; k < this->cameraCount; k++) {
			for (idx = k + savedNums[k]; idx < savedNums[k] + leftCount; idx += this->cameraCount) {
				imwrite(
					this->savePath[k] + to_string(idx / this->cameraCount) + ".jpg",
					frames[idx-savedNums[k]], imgFormat
				);
				cout << setprecision(2) << setw(6)
					<< saveN++ * 100.0 / (this->frameN * this->cameraCount - 1)
					<< " %" << endl;
				GotoXY(r);
			}
		}
	}
	cout << endl;
	//最后删除缓存数据
	remove((this->saveDisc + (string)BUFFER_FILENAME).c_str());
}

const bool Capturor::SaveFpsSeries(const char* fpsFileName)
{
	string fileFullPath = this->saveDisc + (string)fpsFileName + ".csv";
	fstream outFile;
	outFile.open(fileFullPath.c_str(), ios::out | ios::app);
	if (!outFile.is_open()) {
		throw exception("ERROR(-9) Can't Open Buffer.");
	}
	for (size_t k = 0; k < this->vFPS_len; k++) {
		if (fabs(vFPS[k]) > 1e-3) {
			outFile << vFPS[k] << "," << endl;
		}
	}
	outFile.close();
	return true;
}

void Capturor::CleanBuffer(VideoCapture* VCs, unsigned count, bool* isRunning)
{
	while (*isRunning)
	{
		for (unsigned k = 0; k < count; k++) {
			if (*isRunning)
				//抓取每个摄像头保持其缓冲区为空
				VCs[k].grab();
			else
				return;
		}
	}
}

Capturor::~Capturor()
{
	delete[] this->VCs;
	this->VCs = nullptr;
	delete[] this->vFPS;
	this->savePath.~vector();
}
