
// tcp_on_udptoolsDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "tcp_on_udptools.h"
#include "tcp_on_udptoolsDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
int initstop = 0;
int pingliverun = 0;
int cltione = 0;
int chonglian = 0;
int qidong = 1;
char rawpath[0x1000] = { NULL };
char kcppath[0x1000] = { NULL };

HANDLE raw_hReadPipe = NULL;
HANDLE raw_hWritePipe = NULL;

HANDLE kcp_hReadPipe = NULL;
HANDLE kcp_hWritePipe = NULL;

HANDLE raw_Handle = NULL;
HANDLE kcp_Handle = NULL;

bool recv_raw = false;
bool recv_kcp = false;

CtcponudptoolsDlg* dlg = NULL;
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
bool rawok = 0;
DWORD Terminateprocess(const wchar_t* pProcess)
{
	HANDLE hSnapshot;
	DWORD hprocess = 0;
	PROCESSENTRY32 lppe;
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (hSnapshot == NULL)
		return false;
	lppe.dwSize = sizeof(lppe);
	if (!Process32First(hSnapshot, &lppe)) return false;
	do
	{
		if (memcmp(lppe.szExeFile, pProcess, wcslen(pProcess) * 2) == 0)
		{
			hprocess = lppe.th32ProcessID;
			HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, false, hprocess);
			if (handle)
			{
				TerminateProcess(handle, 0);
				WaitForSingleObject(handle, INFINITE);
				CloseHandle(handle);
			}

		}
	} while (Process32Next(hSnapshot, &lppe));
	if (!CloseHandle(hSnapshot))
		return hprocess;
	return hprocess;
}
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


// CtcponudptoolsDlg 对话框



CtcponudptoolsDlg::CtcponudptoolsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TCP_ON_UDPTOOLS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CtcponudptoolsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_out1);
	DDX_Control(pDX, IDC_EDIT2, m_out2);
	DDX_Control(pDX, IDC_BUTTON1, btnstart);
	DDX_Control(pDX, IDC_BUTTON2, btnstop);
	DDX_Control(pDX, IDC_STATIC1, m_chonglian);
}
#define IDM_EXIT 0x1003
BEGIN_MESSAGE_MAP(CtcponudptoolsDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCANCEL, &CtcponudptoolsDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON1, &CtcponudptoolsDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CtcponudptoolsDlg::OnBnClickedButton2)
	ON_MESSAGE(WM_SYSTEMTRAY, &CtcponudptoolsDlg::OnSystemtray)
	ON_COMMAND(IDM_EXIT, &OnBnClickedCancel)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()
void ReleasePipe()
{
	if (raw_hReadPipe)
	{
		CloseHandle(raw_hReadPipe);
		raw_hReadPipe = NULL;
	}
	if (raw_hWritePipe)
	{
		CloseHandle(raw_hWritePipe); 
		raw_hWritePipe = NULL;
	}
	if (kcp_hReadPipe)
	{
		CloseHandle(kcp_hReadPipe);
		kcp_hReadPipe = NULL;
	}
	if (kcp_hWritePipe)
	{
		CloseHandle(kcp_hWritePipe);
		kcp_hWritePipe = NULL;
	}
}
BOOL InitPipe()
{
	BOOL ret = FALSE;
	//ReleasePipe();
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	SECURITY_ATTRIBUTES sa1;
	sa1.nLength = sizeof(sa1);
	sa1.bInheritHandle = TRUE;
	sa1.lpSecurityDescriptor = NULL;
	ret = CreatePipe(&raw_hReadPipe, &raw_hWritePipe, &sa, 0);
	if (!ret)
		return ret;
	ret = CreatePipe(&kcp_hReadPipe, &kcp_hWritePipe, &sa1, 0);
	if (!ret)
		return ret;
	return ret;
}


std::wstring string2wstring(std::string str)
{
	std::wstring result;
	//获取缓冲区大小，并申请空间，缓冲区大小按字符计算      
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
	TCHAR* buffer = new TCHAR[len + 1];
	//多字节编码转换成宽字节编码      
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);
	buffer[len] = L'\0';
	//添加字符串结尾     
	//删除缓冲区并返回值      
	result.append(buffer);
	delete[] buffer;
	return result;
}

