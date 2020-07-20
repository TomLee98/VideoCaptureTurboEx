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
	/**************************** 0.��������Ƿ�Ϸ� ********************************/
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
	/*********************** 1.������Ŀ¼��ʼ������ͷд��·�� ***********************/
	//================================================================================
	this->cameraCount = cameraCount;
	if (!this->cameraCount) {
		throw exception("ERROR(-6):Not Found Camera In this Device");
	}
	//��������ͷ������ʼ��ÿ������ͷ�Ĳ�����(�����ڴ�)
	this->VCs = new cv::VideoCapture[this->cameraCount];
	//this->frames = new cv::Mat[this->cameraCount];
	this->vFPS = nullptr;
	this->vFPS_len = 0;
	this->saveDisc = mainPath.substr(0, 3);	//format as: "C:\\"
	this->runningBufferCleaner = false;

	//��ʼ���������ͷ
	cv::Mat tmpFrame = Mat(480, 640, CV_8UC3);
	SendMsgToConsole(
		hConsoleOutput,
		TIPS,
		GB_FB,
		"����ͷ���ó�ʼ��...", true
	);
	
	for (unsigned k = 0; k < this->cameraCount; k++) {
		this->savePath.push_back(mainPath + "\\" + GetYMD() + "\\Camera_" + to_string(k) + "\\");

		//����ÿ���������Ĳ���
		this->VCs[k].set(CAP_PROP_FPS, 30.0);
		this->VCs[k].set(CAP_PROP_FOURCC, VideoWriter::fourcc('M', 'J', 'P', 'G'));
		this->VCs[k].set(CAP_PROP_FRAME_WIDTH, 640);
		this->VCs[k].set(CAP_PROP_FRAME_HEIGHT, 480);
		//���δ�����ͷ(�� DSHOW ģʽ)
		if (!this->VCs[k].open(k, CAP_DSHOW)) {
			SendMsgToConsole(
				hConsoleOutput,
				ERR,
				RB_FB,
				"Camera_" + to_string(k) + "��ʼ��ʧ��...", true
			);
			throw exception("ERROR(-7):Camera Initial Failed");
		}
		else {
			//��ȡ��һ֡�������ڲ������ڴ棬ռ��~750ms��
			if (!VCs[k].read(tmpFrame)) {
				SendMsgToConsole(
					hConsoleOutput,
					ERR,
					RB_FB,
					"Camera_" + to_string(k) + "���ݶ�ȡʧ��...", true
				);
				throw exception("ERROR(-8):Bad Camera Input Stream");
			}
			else {
				//����·��
				RecursiveMkdir(this->savePath[k].c_str());
			}
		}
	}
	//==============================================================================
	/***************************** 2.���ò������ؼ���Ϣ****************************/
	//==============================================================================
	this->fps = fps;
	this->recordTime = recordTime;
	this->jpegQuality = 75;
	this->frameN = 0u;
	//Ĭ�ϲ���Turbo_Offģʽ���в�����
	this->mode = captureMode;
	//���þ�׼��ʱ��ʱ��Ƶ��
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
	cv::Mat tmpFrame = cv::Mat(480, 640, CV_8UC3);	//����Ϊ640*480��Ƶ���
	CString fileName_CStr((this->saveDisc + (string)BUFFER_FILENAME).c_str()); 
	vector<int> imgFormat = { cv::IMWRITE_JPEG_QUALITY, this->jpegQuality };
	auto hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	cout << setiosflags(ios::fixed);
	//�����ڴ�ռ�
	this->vFPS = new double[(size_t)round(this->recordTime * this->fps)];

	SendMsgToConsole(
		hConsoleOutput,
		TIPS,
		GB_FB,
		"��ʼ��¼(With Capture Turbo On)...", true
	);
	//�ڵ�ǰѡ���̷��´���һ�������ļ�
	//���ڱ���ͼ��Ķ���������
	outFile.open(this->saveDisc + (string)BUFFER_FILENAME, ios::binary | ios::app);
	if (!outFile.is_open()) {
		throw exception("ERROR(-9) Can't Open Buffer.");
	}
	SetFileAttributes(fileName_CStr, FILE_ATTRIBUTE_HIDDEN);

	//��¼��ǰ�������λ��
	WhereXY(cursorOldPosition);
	//������ջ��������߳�
	thread subth(CleanBuffer, this->VCs, this->cameraCount, &this->runningBufferCleaner);
	subth.detach();	//�������߳�
	this->runningBufferCleaner = true;
	//��ʼ��ѭ����¼����
	while (recordingTime < this->recordTime) {
		//��ȡ�߾��ȶ�ʱ
		QueryPerformanceCounter(&litmp);
		QPart1 = litmp.QuadPart;

		//���η���ÿһ������ͷ,д������
		for (unsigned k = 0; k < this->cameraCount; k++) {
			//���û�������ݣ�ֱ���˳�ѭ��
			if (!VCs[k].read(tmpFrame)) {
				//������̻���
				outFile.close();
				remove((this->saveDisc + BUFFER_FILENAME).c_str());
				//�׳��쳣
				throw exception("ERROR(-8):Bad Camera Input Stream");
			}

			//��ʾ��ǰ֡
			cv::imshow("Camera_" + to_string(k), tmpFrame);
			cv::waitKey(1);

			//����ǰ֡д����̻����ļ�
			for (int r = 0; r < tmpFrame.rows; r++) {
				outFile.write(
					reinterpret_cast<const char*>(tmpFrame.ptr(r)),
					tmpFrame.cols * tmpFrame.elemSize()
				);
			}
		}
		//֡������
		cnt++;

		if (fr_time >= 1000.0) {
			recordingTime += fr_time / 1000.0;
			//��ԭ���λ��
			GotoXY(cursorOldPosition);
			//��ʾʵʱ֡��
			SetConsoleTextAttribute(hConsoleOutput, GB_FB);
			cout << "[��ʾ] ";
			SetConsoleTextAttribute(hConsoleOutput, WB_FB);
			cout << "��ǰ֡�ʣ�" << setprecision(2) << setw(5) << cnt * 1000.0 / fr_time << " FPS, "
				<< "�Ѽ�¼ʱ����" << GetHMSFrom(recordingTime);
			this->vFPS[this->vFPS_len++] = cnt * 1000.0 / fr_time;
			cnt = 0;
			fr_time = 0;
		}
		this->frameN++;
		QueryPerformanceCounter(&litmp);
		QPart2 = litmp.QuadPart;
		//ʹ�ø߾��ȼ�ʱ������ʱ��
		while ((QPart2 - QPart1) * 1000.0 / this->dfFrequency < delay) {
			QueryPerformanceCounter(&litmp);
			QPart2 = litmp.QuadPart;
		}
		fr_time += (QPart2 - QPart1) * 1000.0 / this->dfFrequency;
	}
	
	this->runningBufferCleaner = false;
	cout << endl;
	//�ر������ѷ���Ĵ���
	cv::destroyAllWindows();

	outFile.close();
	ImageRewriter(imgFormat);

	return true;
}

