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

#define TIPS	"[提示] "
#define WARN	"[警告] "
#define ERR		"[错误] "
#define DBG		"[调试] "

constexpr auto MIN_FPS = 1E-1;			//可记录最小帧率限制在0.1帧/秒
constexpr auto MAX_TURBO_FPS = 30.0;	//可记录最大帧率限制在30帧/秒
const vector<string> DRIVE_NAME = { "A:","B:","C:","D:","E:","F:",
"G:","H:","I:","J:","K:","L:","M:","N:","O:","P:","Q:",
"R:","S:","T:","U:","V:","W:","X:","Y:","Z:" };
typedef enum class _INSTRUCTION_CODE_ { ADD, RMOV, BGN }IstrCode;

//将字符串对象(string)分割为字符串向量
void SplitString(const string& src, const string& c, vector<string>& dst);

//判断输入的字符串是否为合法的文件夹名
const bool IsValidFolderName(const char* pName);

//判断输入的字符串是否为合法的路径名
const bool IsValidPathName(const char* pName);

//根据目标字符串生成对应的文件夹
const bool RecursiveMkdir(const char* dir);

//获取运行参数
const bool GetBasicParams(string& mainPath, double& fps, double& recordT);

//获取年-月-日的字符串对象表示
const string GetYMD();

//获得秒数对应的时-分-秒字符串对象表示
const string GetHMSFrom(const double& seconds);

//获得当前光标在控制台中的位置(X,Y)
const COORD WhereXY(COORD& r);

//设置控制台窗口中光标的位置(X,Y)
void GotoXY(const COORD& r);

//将信息输出至标准输出
void SendMsgToConsole(HANDLE out, string header, UINT color, string msg, bool isLineBreak = false);

//由控制台读取信息
void GetIstrFromConsole(HANDLE out, IstrCode& code, unsigned& count, unsigned maxCount);

//将双精度数值转为字符串
string DoubleToString(const double val, unsigned precision);
#endif // !_USER_IO_H
