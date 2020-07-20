#pragma once
#ifndef _USER_IO_H
#define _USER_IO_H
#include<Windows.h>
#include<iostream>
#include<string>
#include<vector>
using namespace std;

#define MAX_FOLDER_NAME_LEN 255

#define WB_FB	FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE
#define RB_FB	FOREGROUND_INTENSITY|FOREGROUND_RED
#define GB_FB	FOREGROUND_INTENSITY|FOREGROUND_GREEN
#define YB_FB	FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_GREEN
#define MB_FB	FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_BLUE

#define TIPS	"[��ʾ] "
#define WARN	"[����] "
#define ERR		"[����] "
#define DBG		"[����] "

constexpr auto MIN_FPS = 1E-1;			//�ɼ�¼��С֡��������0.1֡/��
constexpr auto MAX_TURBO_FPS = 30.0;	//�ɼ�¼���֡��������30֡/��
const vector<string> DRIVE_NAME = { "A:","B:","C:","D:","E:","F:",
"G:","H:","I:","J:","K:","L:","M:","N:","O:","P:","Q:",
"R:","S:","T:","U:","V:","W:","X:","Y:","Z:" };
typedef enum class _INSTRUCTION_CODE_ { ADD, RMOV, BGN }IstrCode;

//���ַ�������(string)�ָ�Ϊ�ַ�������
void SplitString(const string& src, const string& c, vector<string>& dst);

//�ж�������ַ����Ƿ�Ϊ�Ϸ����ļ�����
const bool IsValidFolderName(const char* pName);

//�ж�������ַ����Ƿ�Ϊ�Ϸ���·����
const bool IsValidPathName(const char* pName);

//����Ŀ���ַ������ɶ�Ӧ���ļ���
const bool RecursiveMkdir(const char* dir);

//��ȡ���в���
const bool GetBasicParams(string& mainPath, double& fps, double& recordT);

//��ȡ��-��-�յ��ַ��������ʾ
const string GetYMD();

//���������Ӧ��ʱ-��-���ַ��������ʾ
const string GetHMSFrom(const double& seconds);

//��õ�ǰ����ڿ���̨�е�λ��(X,Y)
const COORD WhereXY(COORD& r);

//���ÿ���̨�����й���λ��(X,Y)
void GotoXY(const COORD& r);

//����Ϣ�������׼���
void SendMsgToConsole(HANDLE out, string header, UINT color, string msg, bool isLineBreak = false);

//�ɿ���̨��ȡ��Ϣ
void GetIstrFromConsole(HANDLE out, IstrCode& code, unsigned& count, unsigned maxCount);

//��˫������ֵתΪ�ַ���
string DoubleToString(const double val, unsigned precision);
#endif // !_USER_IO_H
