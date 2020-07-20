#include"userio.h"
#include<algorithm>
#include<ctime>
#include<io.h>
#include<direct.h>
#include<Windows.h>
#include<sstream>

//将字符串对象(string)分割为字符串向量
//src: 源字符串对象
//c: 分割字符(串)
//dst: 分割后的字符串向量
//code visit: https://www.cnblogs.com/dfcao/p/cpp-FAQ-split.html @egmkang
void SplitString(const string& src, const string& c, vector<string>& dst)
{
	size_t pos1, pos2;
	pos2 = src.find(c);
	pos1 = 0;
	while (string::npos != pos2)
	{
		dst.push_back(src.substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = src.find(c, pos1);
	}
	if (pos1 != src.length())
		dst.push_back(src.substr(pos1));
}

//判断输入的字符串是否为合法的文件夹名
//pName:字符数组(C-Type)
//code visit:https://blog.csdn.net/WEI_GW2012/article/details/43988529
//copy right: CC 4.0 BY-SA
const bool IsValidFolderName(const char* pName)
{
	bool ret = true;
	size_t u32Length = 0, u32Index = 0;
	char u8SpecialChar[] = { '\\','<','>',':','"','/','|','?','*','\0' };
	char u8CtrlCharBegin = '0', u8CtrlCharEnd = '9';
	if (pName == NULL)
	{
		ret = false;
	}
	else
	{
		u32Length = strlen(pName);
		if (u32Length >= MAX_FOLDER_NAME_LEN)
			ret = false;
	}

	for (u32Index = 0; (u32Index < u32Length) && ret;
		u32Index++)
	{
		if (strchr(u8SpecialChar, pName[u32Index]) != NULL)
		{
			ret = false;
		}
	}
	return ret;
}

//判断输入的字符串是否为合法的文件夹名
//pName:字符数组(C-Type)
const bool IsValidPathName(const char* pName)
{
	string path(pName);
	vector<string> dst;
	//将字符串对象按"\\"分割为字符串向量
	SplitString(path, "\\", dst);
	//判断第一个字符串是否代表驱动器
	if (dst.size() == 0 || find(DRIVE_NAME.begin(), DRIVE_NAME.end(), dst[0]) == DRIVE_NAME.end())
		return false;
	for (size_t k = 1; k < dst.size(); k++) {
		if (!IsValidFolderName(dst[k].c_str()))
			return false;
	}
	return true;
}

//根据文件路径生成文件夹
//dir: 字符数组(C-Type)
const bool RecursiveMkdir(const char* dir)
{
	//分解路径名E:\\AA\\BB\\CC
	string str = dir;
	size_t index = 0;
	int i = 0;
	while (true)
	{
		//size_type由string类类型和vector类类型定义的类型，用以保存任意string对象或vector对象的长度
		// find 函数 返回"\\" 在index 中的下标位置 
		string::size_type pos = str.find("\\", index);
		string str1;
		str1 = str.substr(0, pos);//str.substr(a,b)是从a开始截取b个字符。
		if (pos != -1 && i > 0)
		{
			if (_access(str1.c_str(), 0) == -1)
			{
				_mkdir(str1.c_str());
			}
		}
		if (pos == -1)
		{
			break;
		}
		i++;
		index = pos + 1;
	}
	return true;
}

//获取主程序运行参数
const bool GetBasicParams(string& mainPath, double& fps, double& recordT)
{
	auto hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	//从键盘读入必要参数
	bool retry = true;
	while (retry) {
		SendMsgToConsole(
			hConsoleOutput, 
			TIPS, 
			GB_FB, 
			"请输入存储主目录(若不存在将自动创建)："
		);
		cin >> mainPath;
		if (IsValidPathName(mainPath.c_str())) {
			retry = false;
		}
		else {
			SendMsgToConsole(
				hConsoleOutput,
				ERR,
				RB_FB,
				"文件夹格式有误，请重新输入.",true
			);
			mainPath.clear();
			cin.clear();
			while (cin.get() != '\n')continue;
		}
	}
	retry = !retry;
	while (retry)
	{
		SendMsgToConsole(
			hConsoleOutput,
			TIPS,
			GB_FB,
			"请输入记录帧率(FPS)："
		);
		cin >> fps;
		if (cin.good() && fps>=MIN_FPS && fps<=MAX_TURBO_FPS) {
			retry = false;
		}
		else {
			SendMsgToConsole(
				hConsoleOutput,
				ERR,
				RB_FB,
				"记录帧率范围(0.1 FPS--30.0 FPS)，请重新输入.", true
			);
			cin.clear();
			while (cin.get() != '\n')continue;
		}
	}
	retry = !retry;
	while (retry) {
		SendMsgToConsole(
			hConsoleOutput,
			TIPS,
			GB_FB,
			"请输入记录总时长(小时)："
		);
		cin >> recordT;
		if (cin.good() && recordT > 0 && recordT * 3600 > 1.0 / fps)
		{
			recordT *= 3600;	//将记录时长转换为秒
			retry = false;
		}
		else {
			SendMsgToConsole(
				hConsoleOutput,
				ERR,
				RB_FB,
				"记录时长不符合规范，请重新输入.", true
			);
			cin.clear();
			while (cin.get() != '\n')continue;
		}
	}
	return true;
}

//获取当前系统时间 年-月-日 字符串对象
const string GetYMD()
{
	string ymd;
	time_t nowtime;
	struct tm p;
	time(&nowtime);
	localtime_s(&p, &nowtime);
	//本地时间转换
	ymd = to_string(p.tm_year + 1900) + "_"
		+ to_string(p.tm_mon + 1) + "_"
		+ to_string(p.tm_mday);
	return ymd;
}

//利用输入秒数转换为 时:分:秒 字符串对象
const string GetHMSFrom(const double& seconds)
{
	int ihour = (int)seconds / 3600;
	int	iminute = ((int)seconds - ihour * 3600) / 60;
	int	isecond = (int)seconds - ihour * 3600 - iminute * 60;
	//所有格式为：个位数字输出在十位补0
	string shour = ihour < 10 ? "0" + to_string(ihour) : to_string(ihour);
	string sminute = iminute < 10 ? "0" + to_string(iminute) : to_string(iminute);
	string ssecond = isecond < 10 ? "0" + to_string(isecond) : to_string(isecond);
	return shour + ":" + sminute + ":" + ssecond;
}

//获得当前光标在控制台中的位置(X,Y)
const COORD WhereXY(COORD& r)
{
	CONSOLE_SCREEN_BUFFER_INFO pBuffer;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &pBuffer);
	r.X = pBuffer.dwCursorPosition.X + 1;
	r.Y = pBuffer.dwCursorPosition.Y + 1;
	return r;
}

