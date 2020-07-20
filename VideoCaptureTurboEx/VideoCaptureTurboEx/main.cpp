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

//��������ѡ��ͬ������ģʽ
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
			"�쳣���ã��������.", true
		);
		break;
	}
}

//һ��ģʽ
//��ģʽһ�δ���������ͷ��ͬ����ȡ����
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

//����ģʽ�����ԣ�
//��ģʽ��һ��ģʽ�Ļ����Ͻ�֡��-ʱ�����б����ڱ��ش�����
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
		"ƽ��֡��Ϊ��" + DoubleToString(capturor.GetAverageFPS(), 4) + " FPS", true
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
			"֡�ʼ�¼�ѱ�������Ŀ¼.", true
		);
	}
}

//@brief ���ģʽ
//��ģʽ�����������ͷ����ȡ����
void appendMode(void)
{

}