int start = 0;
int shakcount = 0;
void pinglive()
{

	if (raw_Handle)
	{
		dlg->m_out1.SetWindowTextW(L"");
		recv_raw = false;
		TerminateProcess(raw_Handle, 0);
		raw_Handle = NULL;
		Terminateprocess(L"udp2raw_mp_nolibnet.exe");
		start = 0;
		rawok = 0;
	}
	if (kcp_Handle)
	{
		dlg->m_out2.SetWindowTextW(L"");
		recv_kcp = false;
		TerminateProcess(kcp_Handle, 0);
		kcp_Handle = NULL;
		Terminateprocess(L"client_windows_amd64.exe");
	}
	dlg->btnstart.EnableWindow(FALSE);
	dlg->btnstop.EnableWindow(FALSE);
	initstop = 1;
	chonglian = 1;
	dlg->InitRaw();
}
int countchonglian = 0;
void recvrawpipe()
{
	DWORD dwRead = 0;
	DWORD dwAvail = 0;
	std::string out = "";
	char recvbuffer[0x1000];
	while (recv_raw)
	{
		Sleep(100);
		if (!PeekNamedPipe(raw_hReadPipe, NULL, NULL, &dwRead, &dwAvail, NULL) || dwAvail <= 0)
			continue;
		memset(recvbuffer, 0, 0x1000);
		if (ReadFile(raw_hReadPipe, recvbuffer, 0x1000, &dwRead, NULL))
		{
			if (dwRead == 0)
				break;
			out = recvbuffer;
			if (strstr(recvbuffer, "client_ready"))
			{
				rawok = 1;
				//MessageBoxA(0,"成功", "成功",0);
				if (!start)
				{
					start = 1;
					if (chonglian)
					{
						dlg->btnstart.EnableWindow(FALSE);
						chonglian = 0;
						dlg->InitKcp();
						dlg->btnstop.EnableWindow(TRUE);
					}
					else
					{
						dlg->btnstart.EnableWindow(TRUE);
						dlg->btnstop.EnableWindow(FALSE);
					}
					//dlg->InitKcp();
					
					
					//dlg->btnstart.EnableWindow(FALSE);
					//dlg->btnstop.EnableWindow(TRUE);
				}
				else
				{
					//dlg->btnstart.EnableWindow(TRUE);
					//dlg->btnstop.EnableWindow(FALSE);
				}

			}
			if (strstr(recvbuffer, "client_tcp_handshake_dummy"))
			{
				shakcount++;
			}
			if (qidong && shakcount > 3)
			{
				pinglive();
				shakcount = 0;
				countchonglian++;
				wchar_t tmp[30] = { NULL };
				_itow(countchonglian, tmp, 10);
				dlg->m_chonglian.SetWindowTextW(tmp);
				break;
			}
			dlg->SetOutPut1((wchar_t*)string2wstring(out).c_str());
		}
	}
}

void recvrawpipe2()
{
	DWORD dwRead = 0;
	DWORD dwAvail = 0;
	std::string out = "";
	char recvbuffer[0x1000] = { NULL };
	while (recv_kcp)
	{
		Sleep(100);
		if (!PeekNamedPipe(kcp_hReadPipe, NULL, NULL, &dwRead, &dwAvail, NULL) || dwAvail <= 0)
			continue;
		memset(recvbuffer, 0, 0x1000);
		if (ReadFile(kcp_hReadPipe, recvbuffer, 0x1000, &dwRead, NULL))
		{
			if (dwRead == 0)
				break;
			out = recvbuffer;
			dlg->SetOutPut2((wchar_t*)string2wstring(out).c_str());
		}
		memset(recvbuffer, 0, 0x1000);
	}
}


