// NASIP.cpp: 实现文件
//

#include "pch.h"
#include "EAuthenticator.h"
#include "NASIP.h"
#include "afxdialogex.h"


// NASIP 对话框

IMPLEMENT_DYNAMIC(NASIP, CDialogEx)

NASIP::NASIP(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SetNasip, pParent)
{

}

NASIP::~NASIP()
{
}

void NASIP::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(NASIP, CDialogEx)
	ON_BN_CLICKED(IDOK, &NASIP::OnBnClickedOk)
	ON_BN_CLICKED(IDC_Help, &NASIP::OnBnClickedHelp)
END_MESSAGE_MAP()


// NASIP 消息处理程序


void NASIP::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码

	GetDlgItemText(IDC_NASIP, nasip);
	CDialogEx::OnOK();
}


void NASIP::OnBnClickedHelp()
{
	// TODO: 在此添加控件通知处理程序代码
	MessageBox("1.断网时会自动打开一个网页，上面会有天翼校园的客户端下载。\r\n此时查看网址，\"&wlanacip=\"后面的IP地址即为nasip。\r\n\r\n"
		"2.右键你的天翼校园客户端，打开文件位置，找到Config文件夹，打开ConnectSetting.ini文件，wlanacip后面的IP地址即为nasip。"
		, "如何获取NASIP");
}
