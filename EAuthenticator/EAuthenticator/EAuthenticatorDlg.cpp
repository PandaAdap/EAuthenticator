
// EAuthenticatorDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "EAuthenticator.h"
#include "EAuthenticatorDlg.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define Message WM_USER+1001
#define Message_Login_Success WM_USER+1002
#define Message_Login_Failed WM_USER+1003
#define Message_Logout_Log WM_USER+1003
#define Message_KeepActive_Log WM_USER+1004

#define WM_NotiMenu WM_USER+1005

UINT Thread_Login(LPVOID lpParam);
UINT Thread_Logout(LPVOID lpParam);
UINT Thread_KeepActive(LPVOID lpParam);

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CEAuthenticatorDlg 对话框

CEAuthenticatorDlg::CEAuthenticatorDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EAUTHENTICATOR_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEAuthenticatorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEAuthenticatorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(Message, &CEAuthenticatorDlg::OnUserMsg)
	ON_MESSAGE(WM_NotiMenu, &CEAuthenticatorDlg::OnTray)
	ON_BN_CLICKED(IDC_LoginSwitch, &CEAuthenticatorDlg::OnBnClickedLoginswitch)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_COMMAND(ID_NotiMenu_LoginSwitch, &CEAuthenticatorDlg::OnNotimenuLoginswitch)
	ON_COMMAND(ID_NotiMenu_Quit, &CEAuthenticatorDlg::OnNotimenuQuit)
	ON_COMMAND(ID_NotiMenu_NASIPStat, &CEAuthenticatorDlg::OnNotimenuNasipstat)
END_MESSAGE_MAP()


// CEAuthenticatorDlg 消息处理程序

BOOL CEAuthenticatorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	notifyicon.cbSize = sizeof(NOTIFYICONDATA);//结构体的大小
	notifyicon.hWnd = this->m_hWnd;//接收托盘消息的窗口句柄 this是当前dialog
	notifyicon.uID = IDR_MAINFRAME;//定义的托盘图标ID
	notifyicon.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;//设置属性，下面的三个属性是否有效
	lstrcpyn(notifyicon.szTip, "EAuthenticator", sizeof("EAuthenticator"));//图标上的提示字符串
	notifyicon.uCallbackMessage = WM_NotiMenu;//自定义的消息，
	notifyicon.hIcon = ::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));//显示在系统托盘上的图标
	Shell_NotifyIcon(NIM_ADD, &notifyicon);// 向任务栏的状态栏发送添加托盘图标的消息

	notimenu.LoadMenu(IDR_NotiMenu);

	SetTimer(0, 1000, NULL);

	/*
	//Read from encrypted file.
	*/
	
	if (!(username == "null" || password == "null" || nasip == "null") && !(username == "" || password == "" || nasip == ""))
	{
		notimenu.GetSubMenu(0)->ModifyMenu(0, MF_BYPOSITION, 0, "账户: " + username);
		notimenu.GetSubMenu(0)->ModifyMenu(3, MF_BYPOSITION, 3, nasip);

		if (autologin == true)
		{
			OnBnClickedLoginswitch();
			return TRUE;
		}
	}
	
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CEAuthenticatorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CEAuthenticatorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CEAuthenticatorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CEAuthenticatorDlg::InitNetworkAdapterInfo()
{
	free(m_pIfTable);
	m_dwSize = sizeof(MIB_IFTABLE);
	m_pIfTable = (MIB_IFTABLE*)malloc(m_dwSize);
	int rtn;
	rtn = GetIfTable(m_pIfTable, &m_dwSize, FALSE);
	if (rtn == ERROR_INSUFFICIENT_BUFFER)	//如果函数返回值为ERROR_INSUFFICIENT_BUFFER，说明m_pIfTable的大小不够
	{
		free(m_pIfTable);
		m_pIfTable = (MIB_IFTABLE*)malloc(m_dwSize);	//用新的大小重新开辟一块内存
	}
	//获取当前所有的连接，并保存到m_connections容器中
	m_connections.clear();
	GetIfTable(m_pIfTable, &m_dwSize, FALSE);
	for (unsigned int i{}; i < m_pIfTable->dwNumEntries; i++)
	{
		std::string descr = (const char*)m_pIfTable->table[i].bDescr;
		if (m_pIfTable->table[i].dwInOctets > 0 || m_pIfTable->table[i].dwOutOctets > 0 || descr == m_connection_name)		//查找接收或发送数据量大于0的连接和上次选择的连接
		{
			m_connections.emplace_back(i, descr, m_pIfTable->table[i].dwInOctets, m_pIfTable->table[i].dwOutOctets);
		}
	}
	if (m_connections.empty())
		m_connections.emplace_back(0, std::string((const char*)m_pIfTable->table[0].bDescr), 0, 0);

	for (size_t i{}; i < m_connections.size(); i++)
	{
		if (m_connections[i].description == m_connection_name)
			m_connection_selected = i;
	}
}

