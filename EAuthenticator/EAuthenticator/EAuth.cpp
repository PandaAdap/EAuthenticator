#include "pch.h"
#include "EAuth.h"

#include <sys/timeb.h> 
#include <time.h>  
typedef long long int64;

#include "md5.h"
#include "json/json.h"
#include "HttpClient.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Private functions.

//Convert "CString" to "string".
std::string EAuth::_2string(CString str)
{
	return str.GetBuffer(0);
}

//Check whether the network adapter is a physical adapter.
UINT EAuth::GetAdapterCharacteristics(char* adapter_name)
{
	if (adapter_name == NULL || adapter_name[0] == 0)
		return 0;

	HKEY root = NULL;
	// 打开存储适配器信息的注册表根键
	if (ERROR_SUCCESS != RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}", 0, KEY_READ, &root))
		return 0;

	DWORD subkeys = 0;
	// 获取该键下的子键数
	if (ERROR_SUCCESS != RegQueryInfoKeyA(root, NULL, NULL, NULL, &subkeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL))
		subkeys = 100;

	DWORD ret_value = 0;
	for (DWORD i = 0; i < subkeys; i++)
	{
		// 每个适配器用一个子键存储，子键名为从 0 开始的 4 位数
		char subkey[MAX_SIZE];
		memset(subkey, 0, MAX_SIZE);
		StringCbPrintfA(subkey, MAX_SIZE, "%04u", i);

		// 打开该子键
		HKEY hKey = NULL;
		if (ERROR_SUCCESS != RegOpenKeyExA(root, subkey, 0, KEY_READ, &hKey))
			continue;

		// 获取该子键对应的适配器 ID，存于 name 中
		char name[MAX_PATH];
		DWORD type = 0;
		DWORD size = MAX_PATH;
		if (ERROR_SUCCESS != RegQueryValueExA(hKey, "NetCfgInstanceId", NULL, &type, (LPBYTE)name, &size))
		{
			RegCloseKey(hKey);
			continue;
		}

		// 对比该适配器 ID 是不是要获取特性的适配器 ID
		if (StrCmpIA(name, adapter_name) != 0)
		{
			RegCloseKey(hKey);
			continue;
		}

		// 读取该适配器的特性标志，该标志存储于值 Characteristics 中
		DWORD val = 0;
		size = 4;
		LSTATUS ls = RegQueryValueExA(hKey, "Characteristics", NULL, &type, (LPBYTE)&val, &size);
		RegCloseKey(hKey);

		if (ERROR_SUCCESS == ls)
		{
			ret_value = val;
			break;
		}
	}

	RegCloseKey(root);
	return ret_value;
}

//Thread for keep login state on server.
//Only for "startLogin(Params)".
UINT EAuth::KeepActive(LPVOID lpParam)
{
	EAuth* obj = (EAuth*)lpParam;

	int retry_count = 0;

	while (!obj->islogout && retry_count < 3)
	{
		if (obj->time_counter >= (obj->active_period * 1))//Time to send message to keep active.
		{
			if (obj->doActive().returnCode != 0)
			{
				if (obj->reConnect().returnCode == 0)
				{
					obj->time_counter = 0;
					retry_count = 0;
				}
				else
				{
					retry_count++;
				}
			}
			else
			{
				obj->time_counter = 0;
				retry_count = 0;
			}

		}
		Sleep(1000);
		obj->time_counter++;
	}

	if (retry_count == 0)
	{
		obj->islogout = true;
	}

	return 0;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Public functions.

//Initialize account information.
int EAuth::InitAccountInfo(CString _username, CString _password)
{
	if (_username == "" || _password == "")
		return -1;

	username = _username;
	password = _password;
	return 0;
}

//Set the time interval for sending messages to the server to keep login state.
int EAuth::SetActivePeriodTime(int minute)
{
	if (minute < 10 || minute > 20)
		return ERROR_INVALID_PERIODTIME;

	active_period = minute;
	return 0;
}

//TODO: "auto" to get nasip automatic.
int EAuth::getNASIP(CString url)
{
	if (url == "auto")
	{
		/*Auto get nasip here*/
	}
	else
	{		
		nasip = url;
	}
	return 0;
}

//Query all physical network adapter and get the information.
//Must call this function before calling "getVerifyCodeString()".
int EAuth::getAdapterInfo()
{
	CString strMacTemp, strAdaterName, strAdapterIP;

	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL)
		return -1;
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
	{
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
		if (pAdapterInfo == NULL)
			return -1;
	}

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == NO_ERROR)
	{
		for (PIP_ADAPTER_INFO pAdapter = pAdapterInfo; pAdapter != NULL; pAdapter = pAdapter->Next)
		{
			UINT flag = GetAdapterCharacteristics(pAdapter->AdapterName);
			bool is_physical = ((flag & 0x4) == 0x4);
			if (!is_physical)
				continue;

			if (pAdapter->AddressLength != 6)
				continue;
			strAdaterName = pAdapter->Description;
			strAdapterIP = pAdapter->IpAddressList.IpAddress.String;
			strMacTemp.Format("%02X:%02X:%02X:%02X:%02X:%02X",
				int(pAdapter->Address[0]),
				int(pAdapter->Address[1]),
				int(pAdapter->Address[2]),
				int(pAdapter->Address[3]),
				int(pAdapter->Address[4]),
				int(pAdapter->Address[5]));

			Adapterlist[device_count] = strAdaterName + "&1" + strAdapterIP + "&2" + strMacTemp + "\r\n";
			device_count++;
		}
	}
	free(pAdapterInfo);

	return 0;
}