// CtcponudptoolsDlg 消息处理程序
void CtcponudptoolsDlg::InitRaw()
{

	if (!raw_Handle)
	{
		PROCESS_INFORMATION   pi = { 0 };
		STARTUPINFOA si = { 0 };
		si.cb = sizeof(STARTUPINFOA);
		//GetStartupInfoA(&si);
		SECURITY_ATTRIBUTES st = { 0 };
		st.nLength = sizeof(SECURITY_ATTRIBUTES);
		st.bInheritHandle = true;
		st.lpSecurityDescriptor = NULL;
		si.wShowWindow = SW_HIDE;
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		si.hStdOutput = raw_hWritePipe;
		si.hStdError = raw_hWritePipe;
		if (!CreateProcessA(NULL, rawpath, &st, &st, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi))
		{
			ReleasePipe();
		}
		raw_Handle = pi.hProcess;
		CloseHandle(pi.hThread);

		recv_raw = true;
		HANDLE handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)recvrawpipe, 0, 0, 0);
		CloseHandle(handle);
	}

}
void CtcponudptoolsDlg::InitKcp()
{

	if (!kcp_Handle)
	{
		PROCESS_INFORMATION   pi = { 0 };
		STARTUPINFOA si = { 0 };
		si.cb = sizeof(STARTUPINFOA);
		SECURITY_ATTRIBUTES st = { 0 };
		st.nLength = sizeof(SECURITY_ATTRIBUTES);
		st.bInheritHandle = true;
		st.lpSecurityDescriptor = NULL;
		//GetStartupInfoA(&si);
		si.wShowWindow = SW_HIDE;
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		si.hStdOutput = kcp_hWritePipe;
		si.hStdError = kcp_hWritePipe;
		if (!CreateProcessA(NULL, kcppath, &st, &st, TRUE, NULL, NULL, NULL, &si, &pi))
		{
			ReleasePipe();
		}
		kcp_Handle = pi.hProcess;
		CloseHandle(pi.hThread);
		recv_kcp = true;
		HANDLE handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)recvrawpipe2, 0, 0, 0);
		CloseHandle(handle);
		if (!pingliverun)
		{
			//HANDLE handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)pinglive, 0, 0, 0);
			//CloseHandle(handle);
		}
	}

}

BOOL CtcponudptoolsDlg::OnInitDialog()
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


	NotifyIcon.cbSize = sizeof(NOTIFYICONDATA);
	//NotifyIcon.hIcon=AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	NotifyIcon.hIcon = m_hIcon;  //上面那句也可以
	NotifyIcon.hWnd = m_hWnd;
	lstrcpy(NotifyIcon.szTip, _T("udp转发"));
	NotifyIcon.uCallbackMessage = WM_SYSTEMTRAY;
	NotifyIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	Shell_NotifyIcon(NIM_ADD, &NotifyIcon);   //添加系统托盘



	// TODO: 在此添加额外的初始化代码
	dlg = this;
	m_out1.SetLimitText(UINT_MAX);
	m_out2.SetLimitText(UINT_MAX);

	FILE* fp = fopen("seting1", "rt");
	if (fp == NULL)
	{
		MessageBoxA(0, "找不到配置文件1", 0, 0);
		return TRUE;
	}
	fgets(rawpath, 0x1000, fp);
	fclose(fp);

	fp = fopen("seting2", "rt");
	if (fp == NULL)
	{
		MessageBoxA(0, "找不到配置文件1", 0, 0);
		return TRUE;
	}
	fgets(kcppath, 0x1000, fp);
	fclose(fp);
	dlg->btnstart.EnableWindow(FALSE);
	dlg->btnstop.EnableWindow(FALSE);
	if (!InitPipe())
	{
		MessageBoxA(0, "管道创建失败", 0, 0);
		return TRUE;
	}
	Terminateprocess(L"udp2raw_mp_nolibnet.exe");
	Terminateprocess(L"client_windows_amd64.exe");
	chonglian = 1;
	InitRaw();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}
LRESULT CtcponudptoolsDlg::Onclose(WPARAM wParam, LPARAM lParam)
{
	OnBnClickedCancel();
	return TRUE;
}
bool wnshow = 1;