LRESULT CEAuthenticatorDlg::OnTray(WPARAM wParam, LPARAM lParam)
{
	UINT MouseMsg = (UINT)lParam;
	if (MouseMsg == WM_RBUTTONDOWN) 
	{
		CMenu* pPopup = notimenu.GetSubMenu(0);
		CPoint point;
		GetCursorPos(&point);
		SetForegroundWindow();
		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_VERTICAL,
			point.x - 1, point.y, AfxGetApp()->m_pMainWnd, TPM_LEFTALIGN);
	}

	if (MouseMsg == WM_LBUTTONDOWN)
	{   
		if (iswinshow)
		{
			ShowWindow(SW_HIDE);
			iswinshow = false;
		}
		else
		{
			iswinshow = true;
			ShowWindow(SW_SHOW);
		}
		
	}
	return 0;
}

BOOL CEAuthenticatorDlg::ShowBalloonTip(LPCTSTR szMsg, LPCTSTR szTitle, UINT uTimeout, DWORD dwInfoFlags)
{
	notifyicon.uTimeout = uTimeout;
	notifyicon.dwInfoFlags = dwInfoFlags;
	strcpy_s(notifyicon.szInfo, sizeof(notifyicon.szInfo), szMsg ? szMsg : _T(""));
	strcpy_s(notifyicon.szInfoTitle, sizeof(notifyicon.szInfoTitle), szTitle ? szTitle : _T(""));
	return Shell_NotifyIcon(NIM_MODIFY, &notifyicon);
}

LRESULT CEAuthenticatorDlg::OnUserMsg(WPARAM wParam, LPARAM lParam)
{
	if (lParam == Message_Login_Success)
	{
		//Start KeepActive thread.
		m_connection_name = eauth.adapter;
		SetTimer(1, 1000, NULL);//Netflow
		iswinshow = false;

		is_threadrun = false;
		ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LoginSwitch)->EnableWindow(TRUE);
		SetDlgItemText(IDC_LoginSwitch, "登出");
		AfxBeginThread(Thread_KeepActive, this);
	}
	else if (lParam == Message_Login_Failed)
	{
		KillTimer(1);
		netflow_in = 0, netflow_out = 0, last_netflow_in = 0, last_netflow_out = 0;
		is_threadrun = false;
		GetDlgItem(IDC_LoginSwitch)->EnableWindow(TRUE);
		GetDlgItem(IDC_User)->EnableWindow(TRUE);
		GetDlgItem(IDC_Password)->EnableWindow(TRUE);
		SetDlgItemText(IDC_LoginSwitch, "登入");
	}
	else if (lParam == Message_Logout_Log)
	{
		KillTimer(1);
		netflow_in = 0, netflow_out = 0, last_netflow_in = 0, last_netflow_out = 0;
		is_threadrun = false;
		GetDlgItem(IDC_LoginSwitch)->EnableWindow(TRUE);
		GetDlgItem(IDC_User)->EnableWindow(TRUE);
		GetDlgItem(IDC_Password)->EnableWindow(TRUE);
		SetDlgItemText(IDC_LoginSwitch, "登入");
	}
	else if (lParam == Message_KeepActive_Log)
	{
		KillTimer(1);
		netflow_in = 0, netflow_out = 0, last_netflow_in = 0, last_netflow_out = 0;
		ShowBalloonTip("异常下线!", "", 3000, 1);
		is_threadrun = false;
		GetDlgItem(IDC_LoginSwitch)->EnableWindow(TRUE);
		GetDlgItem(IDC_User)->EnableWindow(TRUE);
		GetDlgItem(IDC_Password)->EnableWindow(TRUE);
		SetDlgItemText(IDC_LoginSwitch, "登入");
	}
	return LRESULT();
}