//Get login verify code from the server.
ReturnInfo EAuth::getVerifyCodeString()
{
	ReturnInfo returninfo;
	if (Adapterlist[0] == "")
	{
		returninfo.returnCode = ERROR_INVALID_DEVICE;
		returninfo.returnInfo = "Network adapters not found.";
		return returninfo;
	}

	CHttpClient hc;
	md5::MD5 crypto_md5;
	CString url = "http://enet.10000.gd.cn:10001/client/challenge",
		timestamp,
		md5String,
		postback = "",
		strConvert;

	long long time_last;
	struct timeb t1;

	for (int device = 0; device < device_count; device++)
	{
		clientip = Adapterlist[device].Mid(Adapterlist[device].Find("&1") + 2, Adapterlist[device].Find("&2") - 2 - Adapterlist[device].Find("&1"));
		if (clientip == "0.0.0.0")continue;
		mac = Adapterlist[device].Mid(Adapterlist[device].Find("&2") + 2, Adapterlist[device].Find("\r\n") - 2 - Adapterlist[device].Find("&2"));

		time_last = time(NULL);
		ftime(&t1);
		timestamp.Format("%lld", t1.time * 1000 + t1.millitm);

		md5String = crypto_md5.digestString((clientip + nasip + mac + timestamp + secret).GetBuffer(0));
		md5String.MakeUpper();

		Json::Value post_params;
		post_params["username"] = _2string(username);
		post_params["clientip"] = _2string(clientip);
		post_params["nasip"] = _2string(nasip);
		post_params["mac"] = _2string(mac);
		post_params["iswifi"] = _2string(iswifi);
		post_params["timestamp"] = _2string(timestamp + "");
		post_params["authenticator"] = _2string(md5String);

		strConvert = post_params.toStyledString().c_str();
		returninfo.returnCode = hc.HttpPost(url, strConvert, postback);//{"challenge":"3RYP","resinfo":"this user is ok!","rescode":"0"}
		if (returninfo.returnCode != 0)
		{
			returninfo.returnCode = ERROR_CONNECT_FAILED;
			returninfo.returnInfo = "Unable to connect the server.";
			continue;
		}

		returninfo.originalInfo = postback;

		Json::Value verifyInfo;
		Json::CharReaderBuilder b;
		Json::CharReader* reader(b.newCharReader());
		JSONCPP_STRING errs;
		bool ok = reader->parse(postback.GetBuffer(0), postback.GetBuffer(0) + std::strlen(postback.GetBuffer(0)), &verifyInfo, &errs);
		if (ok && errs.size() == 0)
		{
			returninfo.returnCode = _ttoi(verifyInfo["rescode"].asCString());
			if (returninfo.returnCode == 0)
			{
				adapter = Adapterlist[device].Mid(0, Adapterlist[device].Find("&1"));
				returninfo.returnInfo = verifyInfo["challenge"].asCString(); 
				delete reader;
				break;
			}
		}
		delete reader;
	}
	return returninfo;
}

//Login directly without loginfo.
int EAuth::startLogin(CString _username,CString _password,CString _nasip,int _active_period)
{
	if (InitAccountInfo(username, password) == ERROR_INVALID_ACCOUNT)
		return ERROR_INVALID_ACCOUNT;

	if (SetActivePeriodTime(15) == ERROR_INVALID_PERIODTIME)
		return ERROR_INVALID_PERIODTIME;

	getNASIP(_nasip);

	if (getAdapterInfo() == -1)
		return ERROR_GET_ADAPTERINFO_FAILED;

	ReturnInfo verifyCode = getVerifyCodeString();

	if (verifyCode.returnCode == ERROR_GET_VERIFYCODE_FAILED)
		return ERROR_GET_VERIFYCODE_FAILED;

	islogout = false;

	ReturnInfo returninfo;

	returninfo = Login(verifyCode.returnInfo);
	if (returninfo.returnCode == FAILURE || returninfo.returnCode == OUTTIME)
	{
		islogout = true;
		return ERROR_CONNECT_FAILED;
	}
	AfxBeginThread(KeepActive, this);

	return 0;
}

