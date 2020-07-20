#include"sysinfo.h"
#include"userio.h"
#include"Capturor.h"
#include"CapturorHelper.h"

void ModeCase(int argc, char* argv[]);

void normalMode(void);

void factoryMode(int argc,char* argv[]);

void appendMode(void);
//

int main(int argc, char* argv[])
{
	ModeCase(argc, argv);

	system("pause");

	return 0;
}

//根据输入选择不同的运行模式
void ModeCase(int argc, char* argv[])
{
	auto hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	switch (argc)
	{
	case 1:
		normalMode();
		break;
	case 2:
	case 3:
		factoryMode(argc, argv);
		break;
	default:
		SendMsgToConsole(
			hConsoleOutput,
			ERR,
			RB_FB,
			"异常调用，程序结束.", true
		);
		break;
	}
}

//一般模式
//该模式一次打开所有摄像头并同步获取数据
void normalMode(void)
{
	string mainPath;
	double fps;
	double recordT;
	auto hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	GetBasicParams(mainPath, fps, recordT);

	CapturorHelper helper(mainPath, fps, recordT);

	Capturor capturor(helper.GetRunningParams());

	capturor.Run();
}

//工厂模式（调试）
//该模式在一般模式的基础上将帧率-时间序列保存在本地磁盘上
void factoryMode(int argc, char* argv[])
{
	string mainPath;
	double fps;
	double recordT;
	auto hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	GetBasicParams(mainPath, fps, recordT);

	CapturorHelper helper(mainPath, fps, recordT);

	Capturor capturor(helper.GetRunningParams());

	capturor.Run();
	//
	SendMsgToConsole(
		hConsoleOutput,
		DBG,
		MB_FB,
		"平均帧率为：" + DoubleToString(capturor.GetAverageFPS(), 4) + " FPS", true
	);
	//
	if (!strcmp(argv[1], "-f")) {
		if (argc == 2) {
			capturor.SaveFpsSeries("FPS");
		}
		else {
			capturor.SaveFpsSeries(argv[2]);
		}
		SendMsgToConsole(
			hConsoleOutput,
			DBG,
			MB_FB,
			"帧率记录已保存至根目录.", true
		);
	}
}

//@brief 添加模式
//该模式依次添加摄像头并获取数据
void appendMode(void)
{

}
