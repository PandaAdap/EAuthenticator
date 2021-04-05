
// EAuthenticator.cpp: 定义应用程序的类行为。
//

#include "pch.h"
#include "framework.h"
#include "EAuthenticator.h"
#include "EAuthenticatorDlg.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CEAuthenticatorApp

BEGIN_MESSAGE_MAP(CEAuthenticatorApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CEAuthenticatorApp 构造

CEAuthenticatorApp::CEAuthenticatorApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的 CEAuthenticatorApp 对象

CEAuthenticatorApp theApp;


// CEAuthenticatorApp 初始化

BOOL CEAuthenticatorApp::InitInstance()
{
	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}
	
	CEAuthenticatorDlg dlg;
	m_pMainWnd = &dlg;

	CString cmdline = this->m_lpCmdLine, param_read;
	std::vector<CString> sub_param;

	for (int pos = 0; pos < cmdline.GetLength(); pos++)
	{
		param_read += cmdline.Mid(pos, 1);
		if (cmdline.Mid(pos, 1) == " " || pos == cmdline.GetLength() - 1)
		{
			sub_param.push_back(param_read);
			param_read = "";
		}
	}

	for (int param_count = 0; param_count < sub_param.size(); param_count++)
	{
		param_read = sub_param.at(param_count);
		if (param_read.Find("-username") > -1)
		{
			if (param_read.Find("=") > -1)
			{
				dlg.username = param_read.Mid(param_read.Find("=") + 1, param_read.GetLength());
			}
			else
			{
				param_count++;
				dlg.username = sub_param.at(param_count);
			}
		}
		else if (param_read.Find("-password") > -1)
		{
			if (param_read.Find("=") > -1)
			{
				dlg.password = param_read.Mid(param_read.Find("=") + 1, param_read.GetLength());
			}
			else
			{
				param_count++;
				dlg.password = sub_param.at(param_count);
			}
		}
		else if (param_read.Find("-nasip") > -1)
		{
			if (param_read.Find("=") > -1)
			{
				dlg.nasip = param_read.Mid(param_read.Find("=") + 1, param_read.GetLength());
			}
			else
			{
				param_count++;
				dlg.nasip = sub_param.at(param_count);
			}
		}
		else if (param_read.Find("-auto") > -1)
		{
			dlg.autologin = true;
		}
	}

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager *pShellManager = new CShellManager;

	// 激活“Windows Native”视觉管理器，以便在 MFC 控件中启用主题
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	SetRegistryKey(_T("EAuthenticator"));

	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}
	else if (nResponse == -1)
	{
		//TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
		//TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
	}

	// 删除上面创建的 shell 管理器。
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}