const bool Capturor::RunWithTurboOff()
{
	//�������ʽ����Ϊjpeg
	vector<int> imgFormat = { cv::IMWRITE_JPEG_QUALITY, this->jpegQuality };
	double recordingTime = 0.0, fr_time = 0.0, delay = 1000.0 / this->fps;
	unsigned cnt = 0u;
	LARGE_INTEGER litmp;
	LONGLONG QPart1, QPart2;
	COORD cursorOldPosition;
	cv::Mat tmpFrame = cv::Mat(480, 640, CV_8UC3);	//����Ϊ640*480��Ƶ���
	auto hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	cout << setiosflags(ios::fixed);

	this->vFPS = new double[(size_t)round(this->recordTime * this->fps)];

	SendMsgToConsole(
		hConsoleOutput,
		TIPS,
		GB_FB,
		"��ʼ��¼(With Capture Turbo Off)...", true
	);

	//��¼��ǰ�������λ��
	WhereXY(cursorOldPosition);
	//������ջ��������߳�
	thread subth(CleanBuffer, this->VCs, this->cameraCount,&this->runningBufferCleaner);
	subth.detach();
	this->runningBufferCleaner = true;
	//��ʼ��ѭ����¼����
	while (recordingTime < this->recordTime) {
		//��ȡ�߾��ȶ�ʱ
		QueryPerformanceCounter(&litmp);
		QPart1 = litmp.QuadPart;
		//���߳����η���ÿһ������ͷ,��ѹ��д������
		for (unsigned k = 0; k < this->cameraCount; k++) {
			//���û�������ݣ�ֱ���˳�ѭ��
			if (!this->VCs[k].read(tmpFrame)) {
				throw exception("ERROR(-8):Bad Camera Input Stream");
			}
			//��ʾ��ǰ֡
			cv::imshow("Camera_" + to_string(k), tmpFrame);
			cv::waitKey(1);	//ˢ�µ�ǰͼ��
			//����ǰ֡д�����
			cv::imwrite(this->savePath[k] + to_string(this->frameN) + ".jpg", tmpFrame, imgFormat);
		}
		//֡������
		cnt++;

		if (fr_time >= 1000.0) {
			recordingTime += fr_time / 1000.0;
			//��ԭ���λ��
			GotoXY(cursorOldPosition);
			//��ʾʵʱ֡��
			SetConsoleTextAttribute(hConsoleOutput, GB_FB);
			cout << "[��ʾ] ";
			SetConsoleTextAttribute(hConsoleOutput, WB_FB);
			cout << "��ǰ֡�ʣ�" << setprecision(2) << setw(5) << cnt * 1000.0 / fr_time << " FPS, "
				<< "�Ѽ�¼ʱ����" << GetHMSFrom(recordingTime);
			this->vFPS[this->vFPS_len++] = cnt * 1000.0 / fr_time;
			cnt = 0;
			fr_time = 0;
		}
		this->frameN++;
		QueryPerformanceCounter(&litmp);
		QPart2 = litmp.QuadPart;
		//ʹ�ø߾��ȼ�ʱ������ʱ��
		while ((QPart2 - QPart1) * 1000.0 / this->dfFrequency < delay) {
			QueryPerformanceCounter(&litmp);
			QPart2 = litmp.QuadPart;
		}
		fr_time += (QPart2 - QPart1) * 1000.0 / this->dfFrequency;
	}

	this->runningBufferCleaner = false;
	cout << endl;
	//�ر������ѷ���Ĵ���
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
	COORD r;	//��¼���λ��

	unsigned oneLoadCount = GetRAMFreeSpace() / IMG_SIZE_EXCEPT / 2u; //ʹ�õ�ǰ���������ڴ��һ����Ϊbuffer
	oneLoadCount -= oneLoadCount % this->cameraCount; //��һ�εļ���������ΪcameraCount��������
	unsigned leftCount = this->frameN * this->cameraCount % oneLoadCount;	//���µļ��ش���
	
	//Ԥ�ȷ���һ�ζ�����Ŀ���ڴ�ռ�
	//������Ҫ��ʼ�����󣡣���
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
		"����ѹ����������������� "
	);
	WhereXY(r);

	for (unsigned img_all_c = 0; img_all_c < this->frameN * this->cameraCount; img_all_c++) {
		//����һ��ͼ��
		for (unsigned r = img_all_c; r < img_all_c + img.rows; r++) {
			inFile.read(
				reinterpret_cast<char*>(img.ptr(r - img_all_c)),
				img.cols * img.elemSize()
			);
		}
		//�������img�����
		frames.push_back(img.clone());

		//���queue�ж�����������ﵽһ�ζ�����޶�
		if (frames.size() == oneLoadCount) {
			//����һ������ʹ��imwrite��������ӦĿ¼
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
			//��յ�ǰ�洢�ļ�
			frames.clear();
		}
		else {
			//���һ�ζ������������Ȼ������oneLoadCount
		}
	}
	inFile.close();

	//������һ�ζ���������
	if (frames.size()) {
		//����һ������ʹ��imwrite��������ӦĿ¼
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
	//���ɾ����������
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
				//ץȡÿ������ͷ�����仺����Ϊ��
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