//Login to the server.
ReturnInfo EAuth::Login(CString verifyCode)
{
	ReturnInfo returninfo;

	CHttpClient hc;
	md5::MD5 crypto_md5;
	CString url = "http://enet.10000.gd.cn:10001/client/login",
		timestamp,
		md5String,
		loginString,
		strConvert;

	long long time_last;
	time_last = time(NULL);
	struct timeb t1;

	ftime(&t1);
	timestamp.Format("%lld", t1.time * 1000 + t1.millitm);

	md5String = crypto_md5.digestString((clientip + nasip + mac + timestamp + verifyCode + secret).GetBuffer(0));
	md5String.MakeUpper();

	Json::Value post_params;
	post_params["username"] = _2string(username);
	post_params["password"] = _2string(password);
	post_params["verificationcode"] = "";
	post_params["clientip"] = _2string(clientip);
	post_params["nasip"] = _2string(nasip);
	post_params["mac"] = _2string(mac);
	post_params["iswifi"] = _2string(iswifi);
	post_params["timestamp"] = _2string(timestamp + "");
	post_params["authenticator"] = _2string(md5String);

	strConvert = post_params.toStyledString().c_str();
	returninfo.returnCode = hc.HttpPost(url, strConvert, loginString);
	if (returninfo.returnCode != 0)
	{
		returninfo.returnCode = ERROR_CONNECT_FAILED;
		returninfo.returnInfo = "Unable to connect the server.";
		return returninfo;
	}
	returninfo.originalInfo = loginString;

	Json::Value loginResult;
	Json::CharReaderBuilder b;
	Json::CharReader* reader(b.newCharReader());
	JSONCPP_STRING errs;
	bool ok = reader->parse(loginString.GetBuffer(0), loginString.GetBuffer(0) + std::strlen(loginString.GetBuffer(0)), &loginResult, &errs);
	if (ok && errs.size() == 0)
	{
		returninfo.returnCode = _ttoi(loginResult["rescode"].asCString());
	}
	delete reader;

	return returninfo;

	/*Java code
	url = "http://enet.10000.gd.cn:10001/client/login";
	timestamp = System.currentTimeMillis() + "";
	md5String = MD5Util.MD5(clientip + nasip + mac + timestamp + verifyCode + secret);

	JSONObject jsonObject = new JSONObject();
	jsonObject.put("username", username);
	jsonObject.put("password", password);
	jsonObject.put("verificationcode", "");
	jsonObject.put("clientip", clientip);
	jsonObject.put("nasip", nasip);
	jsonObject.put("mac", mac);
	jsonObject.put("iswifi", iswifi);
	jsonObject.put("timestamp", timestamp);
	jsonObject.put("authenticator", md5String);

	String loginString = HttpUtil.doPost(url, jsonObject.toString());
	*/
}

//Keep connecting.
ReturnInfo EAuth::doActive()
{
	ReturnInfo returninfo;
	CHttpClient hc;
	md5::MD5 crypto_md5;

	CString url = "http://enet.10000.gd.cn:8001/hbservice/client/active",
		post_param,
		timestamp,
		md5String,
		keepactiveString;

	long long time_last;
	struct timeb t1;

	time_last = time(NULL);
	ftime(&t1);
	timestamp.Format("%lld", t1.time * 1000 + t1.millitm);

	md5String = crypto_md5.digestString((clientip + nasip + mac + timestamp + secret).GetBuffer(0));
	md5String.MakeUpper();

	post_param =
		"?username=" + username
		+ "&clientip=" + clientip
		+ "&nasip=" + nasip
		+ "&mac=" + mac
		+ "&timestamp=" + timestamp
		+ "&authenticator=" + md5String;

	returninfo.returnCode = hc.HttpGet(url + post_param, "", keepactiveString);


	if (returninfo.returnCode != 0)
	{
		returninfo.returnCode = ERROR_CONNECT_FAILED;
		returninfo.returnInfo = "Unable to connect the server.";
		return returninfo;
	}

	returninfo.originalInfo = keepactiveString;

	Json::Value keepactiveResult;
	Json::CharReaderBuilder b;
	Json::CharReader* reader(b.newCharReader());
	JSONCPP_STRING errs;
	bool ok = reader->parse(keepactiveString.GetBuffer(0), keepactiveString.GetBuffer(0) + std::strlen(keepactiveString.GetBuffer(0)), &keepactiveResult, &errs);
	if (ok && errs.size() == 0)
	{
		returninfo.returnInfo = keepactiveResult["rescode"].asCString();
	}
	delete reader;

	return returninfo;
	/*Java code
	timestamp = System.currentTimeMillis() + "";
	md5String = MD5Util.MD5(clientip + nasip + mac + timestamp + secret);
	url = "http://enet.10000.gd.cn:8001/hbservice/client/active";

	String param = "username=" + username + "&clientip=" + clientip + "&nasip=" + nasip + "&mac=" + mac + "&timestamp=" + timestamp + "&authenticator="+ md5String;
	String activeString = HttpUtil.doGet(url, param);

	int activeTimeInt = Integer.parseInt(activeTime);

		TimerTask timerTask = new TimerTask() {
			String activeString;
			JSONObject jsonObject;
			String rescode;

			@Override
			public void run() {
				try {
					activeString = EsurfingService.this.keepConnection();
					jsonObject = new JSONObject(activeString);
					rescode = (String) jsonObject.opt("rescode");
					if ("0".equals(rescode)) {
						System.out.println(PrintUtil.printPrefix() + "维持连接成功！");
					} else {
						String resinfo = jsonObject.optString("resinfo");
						System.out.println(PrintUtil.printPrefix() + "维持连接失败：" + resinfo);
						reConnect();
					}
				} catch (Exception e) {
					System.out.println(PrintUtil.printPrefix() + "维持连接时出现异常！");
					// 停止执行定时任务
					this.cancel();
					reConnect();
					return;
				}
			}
		};

		Timer timer = new Timer();
		timer.schedule(timerTask, activeTimeInt * 60000, activeTimeInt * 60000);
	*/
}

