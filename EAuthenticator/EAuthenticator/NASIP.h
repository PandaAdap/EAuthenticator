#pragma once


// NASIP 对话框

class NASIP : public CDialogEx
{
	DECLARE_DYNAMIC(NASIP)

public:
	NASIP(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~NASIP();

	CString nasip = "";

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SetNasip };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedHelp();
};