void CEAuthenticatorDlg::Log(CString str)
{
	CString time_b;
	time_b.Format("[%02d:%02d:%02d]",time_hour, time_minute, time_second);

	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_Log);
	int iLen;
	iLen = pEdit->GetWindowTextLength();
	pEdit->SetSel(iLen, iLen, TRUE);
	pEdit->ReplaceSel(time_b + "\r\n" + str + "\r\n", FALSE);
	pEdit->LineScroll(pEdit->GetLineCount(), 0);
}

void CEAuthenticatorDlg::OnBnClickedLoginswitch()
{
	if (nasip == "" || nasip == "null")
	{
		NASIP nasipDlg;
		nasipDlg.DoModal();
		nasip = nasipDlg.nasip;

		if (nasip == "")
		{
			AfxMessageBox("NASIP 无效!");
			return;
		}
			
	}
	ReturnInfo info;
	GetDlgItem(IDC_LoginSwitch)->EnableWindow(FALSE);
	
	if (islogout == true)
	{
		SetDlgItemText(IDC_LoginSwitch, "登入中...");
		CString username, password;
		GetDlgItemText(IDC_User, username);
		GetDlgItemText(IDC_Password, password);

		GetDlgItem(IDC_User)->EnableWindow(FALSE);
		GetDlgItem(IDC_Password)->EnableWindow(FALSE);

		if (eauth.InitAccountInfo(username, password) == -1)
			return;
		if (eauth.SetActivePeriodTime(12) == -1)
			return;
		eauth.getNASIP(nasip);

		//Write to config file

		notimenu.ModifyMenu(0, MF_BYPOSITION, ID_NotiMenu_Username, "账户: " + username);
		notimenu.ModifyMenu(0, MF_BYPOSITION, ID_NotiMenu_NASIP, nasip);

		is_threadrun = true;
		AfxBeginThread(Thread_Login, this);
	}
	else
	{
		is_threadrun = true;
		SetDlgItemText(IDC_LoginSwitch, "登出中...");
		AfxBeginThread(Thread_Logout, this);
	}
}

void CEAuthenticatorDlg::OnOK()
{
	OnBnClickedLoginswitch();
}

UINT Thread_Login(LPVOID lpParam)
{
	CEAuthenticatorDlg* obj = (CEAuthenticatorDlg*)lpParam;
	ReturnInfo info;

	if (obj->eauth.getAdapterInfo() == -1)
	{
		obj->Log("获取网络适配器信息失败!");
		::SendMessage(obj->GetSafeHwnd(), Message, NULL, Message_Login_Failed);
		return -1;
	}

	obj->Log("获取密钥...");
	info = obj->eauth.getVerifyCodeString();
	if (info.returnCode != 0)
	{
		obj->Log(info.returnInfo + "\r\n");
		::SendMessage(obj->GetSafeHwnd(),Message, NULL, Message_Login_Failed);
		return -1;
	}

	obj->Log("登入...");
	info = obj->eauth.Login(info.returnInfo);
	if (info.returnCode != 0)
	{
		obj->Log(info.returnInfo + "\r\n");
		::SendMessage(obj->GetSafeHwnd(), Message, NULL, Message_Login_Failed);
		return -1;
	}
	//Login success.
	obj->Log("登入成功.");
	obj->islogout = false;
	::SendMessage(obj->GetSafeHwnd(), Message, NULL, Message_Login_Success);

	return 0;
}

UINT Thread_Logout(LPVOID lpParam)
{
	CEAuthenticatorDlg* obj = (CEAuthenticatorDlg*)lpParam;
	ReturnInfo info;

	info = obj->eauth.Logout();
	if (info.returnCode != 0)
	{
		obj->Log("登出失败.\r\n");
		obj->islogout = true;
		::SendMessage(obj->GetSafeHwnd(), Message, NULL, Message_Logout_Log);
		return -1;
	}
	obj->islogout = true;
	::SendMessage(obj->GetSafeHwnd(), Message, NULL, Message_Logout_Log);
	obj->Log("登出成功.");
	return 0;
}