//Logout of the server.
ReturnInfo EAuth::Logout()
{
	ReturnInfo returninfo;

	islogout = true;

	CHttpClient hc;
	md5::MD5 crypto_md5;
	CString url = "http://enet.10000.gd.cn:10001/client/logout",
		timestamp,
		md5String,
		logoutString,
		strConvert;

	long long time_last;
	time_last = time(NULL);
	struct timeb t1;
	ftime(&t1);
	timestamp.Format("%lld", t1.time * 1000 + t1.millitm);

	md5String = crypto_md5.digestString((clientip + nasip + mac + timestamp +  secret).GetBuffer(0));
	md5String.MakeUpper();

	Json::Value post_params;
	post_params["clientip"] = _2string(clientip);
	post_params["nasip"] = _2string(nasip);
	post_params["mac"] = _2string(mac);
	post_params["timestamp"] = _2string(timestamp + "");
	post_params["authenticator"] = _2string(md5String);

	strConvert = post_params.toStyledString().c_str();
	returninfo.returnCode = hc.HttpPost(url, strConvert, logoutString);
	if (returninfo.returnCode != 0)
	{
		returninfo.returnCode = ERROR_CONNECT_FAILED;
		returninfo.returnInfo = "Unable to connect the server.";
		return returninfo;
	}
	returninfo.originalInfo = logoutString;

	Json::Value logoutResult;
	Json::CharReaderBuilder b;
	Json::CharReader* reader(b.newCharReader());
	JSONCPP_STRING errs;
	bool ok = reader->parse(logoutString.GetBuffer(0), logoutString.GetBuffer(0) + std::strlen(logoutString.GetBuffer(0)), &logoutResult, &errs);
	if (ok && errs.size() == 0)
	{
		returninfo.returnCode = _ttoi(logoutResult["rescode"].asCString());
	}
	delete reader;

	return returninfo;

	/*Java code
	Properties properties = new Properties();
	FileInputStream fileInputStream = new FileInputStream(debugConfig);

	properties.load(fileInputStream);
	String lastnasip = properties.getProperty("lastnasip");
	String lastclientip = properties.getProperty("lastclientip");
	String lastmac = properties.getProperty("lastmac");
	fileInputStream.close();

	url = "http://enet.10000.gd.cn:10001/client/logout";
	timestamp = System.currentTimeMillis() + "";
	md5String = MD5Util.MD5(lastclientip + lastnasip + lastmac + timestamp + secret);

	JSONObject jsonObject = new JSONObject();
	jsonObject.put("clientip", lastclientip);
	jsonObject.put("nasip", lastnasip);
	jsonObject.put("mac", lastmac);
	jsonObject.put("timestamp", timestamp);
	jsonObject.put("authenticator", md5String);

	String logoutString = HttpUtil.doPost(url, jsonObject.toString());
	*/
}

//Reconnect to the server if keep active failed.
ReturnInfo EAuth::reConnect()
{
	ReturnInfo returninfo;
	if (islogout)
	{
		returninfo.returnCode = ERROR_NOUSERLOGIN;
		returninfo.returnInfo = "No user login.";
		return returninfo;
	}
	ReturnInfo verifyCode = getVerifyCodeString();
	return Login(verifyCode.returnInfo);
}