void GotoXY(const COORD& r)
{
	COORD c;
	c.X = r.X - 1;
	c.Y = r.Y - 1;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void SendMsgToConsole(HANDLE out, string header, UINT color, string msg, bool isLineBreak)
{
	SetConsoleTextAttribute(out, color);
	cout << header;
	SetConsoleTextAttribute(out, WB_FB);
	if (isLineBreak) {
		cout << msg << endl;
	}
	else {
		cout << msg;
	}
}

void GetIstrFromConsole(HANDLE out, IstrCode& code, unsigned& count, unsigned maxCount)
{
	string cmd;
	vector<string> cmdv;
	bool reTry = true;
	while (reTry)
	{
		SendMsgToConsole(
			out,
			TIPS,
			GB_FB,
			"请输入系统指令([add/remove/begin] {count})："
		);
		cin >> cmd;
		if (!cin.good()) {
			SendMsgToConsole(
				out,
				ERR,
				RB_FB,
				"无法识别的指令，请重新输入.", true
			);
			cin.clear();
			while (cin.get() != '\n')continue;
			continue;
		}
		//获取输入指令
		SplitString(cmd, " ", cmdv);

		if (cmdv[0] == "add") {
			code = IstrCode::ADD;
		}
		else if (cmdv[0] == "remove") {
			code = IstrCode::RMOV;
		}
		else if (cmdv[0] == "begin") {
			code = IstrCode::BGN;
		}
		else {
			SendMsgToConsole(
				out,
				ERR,
				RB_FB,
				"无法识别的指令，请重新输入.", true
			);
			cin.clear();
			while (cin.get() != '\n')continue;
			continue;
		}
		//
		if (cmdv.size() == 2) {
			auto tmpCnt = (unsigned)stoi(cmdv[1]);
			if (tmpCnt <= maxCount) {
				count = tmpCnt;
			}
		}
		else {
			count = 1;
		}
		reTry = false;
	}
}

string DoubleToString(const double val, unsigned precision)
{
	std::ostringstream out;
		if (precision > 0)
			out.precision(precision);
		out << val;
	return out.str();
}