UINT Thread_KeepActive(LPVOID lpParam)
{
	CEAuthenticatorDlg* obj = (CEAuthenticatorDlg*)lpParam;
	ReturnInfo info;
	int retry_count = 0;

	while (!obj->islogout && retry_count < 3)
	{
		if (obj->counter >= (obj->eauth.active_period * 60))//Time to send message to keep active.
		{
			info = obj->eauth.doActive();
			if (info.returnCode != 0)
			{
				obj->Log("保持连接失败.\r\n");

				info = obj->eauth.reConnect();
				if (info.returnCode == 0)
				{
					obj->Log("尝试重新登入成功.\r\n");
					obj->counter = 0;
					retry_count = 0;
				}
				else
				{
					obj->Log("尝试重新登入失败.\r\n");
					retry_count++;
				}
			}
			else
			{
				obj->Log("保持连接成功.\r\n");
				obj->counter = 0;
				retry_count = 0;
			}

		}
		Sleep(1000);
		obj->counter++;
	}

	if (retry_count != 0)
	{
		obj->islogout = true;
		obj->Log("多次重连失败,请重新登入.");
		obj->islogout = true;
		::SendMessage(obj->GetSafeHwnd(), Message, NULL, Message_KeepActive_Log);
	}

	return 0;
}

void CEAuthenticatorDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	switch (nIDEvent)
	{
	case 0:
	{
		CTime tm = CTime::GetCurrentTime();
		time_hour = tm.GetHour();
		time_minute = tm.GetMinute();
		time_second = tm.GetSecond();
		break;
	}
	case 1:
	{
		InitNetworkAdapterInfo();

		CString netflow;

		netflow_in = m_pIfTable->table[m_connections[m_connection_selected].index].dwInOctets;
		netflow_out = m_pIfTable->table[m_connections[m_connection_selected].index].dwOutOctets;

		float speed_in = netflow_in - last_netflow_in;
		float speed_out = netflow_out - last_netflow_out;

		CString SizeLevel[4] = { "B","KB","MB","GB" };
		int sl_in = 0, sl_out = 0;
		for (; speed_in > 1024; sl_in++)
		{
			speed_in = speed_in / 1024;
		}
		for (; speed_out > 1024; sl_out++)
		{
			speed_out = speed_out / 1024;
		}

		netflow.Format("发送: %.2f%s/s 接收: %.2f%s/s", speed_out, SizeLevel[sl_out], speed_in, SizeLevel[sl_in]);
		SetDlgItemText(IDC_Netflow, netflow);

		last_netflow_in = netflow_in;
		last_netflow_out = netflow_out;

		break;
	}
	}

	CDialogEx::OnTimer(nIDEvent);
}

void CEAuthenticatorDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (islogout)
	{
		Shell_NotifyIcon(NIM_DELETE, &notifyicon);
		CDialogEx::OnOK();
	}
	else
	{
		ShowWindow(SW_HIDE);
		iswinshow = false;
		ShowBalloonTip("程序已最小化到托盘!", "", 3000, 1);
	}
}

void CEAuthenticatorDlg::OnCancel()
{
	return;
}

void CEAuthenticatorDlg::OnNotimenuLoginswitch()
{
	// TODO: 在此添加命令处理程序代码
	if (!is_threadrun)
	{
		OnBnClickedLoginswitch();
	}
	
}

void CEAuthenticatorDlg::OnNotimenuQuit()
{
	// TODO: 在此添加命令处理程序代码
	Shell_NotifyIcon(NIM_DELETE, &notifyicon);
	CDialogEx::OnOK();
}

void CEAuthenticatorDlg::OnNotimenuNasipstat()
{
	if (islogout)
	{
		NASIP nasipDlg;
		nasipDlg.DoModal();
		nasip = nasipDlg.nasip;
		notimenu.ModifyMenu(0, MF_BYPOSITION, ID_NotiMenu_NASIP, nasip);
	}
}
