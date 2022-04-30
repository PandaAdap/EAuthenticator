#pragma once

#include <WinInet.h>
#pragma comment(lib,"wininet.lib")
#include <afxinet.h>

#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")

#include <Strsafe.h>
#include <string>
#define MAX_SIZE 256

//Error Code
#define ERROR_INVALID_ACCOUNT 0x0A01
#define ERROR_INVALID_PERIODTIME 0x0A02
#define ERROR_INVALID_NASIP 0x0A03
#define ERROR_INVALID_DEVICE 0x0A04
#define ERROR_GET_VERIFYCODE_FAILED 0x0B01
#define ERROR_GET_ADAPTERINFO_FAILED 0x0B02
#define ERROR_CONNECT_FAILED 0x0C01
#define ERROR_NOUSERLOGIN 0x0D01

/// <summary></summary>
/// <param name="returnCode"><summary>Return a code(unsigned integer).</summary></param>
/// <param name="returnInfo"><summary>Return a string(CString) if necessary.</summary></param>
/// <param name="originalInfo"><summary>Return a string(CString) for log.</summary></param>
/// <return></return>
struct ReturnInfo
{
	UINT returnCode = 0;
	CString returnInfo = "";
	CString originalInfo = "";
};

class EAuth
{
private:

#ifdef _DEBUG
	CString username = "";
	CString password = "";
	CString clientip = "100.2.14.179";
	CString nasip = "119.146.175.80";
	CString mac = "B4-2E-99-E1-2F-64";
#else
	CString username = "";
	CString password = "";
	CString clientip = "";
	CString nasip = "";
	CString mac = "";
#endif

	CString Adapterlist[10] = { "" };
	CString secret = "Eshore!@#";
	CString iswifi = "4060";

	bool islogout = true;
	UINT GetAdapterCharacteristics(char* adapter_name);
	static UINT KeepActive(LPVOID lpParam);
	std::string EAuth::_2string(CString str);

	int device_count = 0;//Number of physical network adapter.
	int time_counter = 0;//Timer.(Second)
	
public:

	CString adapter;
	int active_period = 15;//Time interval for sending messages to the server to keep login state.(Minute) 

	int getAdapterInfo();
	int getNASIP(CString url);
	int startLogin(CString _username, CString _password, CString _nasip, int _active_period);

	int InitAccountInfo(CString _username, CString _password);
	int SetActivePeriodTime(int minute);

	ReturnInfo getVerifyCodeString();
	ReturnInfo Login(CString verifyCode);
	ReturnInfo doActive();
	ReturnInfo Logout();
	ReturnInfo reConnect();
};

