
// EAuthenticatorDlg.h: 头文件
//

#pragma once

#include "EAuth.h"
#include "NASIP.h"

#include <string>
#include <vector>

// CEAuthenticatorDlg 对话框
class CEAuthenticatorDlg : public CDialogEx
{
// 构造
public:
	CEAuthenticatorDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EAUTHENTICATOR_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:

	afx_msg void OnBnClickedLoginswitch();
	afx_msg LRESULT OnTray(WPARAM wParam, LPARAM lParam);
	BOOL ShowBalloonTip(LPCTSTR szMsg, LPCTSTR szTitle, UINT uTimeout, DWORD dwInfoFlags);
	afx_msg LRESULT OnUserMsg(WPARAM wParam, LPARAM lParam);

	NOTIFYICONDATA notifyicon;
	CMenu notimenu;

	EAuth eauth;

	CryptoFile easyencrypt;

	CString username = "";
	CString password = "";
	CString nasip = "";

	bool autologin = false;
	//bool server = false;
	//CString server_ip, server_port;

	bool is_threadrun = false;
	bool islogout = true;
	bool iswinshow = true;

	int counter = 0;
	int time_hour = 0, time_minute = 0, time_second = 0;

	struct NetWorkConection
	{
		int index;				//网络索引
		std::string description;//网络描述
		unsigned int in_bytes;	//初始时已接收字节数
		unsigned int out_bytes;	//初始时已发送字节数
		NetWorkConection(int idx, std::string desc, unsigned int in_bytes, unsigned out_bytes)
			: index{ idx }, description{ desc }, in_bytes{ in_bytes }, out_bytes{ out_bytes }		//构造函数
		{}
	};

	int m_connection_selected = 0;
	std::string m_connection_name;
	std::vector<NetWorkConection> m_connections;
	MIB_IFTABLE* m_pIfTable;
	DWORD m_dwSize{};

	int netflow_in = 0, netflow_out = 0, last_netflow_in = 0, last_netflow_out = 0;

	void InitNetworkAdapterInfo();
	virtual void OnOK();
	void Log(CString str);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnNotimenuLoginswitch();
	afx_msg void OnNotimenuQuit();
	afx_msg void OnNotimenuNasipstat();
};
