
// tcp_on_udptoolsDlg.h: 头文件
//

#pragma once
#define   WM_SYSTEMTRAY WM_USER+5

// CtcponudptoolsDlg 对话框
class CtcponudptoolsDlg : public CDialogEx
{
// 构造
public:
	CtcponudptoolsDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TCP_ON_UDPTOOLS_DIALOG };
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
	afx_msg LRESULT OnSystemtray(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT Onclose(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	void SetOutPut1(wchar_t* output);
	void SetOutPut2(wchar_t* output);
	void InitRaw();
	void InitKcp();
	CEdit m_out1;
	CEdit m_out2;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CButton btnstart;
	CButton btnstop;
	NOTIFYICONDATA NotifyIcon;
	CStatic m_chonglian;
};