LRESULT CtcponudptoolsDlg::OnSystemtray(WPARAM wParam, LPARAM lParam)
{
	switch (lParam) 
	{
	case WM_RBUTTONUP://右键起来时弹出快捷菜单         
	{
		LPPOINT lpoint = new tagPOINT;
		::GetCursorPos(lpoint);//得到鼠标位置            
		CMenu menu;
		menu.CreatePopupMenu();//声明一个弹出式菜单            
							   //增加菜单项“关闭”，点击则发送消息WM_DESTROY给主窗口（已             //隐藏），将程序结束。            	    
		menu.AppendMenu(MF_STRING, IDM_EXIT, L"退出");//自己定义的命令，需要手动添加命令响应函数        
												  //确定弹出式菜单的位置          
		menu.TrackPopupMenu(TPM_LEFTALIGN, lpoint->x, lpoint->y, this);
		//资源回收           
		HMENU hmenu = menu.Detach();
		menu.DestroyMenu();
		delete lpoint;
	}
	break;
	case WM_LBUTTONUP://左键起来时打开界面     
	{
		if (wnshow)
		{
			ShowWindow(SW_HIDE);
			wnshow = 0;
		}
		else
		{
			ShowWindow(SW_SHOW);
			wnshow = 1;
		}
		//显示主窗口		  
		//DeleteTray();//删除托盘     
	}
	break;
	case  WM_LBUTTONDBLCLK://左键双击，添加相应的功能		
		break;
	case IDM_EXIT:
		OnBnClickedCancel();
		break;
	}
	return 0;

}
void CtcponudptoolsDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CtcponudptoolsDlg::OnPaint()
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
HCURSOR CtcponudptoolsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CtcponudptoolsDlg::OnBnClickedCancel()
{
	recv_raw = false;
	if (raw_Handle)
	{
		TerminateProcess(raw_Handle, 0);
		raw_Handle = NULL;
		Terminateprocess(L"udp2raw_mp_nolibnet.exe");
		recv_raw = false;
	}
	if (kcp_Handle)
	{
		TerminateProcess(kcp_Handle, 0);
		kcp_Handle = NULL;
		Terminateprocess(L"client_windows_amd64.exe");
		recv_kcp = false;
	}

	ReleasePipe();
	CDialogEx::OnCancel();
}
void CtcponudptoolsDlg::SetOutPut1(wchar_t* output)
{
	CString clog;
	m_out1.GetWindowTextW(clog);
	clog += output;
	m_out1.SetWindowTextW(clog);
	int lines = m_out1.GetLineCount();
	//m_out1.SetSel(counts, counts);
	m_out1.LineScroll(lines, 0);
}
void CtcponudptoolsDlg::SetOutPut2(wchar_t* output)
{
	CString clog = L"\r\n";
	m_out2.GetWindowTextW(clog);
	clog = clog + L"\r\n" + output;
	m_out2.SetWindowTextW(clog);
	int lines = m_out2.GetLineCount();
	//m_out1.SetSel(counts, counts);
	m_out2.LineScroll(lines, 0);
}
//启动

void CtcponudptoolsDlg::OnBnClickedButton1()
{

	if (initstop)
	{
		if (!rawok)
		{
			InitRaw();
		}
		else
		{
			InitKcp();
		}

	}
	else
	{
		InitKcp();
	}
	dlg->btnstart.EnableWindow(FALSE);
	dlg->btnstop.EnableWindow(TRUE);
	qidong = 1;
}

//停止
void CtcponudptoolsDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	//m_out1.SetWindowTextW(L"");
	m_out2.SetWindowTextW(L"");
	if (!rawok && raw_Handle)
	{
		recv_raw = false;
		TerminateProcess(raw_Handle, 0);
		raw_Handle = NULL;
		Terminateprocess(L"udp2raw_mp_nolibnet.exe");
		start = 0;
	}
	if (kcp_Handle)
	{
		recv_kcp = false;
		TerminateProcess(kcp_Handle, 0);
		kcp_Handle = NULL;
		Terminateprocess(L"client_windows_amd64.exe");
	}
	//ReleasePipe();
	//if (kcp_hReadPipe)
	//{
	//	CloseHandle(kcp_hReadPipe);
	//	kcp_hReadPipe = NULL;
	//}
	//if (kcp_hWritePipe)
	//{
	//	CloseHandle(kcp_hWritePipe);
	//	kcp_hWritePipe = NULL;
	//}
	dlg->btnstart.EnableWindow(TRUE);
	dlg->btnstop.EnableWindow(FALSE);
	initstop = 1;
	qidong = 0;


}


HBRUSH CtcponudptoolsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{

	if (pWnd->GetDlgCtrlID() == IDC_EDIT1 || pWnd->GetDlgCtrlID() == IDC_EDIT2)//改变编辑框
	{
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(255, 255, 255));
		pDC->SetBkColor(RGB(0, 0, 0));
		HBRUSH b = CreateSolidBrush(RGB(0, 0, 0));
		return b;
	}
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	return hbr;
}
