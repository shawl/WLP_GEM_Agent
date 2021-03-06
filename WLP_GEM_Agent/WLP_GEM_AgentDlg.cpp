﻿
// WLP_GEM_AgentDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "WLP_GEM_Agent.h"
#include "WLP_GEM_AgentDlg.h"
#include "afxdialogex.h"

#include "ClientSocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CWLP_GEM_AgentDlg 대화 상자
CWLP_GEM_AgentDlg::CWLP_GEM_AgentDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWLP_GEM_AgentDlg::IDD, pParent)
	, m_strCEIDFilePath(L"")
	, m_strSVIDFilePath(L"")
	, m_strALIDFilePath(L"")
	, m_strIniFilePath(L"")
	, m_strStartPortID(L"")
	, m_strHCS_PP_SELECT(L"")
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
	m_bEqConnect = FALSE;

	m_bOnlineRemote = FALSE;

	m_nHCS_PP_Status = 0;

}

void CWLP_GEM_AgentDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EZGEMCTRL1, m_GEM);
	DDX_Control(pDX, IDC_LIST_TCP, m_ctlListTCP);
	DDX_Control(pDX, IDC_LIST_GEM, m_ctlListGEM);
}

BEGIN_MESSAGE_MAP(CWLP_GEM_AgentDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BT_GEM_START, &CWLP_GEM_AgentDlg::OnBnClickedBtGEMStart)
	ON_BN_CLICKED(IDC_BT_GEM_STOP, &CWLP_GEM_AgentDlg::OnBnClickedBtGEMStop)
	ON_BN_CLICKED(IDC_BT_SVR_STOP, &CWLP_GEM_AgentDlg::OnBnClickedBtSvrStop)
	ON_BN_CLICKED(IDC_BT_SVR_START, &CWLP_GEM_AgentDlg::OnBnClickedBtSvrStart)
	ON_BN_CLICKED(IDC_BT_RECIPE, &CWLP_GEM_AgentDlg::OnBnClickedBtRecipe)
	ON_WM_TIMER()
END_MESSAGE_MAP()

// CWLP_GEM_AgentDlg 메시지 처리기

BOOL CWLP_GEM_AgentDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	//중복실행 방지
	HANDLE hEvent;
    hEvent = CreateEvent(NULL, FALSE, TRUE, AfxGetAppName());
    if ( GetLastError() == ERROR_ALREADY_EXISTS)
    {
        PostQuitMessage(WM_QUIT);
    }
	//

	SetDlgItemText(IDC_EDIT_SVR_PORT,L"10101");

	ShowWindow(SW_SHOWMINIMIZED);
	SvrStart();
	
	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CWLP_GEM_AgentDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CWLP_GEM_AgentDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CWLP_GEM_AgentDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CWLP_GEM_AgentDlg::MBCS2Unicode(LPCSTR lpData,LPWSTR ReturnData)
{
	memset(ReturnData,0,(strlen(lpData)/2)+5);
	MultiByteToWideChar(949,0,lpData,-1,ReturnData,strlen(lpData));
	return;
}

void CWLP_GEM_AgentDlg::Unicode2MBCS(LPWSTR lpData,LPSTR lpRtd)
{
	memset(lpRtd,0x00,wcslen(lpData)*2+1);
	WideCharToMultiByte(949,0,lpData,-1,lpRtd,wcslen(lpData)*2,NULL,NULL);
	return;
}

BEGIN_EVENTSINK_MAP(CWLP_GEM_AgentDlg, CDialogEx)
	ON_EVENT(CWLP_GEM_AgentDlg, IDC_EZGEMCTRL1, 1, CWLP_GEM_AgentDlg::OnConnectedEzgemctrl1, VTS_NONE)
	ON_EVENT(CWLP_GEM_AgentDlg, IDC_EZGEMCTRL1, 2, CWLP_GEM_AgentDlg::OnDisconnectedEzgemctrl1, VTS_NONE)
	ON_EVENT(CWLP_GEM_AgentDlg, IDC_EZGEMCTRL1, 24, CWLP_GEM_AgentDlg::OnSecsMsgInEzgemctrl1, VTS_I2 VTS_I2 VTS_I2 VTS_I4)
	ON_EVENT(CWLP_GEM_AgentDlg, IDC_EZGEMCTRL1, 25, CWLP_GEM_AgentDlg::OnSecsMsgOutEzgemctrl1, VTS_I2 VTS_I2 VTS_I2 VTS_I4)
	ON_EVENT(CWLP_GEM_AgentDlg, IDC_EZGEMCTRL1, 7, CWLP_GEM_AgentDlg::OnRemoteCommandEzgemctrl1, VTS_I4 VTS_BSTR VTS_I2)
	ON_EVENT(CWLP_GEM_AgentDlg, IDC_EZGEMCTRL1, 15, CWLP_GEM_AgentDlg::OnMsgRequestedEzgemctrl1, VTS_I4)
	ON_EVENT(CWLP_GEM_AgentDlg, IDC_EZGEMCTRL1, 13, CWLP_GEM_AgentDlg::OnTerminalMessageSingleEzgemctrl1, VTS_I4 VTS_BSTR VTS_I2)
	ON_EVENT(CWLP_GEM_AgentDlg, IDC_EZGEMCTRL1, 3, CWLP_GEM_AgentDlg::OnDateTimeSetRequestEzgemctrl1, VTS_I4 VTS_BSTR)
	ON_EVENT(CWLP_GEM_AgentDlg, IDC_EZGEMCTRL1, 5, CWLP_GEM_AgentDlg::OnOnlineRequestEzgemctrl1, VTS_I4)
	ON_EVENT(CWLP_GEM_AgentDlg, IDC_EZGEMCTRL1, 4, CWLP_GEM_AgentDlg::OnEstablishCommRequestEzgemctrl1, VTS_I4)
	ON_EVENT(CWLP_GEM_AgentDlg, IDC_EZGEMCTRL1, 17, CWLP_GEM_AgentDlg::OnOnlineRemoteEzgemctrl1, VTS_NONE)
	ON_EVENT(CWLP_GEM_AgentDlg, IDC_EZGEMCTRL1, 6, CWLP_GEM_AgentDlg::OnOfflineRequestEzgemctrl1, VTS_I4)
	ON_EVENT(CWLP_GEM_AgentDlg, IDC_EZGEMCTRL1, 14, CWLP_GEM_AgentDlg::OnTerminalMessageMultiEzgemctrl1, VTS_I4 VTS_I2 VTS_I2)
END_EVENTSINK_MAP()

void CWLP_GEM_AgentDlg::OnBnClickedBtGEMStart()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GEMStart();
}

BOOL CWLP_GEM_AgentDlg::GEMStart()
{
	if(!LoadGemConfig())
	{
		AddLogGEM(L"LoadGemConfig fail!");
		return FALSE; //설정파일(GEM.INI) 자체가 없으면 driver 구동하지 않고 return.  SVID,CEID는 설정파일 없음 경고메시지만...구동은 시킴.
	}

//Log 파일 설정
	_wmkdir(L"GEM\\LOG\\"); // log directory
	
	TCHAR szCurrentPath[1024];
	GetCurrentDirectory(1024, szCurrentPath);
	CString strLogFile = szCurrentPath;
	strLogFile.Format(L"%s\\GEM\\LOG\\GEM.LOG", szCurrentPath);

//Format 체크 설정
	/*	m_GEM.SetFormatFile(L"FORMAT.SML"));
	m_GEM.SetFormatCheck(TRUE);*/

	m_GEM.SetLogFile(strLogFile, 1);

//각 FORMAT CODE 재정의
 // I2 = 32, U4 = 54, I4 = 34, U1 = 51, U2 = 52, I1 = 31
	m_GEM.SetFormatCodeSVID(54);
	m_GEM.SetFormatCodeCEID(54);
	m_GEM.SetFormatCodeALID(54);
	m_GEM.SetFormatCodeECID(54);

	m_GEM.DisableAutoReply(7, 3);   //S7,F3 Process Program Send (PPS) - Unformatted
	m_GEM.DisableAutoReply(7, 5);   //S7,F5 Process Program Request (PPR)
	m_GEM.DisableAutoReply(7, 19);	///S7, F19 Current EPPD Request
	
	//2016-02-11
	m_GEM.DisableAutoReply(1,15); //?? 응답이 올라옴. S1F16
	m_GEM.DisableAutoReply(1,17);
	//TO-Check!
	/*m_GEM.DisableAutoReply(7, 17);	
	m_GEM.DisableAutoReply(7, 23);
	m_GEM.DisableAutoReply(7, 25);*/

	///////////////////////////////////////////////////////////////////////
	
	if( m_GEM.Start() >= 0 )
	{
		m_GEM.EnableCommunication();
		m_GEM.SetHostMode(FALSE);

		AddLogGEM(L"Started");
		GetDlgItem(IDC_BT_GEM_START)->EnableWindow(FALSE);	
	}

	return TRUE;
//2016-01-08 박성우과장 문의. 
	//m_GEM.GoOffline();  //offline 상태 event는 먼저 보내주어야 한다. - 이후 ezGEM driver 모든 처리 없으므로 S1,F15 
	//S1, F17 받았을때 이전상태로 - Local 또는 Remote
	//m_GEM.GoOnlineLocal(); //
	//m_GEM.GoOnlineRemote(); // OnlineRemote, OnlineLocal 동일. Remote Command 처리 안하려면 ezGEM driver는 같이 태우므로, 따로 처리해야함(flag등)

//2016-02-04 드림시스 도상호 대리 추가 개발 요구사항
	//Mode가 Online Local일 때 Event를 EAP로 동일하게 올려 주는 부분 개발 필요(EAP에서 명령 내리지 않고 동작하고, Event만 올리는 부분)

}


void CWLP_GEM_AgentDlg::OnBnClickedBtGEMStop()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GEMStop();
}

BOOL CWLP_GEM_AgentDlg::GEMStop()
{
	if(m_GEM.Stop() >= 0)
	{
		AddLogGEM(L"Stopped");
		GetDlgItem(IDC_BT_GEM_START)->EnableWindow(TRUE);
		return TRUE;
	}
	else
	{
		AddLogGEM(L"GEM stop fail");
		return FALSE;
	}
}

void CWLP_GEM_AgentDlg::OnBnClickedBtSvrStart()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	SvrStart();
}

BOOL CWLP_GEM_AgentDlg::SvrStart()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

	CString strMsg;
	if(m_ListenSocket.Create(GetDlgItemInt(IDC_EDIT_SVR_PORT), SOCK_STREAM))
	{
		DWORD optval = true;
		int temp = sizeof(DWORD);
		optval = true;
		temp = sizeof(DWORD);
		
		BOOL bRet = m_ListenSocket.SetSockOpt(SO_KEEPALIVE, (void*)&optval, temp);

		if (bRet == TRUE) {
			strMsg = L"SetSockOpt SO_KEEPALIVE";
			GetLog()->Debug(strMsg.GetBuffer());
		}
		else
		{
			strMsg = L"Fail - SetSockOpt SO_KEEPALIVE";
			GetLog()->Debug(strMsg.GetBuffer());
		}

		if(!m_ListenSocket.Listen())
		{
			strMsg = L"ERROR: Listen() return FALSE";
			AddLogTCP(strMsg);
			GetLog()->Debug(strMsg.GetBuffer());
			return FALSE;
		}
		else
		{
			strMsg = L"Socket Server Started.";
			AddLogTCP(strMsg);
			GetLog()->Debug(strMsg.GetBuffer());
			GetDlgItem(IDC_BT_SVR_START)->EnableWindow(FALSE);
			return TRUE;
		}
	}
	else
	{
		DWORD dErr = GetLastError();
		strMsg.Format(L"ERROR: Failed to create server socket - %d",dErr);
		AddLogTCP(strMsg);
		GetLog()->Debug(strMsg.GetBuffer());
		return FALSE;
	}
}

void CWLP_GEM_AgentDlg::OnBnClickedBtSvrStop()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	SvrStop();
}

BOOL CWLP_GEM_AgentDlg::SvrStop()
{
	POSITION pos;
	pos = m_ListenSocket.m_ptrClientSocketList.GetHeadPosition();
	CClientSocket* pClient = NULL;

	while(pos != NULL)
	{
		pClient = (CClientSocket*)
			m_ListenSocket.m_ptrClientSocketList.GetNext(pos);
		if(pClient != NULL)
		{
			pClient->ShutDown(2);//receives = 0 • sends = 1  • both = 2 
			pClient->Close();

			delete pClient;
		}
	}

	m_ListenSocket.ShutDown(2); //receives = 0 • sends = 1  • both = 2 

	m_ListenSocket.Close();

	CString strMsg = L"Socket Server Stopped.";
	AddLogTCP(strMsg);
	GetLog()->Debug(strMsg.GetBuffer());

	GetDlgItem(IDC_BT_SVR_START)->EnableWindow(TRUE);

	return TRUE;
}

void CWLP_GEM_AgentDlg::AddLogTCP(CString str)
{
	m_ctlListTCP.AddString(str);

	int nCnt = m_ctlListTCP.GetCount();

	while(nCnt > 2000)
	{  
		m_ctlListTCP.DeleteString(0);
		break;
	}	  

	m_ctlListTCP.SetCurSel(nCnt - 1);
}

void CWLP_GEM_AgentDlg::AddLogGEM(CString str)
{
	m_ctlListGEM.AddString(str);

	int nCnt = m_ctlListGEM.GetCount();

	while(nCnt > 2000)
	{  
		m_ctlListGEM.DeleteString(0);
		break;
	}	  

	m_ctlListGEM.SetCurSel(nCnt - 1);
}

/*DATA FORMAT “A”,”B”,”BOOL”,”I1”,”I2”,”I4”,”I8”,”U1”,”U2”,”U4”,”U8”,”F4”,”F8”*/
void CWLP_GEM_AgentDlg::AddSVID()
{
	char *ptr, buff[0xFF], *nextptr, *currentptr;
	long SVID = 0;
	CString strSVID = L"";
	CString strDataType = L"";
	FILE *fp;
	//bool b_testSVID;

	fopen_s(&fp, (CStringA)m_strSVIDFilePath,"r+t"); // !주의	//"GEM\\SVID.TXT"
	if (fp == NULL)
	{
		AfxMessageBox(L"GEM/SVID.TXT 파일을 찾을수 없습니다.");
		return;
	}

	while ((ptr = fgets(buff, 0xFF, fp)) != NULL)
	{
		SVID = 0;
		//currentptr = strtok_s(ptr, "\r\n", &nextptr);
		//if (*currentptr == '#' || currentptr == NULL) continue;
		currentptr = strtok_s(ptr, "\t", &nextptr);	
		if (currentptr)
		{
			SVID = atol(currentptr);
		}
		while (currentptr)
		{
			currentptr = strtok_s(NULL, "\t", &nextptr);
			if (currentptr)
			{
				strSVID= currentptr;

				currentptr = strtok_s(NULL, "\t", &nextptr);
				if (currentptr)	// ALTX
				{
					//m_gem.AddSVID(ALID, ptr, strALCD);
					strDataType = currentptr;
					strDataType.Replace(L"\n", L"");

					m_GEM.AddSVID(SVID, strSVID, strDataType, L"");
				}
			}
		}
	}

	fclose(fp);
}

void CWLP_GEM_AgentDlg::AddCEID()
{
	char *ptr, buff[0xFF], *nextptr, *currentptr;
	long CEID = 0;
	CString strCEID=L"";

	// 파일의 위치는 받아와서 진행이 가능하도록 수정.
	FILE *fp;
	fopen_s(&fp,(CStringA)m_strCEIDFilePath,"r+t"); // !주의 "GEM\\CEID.TXT"
	if (fp == NULL)
	{
		AfxMessageBox(L"GEM/CEID.TXT 파일을 찾을수 없습니다.");
		return;
	}

	while ((ptr = fgets(buff, 0xFF, fp)) != NULL)
	{
		//ptr = strtok_s(ptr, "\r\n", &nextptr);
		//if (*ptr == '#' || ptr == NULL) continue;
		currentptr = strtok_s(ptr, "\t", &nextptr);	
		if (currentptr) CEID = atol(currentptr);
		while (currentptr)
		{
			currentptr = strtok_s(NULL, "\t", &nextptr);
			if (currentptr)	// CEID
			{
				strCEID= currentptr;
				m_GEM.AddCEID(CEID,	strCEID, L"");
			}
		}
	}
	fclose(fp);
}

//ECID는 직접 입력 - NVIAsoft 외주 코드 - GEM_HITS_LSPLC_20131024_FINAL 참조
void CWLP_GEM_AgentDlg::AddECID() //Equipment Constant ID
{
	
	CString strTemp;
	
	m_GEM.AddECID(ECID_PORT,			_T("PortNumber"), _T(""), _T("U2"));
	m_GEM.SetEquipConstRange(ECID_PORT, _T("2000"), _T("65535"));
	strTemp.Format(_T("%ld"), m_nPort);
	m_GEM.SetEquipConstValue(ECID_PORT, strTemp); //(ECID)3001
	
	m_GEM.AddECID(ECID_DEVICEID,		_T("DeviceID"), _T(""), _T("U2"));
	m_GEM.SetEquipConstRange(ECID_DEVICEID, _T("0"), _T("127"));
	strTemp.Format(_T("%d"), m_nDeviceID);
	m_GEM.SetEquipConstValue(ECID_DEVICEID, strTemp);//(ECID)3002
	
	m_GEM.AddECID(ECID_T3,				_T("T3 Timeout"), _T("second"), _T("U2"));
	m_GEM.SetEquipConstRange(ECID_T3, _T("1"), _T("255"));
	strTemp.Format(_T("%d"), m_nT3);
	m_GEM.SetEquipConstValue(ECID_T3, strTemp);//(ECID)3003
	
	m_GEM.AddECID(ECID_T5,				_T("T5 Timeout"), _T("second"), _T("U2"));
	m_GEM.SetEquipConstRange(ECID_T5, _T("1"), _T("255"));
	strTemp.Format(_T("%d"), m_nT5);
	m_GEM.SetEquipConstValue(ECID_T5, strTemp);	//(ECID)3005
	
	m_GEM.AddECID(ECID_T6,				_T("T6 Timeout"), _T("second"), _T("U2"));
	m_GEM.SetEquipConstRange(ECID_T6, _T("1"), _T("255"));
	strTemp.Format(_T("%d"), m_nT6);
	m_GEM.SetEquipConstValue(ECID_T6, strTemp);	//(ECID)3006
	
	m_GEM.AddECID(ECID_T7,				_T("T7 Timeout"), _T("second"), _T("U2"));
	m_GEM.SetEquipConstRange(ECID_T7, _T("1"), _T("255"));
	strTemp.Format(_T("%d"), m_nT7);
	m_GEM.SetEquipConstValue(ECID_T7, strTemp);	//(ECID)3007
	
	m_GEM.AddECID(ECID_T8,				_T("T8 Timeout"), _T("second"), _T("U2"));
	m_GEM.SetEquipConstRange(ECID_T8, _T("1"), _T("255"));
	strTemp.Format(_T("%d"), m_nT8);
	m_GEM.SetEquipConstValue(ECID_T8, strTemp);	//(ECID)3008
	
	m_GEM.AddECID(ECID_LINKTEST,		_T("LinkTestInterVal"), _T("second"), _T("U2"));
	m_GEM.SetEquipConstRange(ECID_LINKTEST, _T("0"), _T("255"));
	strTemp.Format(_T("%d"), m_nLinkInterval);
	m_GEM.SetEquipConstValue(ECID_LINKTEST, strTemp);	//(ECID)3009
	
	m_GEM.AddECID(ECID_RETRY,			_T("RetryLimit"), _T(""), _T("U2"));
	m_GEM.SetEquipConstRange(ECID_RETRY, _T("0"), _T("255"));
	strTemp.Format(_T("%d"), m_nRetry);
	m_GEM.SetEquipConstValue(ECID_RETRY, strTemp);	//(ECID)3010
}

void CWLP_GEM_AgentDlg::AddALID() //Alarm ID
{
//ALID.txt
//1	Alarm1	1
//2	Alarm2	2

	char *ptr, buff[0xFF], *nextptr, *currentptr;
	long ALID = 0;
	CString strDesc = L"";
	CString strCode = L"";
	FILE *fp;
	//bool b_testSVID;

	fopen_s(&fp, (CStringA)m_strALIDFilePath,"r+t"); // !주의	//"GEM\\SVID.TXT"
	if (fp == NULL)
	{
		AfxMessageBox(L"GEM/ALID.TXT 파일을 찾을수 없습니다.");
		return;
	}

	while ((ptr = fgets(buff, 0xFF, fp)) != NULL)
	{
		ALID = 0;
		//currentptr = strtok_s(ptr, "\r\n", &nextptr);
		//if (*currentptr == '#' || currentptr == NULL) continue;
		currentptr = strtok_s(ptr, "\t", &nextptr);	
		if (currentptr)
		{
			ALID = atol(currentptr);
		}
		while (currentptr)
		{
			currentptr = strtok_s(NULL, "\t", &nextptr);
			if (currentptr)
			{
				strDesc= currentptr;

				currentptr = strtok_s(NULL, "\t", &nextptr);
				if (currentptr)	// ALTX
				{
					strCode = currentptr;
					strCode.Replace(L"\n", L"");
//
					m_GEM.AddAlarmID(ALID, strDesc, strCode); //AddAlarmID(ALID, ALTX, ALCD); - 2016-03-10 NVIAsoft 김민철과장 확인. ezGEM 매뉴얼 오류
//ALCD	Alarm code byte	
//bit 8 = 1 means alarm set	
//bit 8 = 0 means alarm cleared	
//bit 7-1 is alarm category (not used in GEM)
				}
			}
		}
	}
	fclose(fp);
}
/*
S5, F1  Alarm Report Send (ARS)						     S , H  E, reply

Description	: Equipment uses this message to report a change in or presence of an alarm condition. This message will be sent when the alarm is set, and again when the alarm is cleared.

Structure	: L, 3
		1. <B       ALCD>
		2. <U2     ALID>
		3. <A[40] ALTX>

Note	:
ALCD	Alarm code byte	
bit 8 = 1 means alarm set	
bit 8 = 0 means alarm cleared	
bit 7-1 is alarm category (not used in GEM)
ALID	Alarm Identification Code
ALTX	Alarm text description
*/
BOOL CWLP_GEM_AgentDlg::ReadConfigFile() // \\GEM\\GEM.INI, \\GEM\SVID.txt, \\GEM\CEID.txt, \\GEM\ALID.txt 
{
	TCHAR szCurrentPath[1024], szValue[0xFF];
	GetCurrentDirectory(1024, szCurrentPath);
	CString strPath = szCurrentPath;
	CString strExePath = GetExecuteDirectory();
	strPath.Format(L"%s\\GEM\\GEM.ini", strExePath);
	
	CString strSVIDDefault;
	CString strCEIDDefault;
	CString strALIDDefault;

	strSVIDDefault.Format(L"%s\\GEM\\SVID.txt", strExePath);
	strCEIDDefault.Format(L"%s\\GEM\\CEID.txt", strExePath);
	strALIDDefault.Format(L"%s\\GEM\\ALID.txt", strExePath);

	if(!PathFileExists(strPath)) 
	{
		MessageBox(L"GEM.INI 파일이 존재하지 않습니다.", L"Error", MB_ICONERROR);
		return FALSE;
	}
	else if(!PathFileExists(strSVIDDefault))
	{
		MessageBox(L"SVID.txt 파일이 존재하지 않습니다.", L"Error", MB_ICONERROR);
		return FALSE;
	}
	else if(!PathFileExists(strCEIDDefault))
	{
		MessageBox(L"CEID.txt 파일이 존재하지 않습니다.", L"Error", MB_ICONERROR);
		return FALSE;
	}
	else if(!PathFileExists(strALIDDefault))
	{
		MessageBox(L"ALID.txt 파일이 존재하지 않습니다.", L"Error", MB_ICONERROR);
		return FALSE;
	}

	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(HSMS, HSMS_PORT,L"", szValue, sizeof(szValue), strPath))
		m_nPort = _ttol(szValue);

	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(HSMS, HSMS_DEVICEID, L"", szValue, sizeof(szValue), strPath))
		m_nDeviceID = _ttoi(szValue);

	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(HSMS, HSMS_T3, L"", szValue, sizeof(szValue), strPath))
		m_nT3 = _ttoi(szValue);

	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(HSMS, HSMS_T5, L"", szValue, sizeof(szValue), strPath))
		m_nT5 = _ttoi(szValue);

	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(HSMS, HSMS_T6, L"", szValue, sizeof(szValue), strPath))
		m_nT6 = _ttoi(szValue);

	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(HSMS, HSMS_T7, L"", szValue, sizeof(szValue), strPath))
		m_nT7 = _ttoi(szValue);

	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(HSMS, HSMS_T8, L"", szValue, sizeof(szValue), strPath))
		m_nT8 = _ttoi(szValue);

	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(HSMS, HSMS_LINKTEST, L"", szValue, sizeof(szValue), strPath))
		m_nLinkInterval = _ttoi(szValue);

	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(HSMS, HSMS_RETRY, L"", szValue, sizeof(szValue), strPath))
		m_nRetry = _ttoi(szValue);

	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(HSMS, L"IP", L"", szValue, sizeof(szValue), strPath))
		m_strIP = szValue;

	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(HSMS, L"MODE" , L"", szValue, sizeof(szValue), strPath))
		m_strMODE = szValue;	

	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(HSMS, L"CTTIME" , L"", szValue, sizeof(szValue), strPath))
		m_nCTTime = _ttoi(szValue);	
// CEID,SVID,ECID,ALID
	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(HSMS, L"CEIDFILEPATH" ,strCEIDDefault, szValue, sizeof(szValue), strPath))
	{	
		m_strCEIDFilePath = strCEIDDefault;
	}

	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(HSMS, L"SVIDFILEPATH" , strSVIDDefault, szValue, sizeof(szValue), strPath))
	{	
		m_strSVIDFilePath = strSVIDDefault;	
	}
	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(HSMS, L"ALIDFILEPATH" , strSVIDDefault, szValue, sizeof(szValue), strPath))
	{	
		m_strALIDFilePath = strALIDDefault;	
	}
//	
	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(L"GEM", L"DefaultCommState", L"", szValue, sizeof(szValue), strPath))
		m_nDefaultCommState = _ttoi(szValue);

	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(L"GEM", L"DefaultControlState", L"", szValue, sizeof(szValue), strPath))
	{
		;
		/*CString strControl = szValue;
		strControl.MakeUpper();
		if (strControl == L"OFFLINE"))
			m_nDefaultControlState	= DFFAULT_EQUIPMENT_OFFLINE;
		else if (strControl == L"LOCAL"))
			m_nDefaultControlState = DEFAULT_ONLINE_LOCAL;
		else if (strControl == L"REMOTE"))
			m_nDefaultControlState = DEFAULT_ONLINE_REMOTE;*/
	}

	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(L"GEM", L"CommRequestTimeout",L"",szValue,sizeof(szValue), strPath))
		m_nCommReqeustTimeout = _ttoi(szValue);

	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(L"GEM", L"MDLN",L"",szValue,sizeof(szValue), strPath))
		wsprintf(m_szModelName, szValue);

	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(L"GEM", L"SOFTREV",L"",szValue,sizeof(szValue), strPath))
		wsprintf(m_szSoftRev, szValue);

	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(L"GEM", L"SpoolingMode",L"",szValue, sizeof(szValue), strPath))
		SPOOLINFO.bSpoolEnabled = (_ttoi(szValue) == 1);

	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(L"GEM",L"MaxSpoolCount",L"",szValue, sizeof(szValue), strPath))
		SPOOLINFO.nMaxSpoolSize = _ttoi(szValue);

	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(L"GEM", L"MaxSpoolTransmit",L"",szValue,sizeof(szValue), strPath))
		SPOOLINFO.nMaxSpoolTransmit = _ttoi(szValue);

	ZeroMemory(szValue, 0xFF); 
	if (GetPrivateProfileString(L"GEM", L"SpoolOverwrite",L"",szValue,sizeof(szValue), strPath))
		SPOOLINFO.bSpoolOverwrite = (_ttoi(szValue) == 1);

	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(L"GEM", L"TimeFormat",L"",szValue,sizeof(szValue), strPath))
		m_nTimeFormat = _ttoi(szValue);

	ZeroMemory(szValue, 0xFF);
	if (GetPrivateProfileString(L"GEM", L"JobInfoOperatorConfirm",L"",szValue,sizeof(szValue), strPath))
		m_bJobInfoConfirm = (_ttoi(szValue) == 1);

	return TRUE;
}

BOOL CWLP_GEM_AgentDlg::LoadGemConfig()
{
	if(!ReadConfigFile())
	{
		//2014-09-16 GEM 설정파일 없으면 종료 - Make it simple...
		AfxGetMainWnd()->PostMessage(WM_QUIT);
		return FALSE;
	}

	AddSVID();			//SVID 추가
	AddCEID();			//CEID[EVENT] 추가
	AddECID();			//Equipment Constant ID
	AddALID();          //Alarm ID
	//////////////////////////////////////////////////////
	//			기본 통신관련 파라미터 설정		
	//			GEM.INI에서 읽어들인 내용 적용
	m_GEM.SetDeviceID((short)m_nDeviceID);
	m_GEM.SetIP(m_strIP);
	m_GEM.SetPort((short)m_nPort);
	m_GEM.SetRetry((short)m_nRetry);
	m_GEM.SetLinkTestInterval(m_nLinkInterval);
	m_GEM.SetT3((short)m_nT3);
	m_GEM.SetT5((short)m_nT5);
	m_GEM.SetT6((short)m_nT6);
	m_GEM.SetT7((short)m_nT7);
	m_GEM.SetT8((short)m_nT8);

	if(m_strMODE == L"ACTIVE")
		m_GEM.SetPassiveMode(FALSE);
	
	else if(m_strMODE == L"PASSIVE")
		m_GEM.SetPassiveMode(TRUE);
		
	//				Remote Command  추가
	m_GEM.AddRemoteCommand(L"PP-SELECT");
	m_GEM.AddRemoteCommand(L"START");
	m_GEM.AddRemoteCommand(L"UNLOAD");

	m_GEM.SetEstablishCommRetryTimer(m_nCommReqeustTimeout);

	m_GEM.SetModelName(m_szModelName);		//GEM.INI에서 읽어들인 Model Name
	m_GEM.SetSoftwareRev(m_szSoftRev);		//GEM.INI에서 읽어들인 Software Revision

	return TRUE;
}

CString CWLP_GEM_AgentDlg::GetExecuteDirectory()
{
	CString strFolderPath;

	// 현재 프로그램의 실행 파일이 있는 폴더명을 추출함
	::GetModuleFileName(NULL, strFolderPath.GetBuffer(MAX_PATH), MAX_PATH);
	strFolderPath.ReleaseBuffer();
	if (strFolderPath.Find('\\') != -1)
	{
		for (int i = strFolderPath.GetLength() - 1; i >= 0; i--) 
		{
			TCHAR ch = strFolderPath[i];
			strFolderPath.Delete(i);
			if (ch == '\\') break; 
		}
	}
	return strFolderPath;
}
//S6, F11  Event Report Send (ERS)					   	    H <- E
//S5, F1   Alarm Report Send (ARS)						    H <- E

//S2, F41  Host Command Send (HCS)					        H -> E
//S10, F3  Terminal Display, Single (VTN)					H -> E

void CWLP_GEM_AgentDlg::ProcGEM_FromEQ(CString strRcv)
{
	CString strMsg = L"[RCV]" + strRcv;
	AddLogTCP(strMsg);
	GetLog()->Debug(strMsg.GetBuffer());

	CString strCommand,strPacketBody;
	CString strSend;

	BOOL bRet = FALSE;
	int  nRet = -1;

	strCommand = strRcv.Left(3);
	strPacketBody = strRcv.Mid(8);

	if(strCommand == L"STA")
	{
		bRet = GEMStart();

		if(	bRet == TRUE)
		{
			strSend = L"STA0011|OK|";
			m_GEM.GoOnlineRemote(); //2016-02-11 ERS만! RCMD 처리안함. 실질적으로 OnlineLocal...
			m_bOnlineRemote = FALSE;
		}
	}
	else if(strCommand == L"STP")
	{
		GEMStop(); 
		
		//nRet = SendERS(strPacketBody);
		strSend = L"STP0011|OK|";
	}
	else if(strCommand == L"END")
	{
		PostQuitMessage(WM_QUIT);
	}
	else if(strCommand == L"ERS") //2016-06-06 1001,1002,1003 추가 - 장비에서 Online Local,Remote 설정시 이벤트
	{
		nRet = SendERS(strPacketBody);
		if(nRet >= 0)
		{
			strSend = L"ERS0011|OK|";
		}
		else
		{
			strSend = L"ERS0011|NG|";
		}
	}
	else if(strCommand == L"ARS")
	{
		nRet = SendARS(strPacketBody);
		if(nRet >= 0)
		{
			strSend = L"ARS0011|OK|";
		}
		else
		{
			strSend = L"ARS0011|NG|";
		}
	}
	else if(strCommand == L"TDS")
	{
		return;
	}
	else
	{
		strSend.Format(L"%s - Unknown Command",strCommand);
	}

	if(m_ListenSocket.SendData(LOCAL_HOST,strSend) == TRUE)
	{
		strMsg.Format(L"[SND]%s",strSend);
		AddLogTCP(strMsg);
		GetLog()->Debug(strMsg.GetBuffer());
	}
}

int CWLP_GEM_AgentDlg::SendERS(CString strPacketBody)
{
	CString strCEID = strPacketBody.Left(4);
	CString strSV = strPacketBody.Mid(5);
	CString strValue;

	int nIdx = -1;
	int nIdxPrev = -1;

	//recv packet ex) ERS0017|1200|2|2|
	if(strCEID == L"1600" || strCEID == L"2600" || strCEID == L"1400") // CASSETTE_START(1600), UNLOAD_COMPLETE(2600), LOAD_COMPLETE(1400)
	{
		nIdx = strSV.Find(L"|");
		strValue = strSV.Left(nIdx);
		nIdxPrev = nIdx;
		m_GEM.SetCurrentStatusValue(SVID_CASSETTE_ID, strValue); //CASSETTE_ID - 2210
		//2016-06-01 Garbage처리- BCR_READ(1200)
		if(strCEID == L"1600") //CASSETTE START(1600)
		{
			m_strStartPortID = strValue;
		}

		nIdx = strSV.Find(L"|",nIdxPrev+1);
		strValue = strSV.Mid(nIdxPrev+1, nIdx-nIdxPrev -1);
		m_GEM.SetCurrentStatusValue(SVID_PORT_ID, strValue); //PORT_ID - 2220
	}
	else if(strCEID == L"1200" ) // BCR_READ(1200)
	{
		nIdx = strSV.Find(L"|");
		strValue = strSV.Left(nIdx);
		//2016-06-01 Garbage처리
		if(_wtoi(strValue) > 3 || _wtoi(strValue) < 0)
		{
			strValue = m_strStartPortID;
		}
		nIdxPrev = nIdx;
		m_GEM.SetCurrentStatusValue(SVID_CASSETTE_ID, strValue); //CASSETTE_ID - 2210

		nIdx = strSV.Find(L"|",nIdxPrev+1);
		strValue = strSV.Mid(nIdxPrev+1, nIdx-nIdxPrev -1);
		//2016-06-01 Garbage처리
		if(_wtoi(strValue) > 3 || _wtoi(strValue) < 0)
		{
			strValue = m_strStartPortID;
		}
		m_GEM.SetCurrentStatusValue(SVID_PORT_ID, strValue); //PORT_ID - 2220
	}
	//else if(strCEID == L"1300" || strCEID == L"2500") //LOAD_REQUEST, UNLOAD REQUEST
	//{
	//	nIdx = strSV.Find(L"|");
	//	strValue = strSV.Left(nIdx);
	//	m_GEM.SetCurrentStatusValue(SVID_PORT_ID, strValue); //PORT_ID -  2220
	//}
	else if(strCEID == L"1500") //PP-SELECTED 
	{
		nIdx = strSV.Find(L"|");
		strValue = strSV.Left(nIdx);
		nIdxPrev = nIdx;
		//2016-06-01 Garbage처리
		if(_wtoi(strValue) > 3 || _wtoi(strValue) < 0)
		{
			strValue = m_strStartPortID;
		}
		m_GEM.SetCurrentStatusValue(SVID_PORT_ID, strValue); //PORT_ID - 2220

		nIdx = strSV.Find(L"|",nIdxPrev+1);
		strValue = strSV.Mid(nIdxPrev+1, nIdx-nIdxPrev -1);
		m_GEM.SetCurrentStatusValue(SVID_PPID, strValue); //PPID - 2010
	}
	else if(strCEID == L"1700" || strCEID == L"1701" || strCEID == L"1800" || strCEID == L"2000" || strCEID == L"2100" || //WAFER LOAD, WAFER LOAD FIRST, WAFER START,WAFER END,WAFER UNLOAD
			strCEID == L"2101" ) //WAFER UNLOAD LAST
	{
		nIdx = strSV.Find(L"|");
		strValue = strSV.Left(nIdx);
		nIdxPrev = nIdx;
		m_GEM.SetCurrentStatusValue(SVID_WAFER_ID, strValue); //WAFER_ID - 2230

		nIdx = strSV.Find(L"|",nIdxPrev+1);
		strValue = strSV.Mid(nIdxPrev+1, nIdx-nIdxPrev -1);
		m_GEM.SetCurrentStatusValue(SVID_SLOT_ID, strValue); //SLOT_ID - 2240
	}
	else if(strCEID == L"1900") //WAFER DCOL //ERS0089|1900|NDZK4076400|2016-01-14 09:57:43/306523 ms/80.1  /7121/0/0/26/22/0/1/0/1720/|
	{
		nIdx = strSV.Find(L"|");
		strValue = strSV.Left(nIdx);
		nIdxPrev = nIdx;
		m_GEM.SetCurrentStatusValue(SVID_WAFER_ID, strValue); //WAFER_ID - 2230

		nIdx = strSV.Find(L"|",nIdxPrev+1);
		CString strResult = strSV.Mid(nIdxPrev+1, nIdx-nIdxPrev -1);

		AfxExtractSubString(strValue, strResult, 0, '/'); //
		m_GEM.SetCurrentStatusValue(DVID_INSP_TIME, strValue); // 3110 - 검사시간

		AfxExtractSubString(strValue, strResult, 1, '/'); //
		m_GEM.SetCurrentStatusValue(DVID_INSP_EXEC_TIME	, strValue); // 3120 - 검사소요시간

		AfxExtractSubString(strValue, strResult, 2, '/'); //
		m_GEM.SetCurrentStatusValue(DVID_YIELD, strValue); // 3130 - 수율

		AfxExtractSubString(strValue, strResult, 3, '/'); //
		m_GEM.SetCurrentStatusValue(DVID_GOOD, strValue); // 3140 - GOOD

		AfxExtractSubString(strValue, strResult, 4, '/'); //
		m_GEM.SetCurrentStatusValue(DVID_CHIPOUT_NG, strValue); // 3150 - Chipout(칩없음) - 91//2016-02-19 현업 NG명,코드(9x)

		AfxExtractSubString(strValue, strResult, 5, '/'); //
		m_GEM.SetCurrentStatusValue(DVID_DICING_ALINE_NG, strValue); // 3160 - Dicing Align(얼라인) - 92

		AfxExtractSubString(strValue, strResult, 6, '/'); //
		m_GEM.SetCurrentStatusValue(DVID_CHIPPING_NG, strValue); // 3170 - 칩핑 - 93

		AfxExtractSubString(strValue, strResult, 7, '/'); //
		m_GEM.SetCurrentStatusValue(DVID_PAD_NG, strValue); // 3180 - Pad(도금형상불량) - 94

		AfxExtractSubString(strValue, strResult, 8, '/'); //
		m_GEM.SetCurrentStatusValue(DVID_FM_NG, strValue); // 3190 - F/M(이물) - 95

		AfxExtractSubString(strValue, strResult, 9, '/'); //
		m_GEM.SetCurrentStatusValue(DVID_CRACK_NG, strValue); // 3200 - Crack(크랙) - 96

		AfxExtractSubString(strValue, strResult, 10, '/'); //
		m_GEM.SetCurrentStatusValue(DVID_PRE_NG, strValue); // 3210 - P/N - 이전 공정 - 97
	
		AfxExtractSubString(strValue, strResult, 11, '/'); //
		m_GEM.SetCurrentStatusValue(DVID_MULTI_NG, strValue); // 3220 - Multiple NG - - 98

	}
	//else if(strCEID == L"2300") //DATA COLLECTION
	//{
	//	nIdx = strSV.Find(L"|");
	//	strValue = strSV.Left(nIdx);
	//	nIdxPrev = nIdx;
	//	m_GEM.SetCurrentStatusValue(SVID_CASSETTE_ID, strValue); //CASSETTE_ID - 2210

	//	nIdx = strSV.Find(L"|",nIdxPrev+1);
	//	strValue = strSV.Mid(nIdxPrev+1, nIdx-nIdxPrev-1);
	//	nIdxPrev = nIdx;
	//	m_GEM.SetCurrentStatusValue(DVID_TPWC, strValue); //TPWC - 3100
	//	
	//	nIdx = strSV.Find(L"|",nIdxPrev+1);
	//	strValue = strSV.Mid(nIdxPrev+1, nIdx-nIdxPrev -1);
	//	m_GEM.SetCurrentStatusValue(DVID_COLLECTION_DATA, strValue); //Collection Data - 3000
	//}
	//else if(strCEID == L"2400") //CASSETTE END
	//{
	//	nIdx = strSV.Find(L"|");
	//	strValue = strSV.Left(nIdx);
	//	nIdxPrev = nIdx;
	//	m_GEM.SetCurrentStatusValue(SVID_CASSETTE_ID, strValue); //CASSETTE_ID - 2210

	//	nIdx = strSV.Find(L"|",nIdxPrev+1);
	//	strValue = strSV.Mid(nIdxPrev+1, nIdx-nIdxPrev-1);
	//	nIdxPrev = nIdx;
	//	m_GEM.SetCurrentStatusValue(SVID_PORT_ID, strValue); //PORT_ID - 2220
	//	
	//	nIdx = strSV.Find(L"|",nIdxPrev+1);
	//	strValue = strSV.Mid(nIdxPrev+1, nIdx-nIdxPrev -1);
	//	m_GEM.SetCurrentStatusValue(DVID_TPWC, strValue); //TPWC - 3100
	//}
	//else if(strCEID == L"3000") //Processing State Change
	//{
	//	nIdx = strSV.Find(L"|");
	//	strValue = strSV.Left(nIdx);
	//	m_GEM.SetCurrentStatusValue(SVID_STATE, strValue); //STATE -  2250
	//}
	else if(strCEID == L"1910") //MAP DCOL
	{
		nIdx = strSV.Find(L"|");
		strValue = strSV.Left(nIdx);
		nIdxPrev = nIdx;
		m_GEM.SetCurrentStatusValue(SVID_WAFER_ID, strValue); //WAFER_ID - 2230

		nIdx = strSV.Find(L"|",nIdxPrev+1);
		strValue = strSV.Mid(nIdxPrev+1, nIdx-nIdxPrev -1);
		m_GEM.SetCurrentStatusValue(DVID_MAP_DATA, strValue); //MAP_DATA - 3230
	}
	else if(strCEID == L"1001" || strCEID == L"1002" || strCEID == L"1003") //CONTROL STATE 
	{
		//2016-06-06
		if(strCEID == L"1003")
		{
			m_bOnlineRemote = TRUE;
		}
		else
		{
			m_bOnlineRemote = FALSE;
		}
		//
		nIdx = strSV.Find(L"|");
		strValue = strSV.Left(nIdx);
		m_GEM.SetCurrentStatusValue(SVID_CONTROL_STATE, strValue); //CONTROL_STATE -  2260
	}
	else
	{
		return -1;
	}

	long lCEID = _wtoi(strCEID);
	int nRet = m_GEM.SendEventReport(lCEID);

	if(strCEID == L"1500" && nRet >= 0) 
	{
		m_nHCS_PP_Status = 3;
	}

	return nRet;
}

int CWLP_GEM_AgentDlg::SendARS(CString strPacketBody)
{

	CString strALID = strPacketBody.Left(4);
	CString strSet = strPacketBody.Mid(5,1); //1: Alarm Set, 0: Alarm Clear
	
	long  ALID = _wtoi(strALID);
	short nALCD = 0x00;

	if(strSet == L"1")
	{
		nALCD = 0x80 + m_GEM.GetAlarmCode(ALID); 
	}
	else if (strSet == L"0")
	{
		nALCD = 0x00;
	}

	int nRet = m_GEM.SendAlarmReport(ALID, nALCD);

	return nRet;

}

BOOL CWLP_GEM_AgentDlg::ProcGEM_ToEQ(CString strSendPacket)
{
	BOOL bRet = FALSE;
	bRet = m_ListenSocket.SendData(LOCAL_HOST,strSendPacket);
	if(bRet == TRUE)
	{
		CString strMsg;
		strMsg.Format(L"[SND]%s",strSendPacket);
		AddLogTCP(strMsg);
		GetLog()->Debug(strMsg.GetBuffer());
	}
	return bRet;
}

//해야하나? Trace Data 하게 되면, SV유지해야하지 않나? 확인 要 
//void CWLP_GEM_AgentDlg::RemoveSV(int CEID)
//{
//	
//	/*switch(CEID)
//	{
//		case 1200:
//		case 1300:
//			break;
//
//	}*/
//
//	CString strValue = L"";
//	
//	long lSVID = 1001;
//	while(lSVID <= 1003) //SVID 1001~1003
//	{
//		m_GEM.SetCurrentStatusValue(lSVID, strValue);
//		lSVID += 1;
//	}
//	lSVID = 1010;
//	while(lSVID <= 1740) //SVID 1010~1740
//	{
//		m_GEM.SetCurrentStatusValue(lSVID, strValue);
//		lSVID += 10;
//	}
//	
//	lSVID = 2100;
//	while(lSVID <= 2900) //SVID 1010~1740
//	{
//		m_GEM.SetCurrentStatusValue(lSVID, strValue);
//		lSVID += 100;
//	}
//
//}

void CWLP_GEM_AgentDlg::OnConnectedEzgemctrl1()
{
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.

	AddLogGEM(L"Connected.");
}

void CWLP_GEM_AgentDlg::OnDisconnectedEzgemctrl1()
{
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	AddLogGEM(L"Disonnected.");

	CString strPacket, strMsg;
	strPacket = L"OFF0008|";

	m_bOnlineRemote = FALSE;
	
	ProcGEM_ToEQ(strPacket); //OFF-LINE
}

void CWLP_GEM_AgentDlg::OnSecsMsgInEzgemctrl1(short nStream, short nFunction, short nWbit, long lSysByte)
{
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	CString str;
	if(nWbit == 1)
		str.Format(L"[SECS-II:IN]S%dF%dW",nStream,nFunction);
	else
		str.Format(L"[SECS-II:IN]S%dF%d",nStream,nFunction);

	AddLogGEM(str);

	if((nStream == 1 && nFunction == 13) || (nStream == 1 && nFunction == 14) )//EAP 접속시
	{
		SetTimer(IDD+1, 1000, NULL);
	}
}

void CWLP_GEM_AgentDlg::OnSecsMsgOutEzgemctrl1(short nStream, short nFunction, short nWbit, long lSysByte)
{
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	CString str;
	if(nWbit == 1)
		str.Format(L"[SECS-II:OUT]S%dF%dW",nStream,nFunction);
	else
		str.Format(L"[SECS-II:OUT]S%dF%d",nStream,nFunction);

	AddLogGEM(str);
}

//EQ에서 S1,F13을 보내고, 그 응답을 받으면...ENVIA GEM driver Manual에는 그렇게 되어있으나, 실제 동작안함. 
void CWLP_GEM_AgentDlg::OnEstablishCommRequestEzgemctrl1(long lMsgId)
{
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	//m_GEM.SetCurrentStatusValue(SVID_STATE, L"1"); //SVID_STATE 2250  1: COMMUNICATING
	//m_GEM.SendEventReport(3000);
	
}
/////////////////////////////////Control State
//HOST S1,F17 Request ON-LINE (RONL)
//2016-02-11 - m_GEM.DisableAutoReply(1,17);
//OnMsgRequestedEzgemctrl1(long lMsgId)에서 처리
void CWLP_GEM_AgentDlg::OnOnlineRequestEzgemctrl1(long lMsgId)
{
}
//HOST S!,F17 ONLINE 허용했을때 발생하는 이벤트
//2016-02-11 m_GEM.GoOnlineRemote() 에서도 발생.
void CWLP_GEM_AgentDlg::OnOnlineRemoteEzgemctrl1()
{
	////if(m_bOnlineRemote == TRUE) //2016-02-11 HOST로부터 HOST S1,F17 Request ON-LINE (RONL) 받았을 때만...
	//{
	//	CString strPacket, strMsg;
	//	strPacket = L"RON0008|";
	//
	//	//if(ProcGEM_ToEQ(strPacket) == TRUE) //ON-LINE Remote
	//	//{
	//	//	strMsg.Format(L"[SND]%s",strPacket);
	//	//	AddLogTCP(strMsg);
	//	//	GetLog()->Debug(strMsg.GetBuffer());
	//	//} 2016-06-06 log 중복 - 막음.
	//	ProcGEM_ToEQ(strPacket);
	//}
	;
}
//
//S1, F15  Request OFF-LINE (ROFL)	- Online 일때...
//2016-02-11 - m_GEM.DisableAutoReply(1,15);
//OnMsgRequestedEzgemctrl1(long lMsgId)에서 처리
void CWLP_GEM_AgentDlg::OnOfflineRequestEzgemctrl1(long lMsgId)
{
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	;
}

//S2,F31
void CWLP_GEM_AgentDlg::OnDateTimeSetRequestEzgemctrl1(long lMsgId, LPCTSTR strNewDateTime)
{
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
//TO-TEST
	SYSTEMTIME st;
	ZeroMemory(&st,sizeof(st));

	CString strDateTime = strNewDateTime;

	st.wYear = _wtoi(strDateTime.Left(4));
	st.wMonth = _wtoi(strDateTime.Mid(2,2));
	st.wDay = _wtoi(strDateTime.Mid(4,2));
	st.wHour = _wtoi(strDateTime.Mid(6,2));
	st.wMinute =_wtoi(strDateTime.Mid(8,2));
	st.wSecond = _wtoi(strDateTime.Mid(10,2));

	SetLocalTime(&st);

	m_GEM.AcceptAndSetNewDateTime(lMsgId,strNewDateTime);
}

//S2,F41
void CWLP_GEM_AgentDlg::OnRemoteCommandEzgemctrl1(long lMsgId, LPCTSTR strCommand, short nParamCount)
{
	// (ACK CODE
	// 0 = ACKNOWLEDGE, COMMAND PERFORMED
	
	// (NAK CODE
	// 1 = CANNOT PERFORM NOW
	// 2 = PARAMETER IS INVALID
	// 3 = ALREADY IN DESIRED CONDITION
	// 4 = UNABLE TO PERFORM, INTERNAL ERROR OCCURRED
//2016-06-06 OnlineRemote 일때만 처리
	
	if(m_bOnlineRemote == FALSE) //OnlineLocal 일때 처리안함.
	{
		short nNAK = 0x01;
		m_GEM.DenyRemoteCommand(lMsgId,strCommand,nNAK);
		return;
	}

	CString strParamName, strParamValue;
	CString strPacketBody = L""; //VISION 전송용 Packet Body;
	int nTotalPacketSize = 0;
	CString	strSendPacket = L"";

	if(wcscmp(strCommand,_T("PP-SELECT")) == 0 || wcscmp(strCommand,_T("START")) == 0 || wcscmp(strCommand,_T("UNLOAD")) == 0)
	{
		if(wcscmp(strCommand,_T("PP-SELECT")) == 0)
		{
			m_nHCS_PP_Status = 0;

			SetTimer(IDD+100,2000,NULL);
		}

		strPacketBody.Format(L"%s|",strCommand);

		for (int i = 0;i < nParamCount; i++) 
		{
			strParamName = m_GEM.GetRemoteCommandParamName(lMsgId, strCommand, i);
			strPacketBody += strParamName;
			strPacketBody += L"|";
			strParamValue = m_GEM.GetRemoteCommandParamValue(lMsgId, strCommand, i);

			if(strParamName == L"PORT_ID")
			{
				if(CheckPort(strParamValue) == FALSE)
				{
					// REMOTE COMMAND NAK
					//2016-01-25 DenyRemoteCommand 쓰지말것? NVIAsoft 문의시 이상한 대댭. NAK Code 는 (-) 아님. 매뉴얼 헷갈림. B 0x00 아니면 NAK
					/* ACK
					<S2F42
					   <L[2/1]      
						  <B[1/1] 0x00>
						  <L[0/1]>
					   >
					>*/
					/* NAK
					<S2F42
					   <L[2/1]      
						  <B[1/1] 0x02>
						  <L[0/1]>
					   >
					>*/
					m_GEM.DenyRemoteCommand(lMsgId,strCommand,2); // PARAMETER IS INVALID
					//m_GEM.ReplyRemoteCommand(lMsgId,strCommand,2);

					return;
				}
			}
			else if(strParamName == L"PPID")
			{
	
				if(CheckPP(strParamValue) == FALSE)
				{
					m_GEM.DenyRemoteCommand(lMsgId,strCommand,2); // PARAMETER IS INVALID
					return;
				}
			}
//
			strPacketBody += strParamValue;
			strPacketBody += L"|";
		}
		
		nTotalPacketSize = strPacketBody.GetLength() + 8;
		strSendPacket.Format(L"HCS%04d|%s",nTotalPacketSize,strPacketBody);

		if(wcscmp(strCommand,_T("PP-SELECT")) == 0)
		{
			m_strHCS_PP_SELECT = strSendPacket;
			m_nHCS_PP_Status = 1;
		}

		BOOL bRet = ProcGEM_ToEQ(strSendPacket);

		if(wcscmp(strCommand,_T("PP-SELECT")) == 0 && bRet == TRUE)
		{
			m_strHCS_PP_SELECT = strSendPacket;
			m_nHCS_PP_Status = 2;
		}

		// REMOTE COMMAND 수락
		short nCommack = 0x00; //OK..
		m_GEM.AcceptRemoteCommand(lMsgId,strCommand,nCommack);
	}
}

BOOL CWLP_GEM_AgentDlg::CheckPort(CString strParamValue)
{
	CString strCurPort = m_GEM.GetCurrentStatusValue(SVID_PORT_ID);

	if(strParamValue == strCurPort || strParamValue == L"65536") //2015.03.31 PI팀 검수 - 10000000000000000 ->  2^16 - PLC 
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CWLP_GEM_AgentDlg::CheckPP(CString strParamValue)
{
	GetRecipeList(m_strListRecipe);

	BOOL bRet = FALSE;

	for(int i = 0; i < m_strListRecipe.GetCount(); i++)
	{
		if(strParamValue == m_strListRecipe.GetAt(m_strListRecipe.FindIndex(i)))
		{
			bRet = TRUE;
			break;
		}
	}

	return bRet;
}

//S7,F5 Process Program Request (PPR) / S7,F6 Process Program Data (PPD) - Unformatted
void CWLP_GEM_AgentDlg::OnMsgRequestedEzgemctrl1(long lMsgId)
{
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	short nStream, nFunction, nWbit;//, nCommack;
	long lReplyMsgId = 0;
	short nACK;

	long lLength;
	CString strPPID;

	m_GEM.GetMsgInfo(lMsgId, &nStream, &nFunction, &nWbit, &lLength);

	//S1, F15  Request OFF-LINE (ROFL)
	if(nStream == 1 && nFunction == 15 && nWbit == 1)
	{ 
		//if(m_bEqConnect == TRUE)
		{
			lReplyMsgId = m_GEM.CreateReplyMsg(lMsgId); //S1F16
			nACK = 0x00;
			m_GEM.AddBinaryItem(lReplyMsgId, &nACK, 1);
			m_GEM.SendMsg(lReplyMsgId);

			//ERS
			//m_GEM.SetCurrentStatusValue(SVID_CONTROL_STATE, L"OFF-LINE"); //S6F11
			m_GEM.SetCurrentStatusValue(SVID_CONTROL_STATE, L"OF"); //S6F11 // 2016-05-31

			long lCEID =1001; //OFF-LINE
			int nRet = m_GEM.SendEventReport(lCEID);

			//2016-03-17 OFF-LINE	
			CString strPacket, strMsg;
			strPacket = L"OFF0008|";

			m_bOnlineRemote = FALSE;
			ProcGEM_ToEQ(strPacket); //OFF-LINE
			//{
			//	//strPacket = L"OFF0008|";
	
			//	//if(ProcGEM_ToEQ(strPacket) == TRUE) //ON-LINE LOCAL
			//	//{
			//	

			//	//GEMStop(); 2015.03.31 PI팀 검수시 주석처리- OFF-LINE 이라도 연결은 유지 

			//	//m_GEM.GoOffline(); //이후 HOST req에 SxF0 올려야 하는데, 표준과 다름. 이상함. - TO KNOW... 필요시 수작업 처리	

			//	//2016.04.18 TO-DO 
			//		//reply S1,F16

			//}
		}
	}
	//HOST S1,F17 Request ON-LINE (RONL)
	if(nStream == 1 && nFunction == 17 && nWbit == 1)
	{
		//REQUEST ONLINE
		m_GEM.GoOnlineRemote(); //Request Online Remote
		m_GEM.AcceptOnlineRequest(lMsgId); //ONLINE 허용;	S1S18
		//ERS
		////m_GEM.SetCurrentStatusValue(SVID_CONTROL_STATE, L"ONLINE-REMOTE"); //S6F11
		//m_GEM.SetCurrentStatusValue(SVID_CONTROL_STATE, L"OR"); //S6F11 // 2016-05-31
		//
		//long lCEID =1003; //ONLINE-REMOTE
		//int nRet = m_GEM.SendEventReport(lCEID);

		//CString strPacket, strMsg;
		//strPacket = L"RON0008|";

		//m_bOnlineRemote = TRUE;
		//ProcGEM_ToEQ(strPacket);
		m_bOnlineRemote = TRUE;
		//SetTimer(IDD+2, 1000, NULL);

	}

	//S7,F3 Process Program Send (PPS)  / S7,F4 Process Program Acknowledge (PPA) - Unformatted
	if(nStream == 7 && nFunction == 3 && nWbit == 1)
	{
		CString strFullpath = _T("");
		nACK = 0x00;
		m_GEM.GetListItemOpen(lMsgId);
		strPPID = m_GEM.GetAsciiItemString(lMsgId);
		
		//::CreateDirectory(EPPD, NULL);

		strFullpath = EPPD + strPPID; //EPPD - C:\\HITS\\RECIPE\\
		
		::CreateDirectory(strFullpath, NULL);

		//m_GEM.GetFileBinaryItem(lMsgId, strFullpath);
		m_GEM.GetFileBinaryItem(lMsgId, strFullpath + L"\\ModelInfo.ini");

		m_GEM.GetListItemClose(lMsgId);

		lReplyMsgId = m_GEM.CreateReplyMsg(lMsgId); //S7F4
		m_GEM.AddBinaryItem(lReplyMsgId, &nACK, 1);
		m_GEM.SendMsg(lReplyMsgId);
	}
	//S7,F5 Process Program Request (PPR) / S7,F6 Process Program Data (PPD) - Unformatted
	if(nStream == 7 && nFunction == 5 && nWbit == 1)
	{
		CString strFullpath = _T("");
	
		strPPID = m_GEM.GetAsciiItemString(lMsgId);
		
		//strFullpath = EPPD + strPPID;
		strFullpath = EPPD + strPPID + L"\\ModelInfo.ini"; //EPPD - C:\\HITS\\RECIPE\\ 

		lReplyMsgId = m_GEM.CreateReplyMsg(lMsgId); //S7F6
	
		m_GEM.OpenListItem(lReplyMsgId);
	
		m_GEM.AddAsciiItem(lReplyMsgId, strPPID, strPPID.GetLength());

		m_GEM.AddFileBinaryItem(lReplyMsgId, strFullpath);
	
		m_GEM.CloseListItem(lReplyMsgId);
	
		m_GEM.SendMsg(lReplyMsgId);

		m_GEM.CloseMsg(lReplyMsgId);
	}
	//S7, F19 Current EPPD Request / S7,F20 Current EPPD Data	
	else if(nStream == 7 && nFunction == 19 && nWbit == 1) //2015-11-26 temp
	{
		GetRecipeList(m_strListRecipe);
		CString strRecipe;
		lReplyMsgId = m_GEM.CreateReplyMsg(lMsgId); //S7F20
		m_GEM.OpenListItem(lReplyMsgId);
	
		for(int i = 0; i < m_strListRecipe.GetCount(); i++)
		{
			strRecipe = m_strListRecipe.GetAt(m_strListRecipe.FindIndex(i));
			m_GEM.AddAsciiItem(lReplyMsgId, strRecipe, strRecipe.GetLength());
		}

		m_GEM.CloseListItem(lReplyMsgId);
		m_GEM.SendMsg(lReplyMsgId);
		m_GEM.CloseMsg(lReplyMsgId);
	}
}

void CWLP_GEM_AgentDlg::OnBnClickedBtRecipe()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GetRecipeList(m_strListRecipe);

	CString strTemp;
	
	for(int i = 0; i < m_strListRecipe.GetCount(); i++)
	{
		strTemp += m_strListRecipe.GetAt(m_strListRecipe.FindIndex(i));
		strTemp += L"\n";
	}

	MessageBox(strTemp, L"Recipe List", MB_OK|MB_ICONINFORMATION);	
}

void CWLP_GEM_AgentDlg::GetRecipeList(CStringList &strList)
{
	strList.RemoveAll();

	//CString strPath = ;

	CString strWildcard(EPPD);
    strWildcard += _T("*");

	CFileFind finder;

	CString strDirName;
	BOOL bWorking = finder.FindFile(strWildcard);

	while(bWorking)
	{
		bWorking = finder.FindNextFileW();

		if (finder.IsDots()) //. & .. 제외
		{
			continue;
		}

		if(finder.IsDirectory())
		{
			strDirName = finder.GetFileName();
			strList.AddTail(strDirName);
		}
	}
}

void  CWLP_GEM_AgentDlg::SelfShutDown()
{
	SetTimer(IDD,1000*5,NULL);
}

void CWLP_GEM_AgentDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if(nIDEvent == IDD)
	{
		/*if(m_bEqConnect == TRUE)
		{
			KillTimer(IDD);
		}
		else if(m_bEqConnect == FALSE)
		{
			PostQuitMessage(WM_QUIT);
		}*/
		PostQuitMessage(WM_QUIT); //2016-06-06 일단 종료후 재실행 유도
	}
	else if(nIDEvent == IDD+1) //ONLINE-LOCAL
	{
		//2016-03-16
		//m_GEM.SetCurrentStatusValue(SVID_CONTROL_STATE, L"ONLINE-LOCAL"); //
		m_GEM.SetCurrentStatusValue(SVID_CONTROL_STATE, L"OL"); // 2016-05-31

		long lCEID =1002;
		int nRet = m_GEM.SendEventReport(lCEID);

		CString strPacket, strMsg;
		strPacket = L"ROL0008|";
	
		if(m_bEqConnect == FALSE)
		{
			return; //접속이후...
		}

		BOOL bRet = ProcGEM_ToEQ(strPacket);

		/*if(bRet == TRUE)
		{
			m_bOnlineRemote = FALSE;
		}*/

		if(nRet >= 0 && bRet == TRUE) //ON-LINE LOCAL
		{
			KillTimer(IDD+1);
			strMsg = L"ONLINE-LOCAL ERS - ProcGEM_ToEQ('ROL0008|')";
			GetLog()->Debug(strMsg.GetBuffer());
		}
		else
		{
			if(nRet < 0)
			{
				strMsg = L"[ERR] ONLINE-LOCAL ERS";
			}
			else if(bRet == FALSE)
			{
				strMsg = L"[ERR] ProcGEM_ToEQ('ROL0008|')";
			}
			if(nRet < 0 && bRet == FALSE)
			{
				strMsg = L"[ERR] ONLINE-LOCAL ERS - [ERR] ProcGEM_ToEQ('ROL0008|')";
			}
			GetLog()->Debug(strMsg.GetBuffer());
		}
	}
	else if(nIDEvent == IDD + 100) //HCS - PP-SELECT packet 재전송(TO VISION) Timer
	{
		if(m_nHCS_PP_Status == 3)
		{
			KillTimer(IDD + 100);
			return;
		}
		else if(m_nHCS_PP_Status == 1 || m_nHCS_PP_Status == 2)
		{
			ProcGEM_ToEQ(m_strHCS_PP_SELECT);
		}
	}
	else if(nIDEvent == IDD+2) //ONLINE-REMOTE
	{
		//2016-03-16
		//m_GEM.SetCurrentStatusValue(SVID_CONTROL_STATE, L"ONLINE-LOCAL"); //
		m_GEM.SetCurrentStatusValue(SVID_CONTROL_STATE, L"OR"); // 2016-05-31

		long lCEID =1003;
		int nRet = m_GEM.SendEventReport(lCEID);

		CString strPacket, strMsg;
		strPacket = L"ROR0008|";
	
		if(m_bEqConnect == FALSE)
		{
			return; //접속이후...
		}

		BOOL bRet = ProcGEM_ToEQ(strPacket);

		/*if(bRet == TRUE)
		{
			m_bOnlineRemote = TRUE;
		}*/

		if(nRet >= 0 && bRet == TRUE) //ON-LINE REMOTE
		{
			KillTimer(IDD+2);
			strMsg = L"ONLINE-REMOTE ERS - ProcGEM_ToEQ('ROR0008|')";
			GetLog()->Debug(strMsg.GetBuffer());
		}
		else
		{
			if(nRet < 0)
			{
				strMsg = L"[ERR] ONLINE-REMOTEL ERS";
			}
			else if(bRet == FALSE)
			{
				strMsg = L"[ERR] ProcGEM_ToEQ('ROR0008|')";
			}
			if(nRet < 0 && bRet == FALSE)
			{
				strMsg = L"[ERR] ONLINE-REMOTE ERS - [ERR] ProcGEM_ToEQ('ROR0008|')";
			}
			GetLog()->Debug(strMsg.GetBuffer());
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}

/*MSDN
void Recurse(LPCTSTR pstr)
{
   CFileFind finder;

   // build a string with wildcards
   CString strWildcard(pstr);
   strWildcard += _T("\\*.*");

   // start working for files
   BOOL bWorking = finder.FindFile(strWildcard);

   while (bWorking)
   {
      bWorking = finder.FindNextFile();

      // skip . and .. files; otherwise, we'd
      // recur infinitely!

      if (finder.IsDots())
         continue;

      // if it's a directory, recursively search it

      if (finder.IsDirectory())
      {
         CString str = finder.GetFilePath();
         TRACE(_T("%s\n"), (LPCTSTR)str);
         Recurse(str);
      }
   }

   finder.Close();
}

void PrintDirs()
{
   Recurse(_T("C:"));
}
*/

//S10F3 H->E Terminal Display, Single (VTN)	/S10F4 H<-E Terminal Display, Single Acknowledge (VTA) 
void CWLP_GEM_AgentDlg::OnTerminalMessageSingleEzgemctrl1(long lMsgId, LPCTSTR strMsg, short nCode)
{
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	CString strPacketBody = L""; //VISION 전송용 Packet Body;
	int nTotalPacketSize = 0;
	CString	strSendPacket = L"";

	strPacketBody = strMsg;
	//strPacketBody += L"|";
			
	nTotalPacketSize = strPacketBody.GetLength() + 8;
	strSendPacket.Format(L"TDS%04d|%s|",nTotalPacketSize,strPacketBody);

	ProcGEM_ToEQ(strSendPacket);

	m_GEM.AcceptTerminalMessage(lMsgId);
}


void CWLP_GEM_AgentDlg::OnTerminalMessageMultiEzgemctrl1(long lMsgId, short nCode, short nCount)
{
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	//2016-06-01 Single과 동일하게 처리 TDS
	CString strPacketBody = L""; //VISION 전송용 Packet Body;
	int nTotalPacketSize = 0;
	CString	strSendPacket = L"";

	CString strMsg = L"";

	for (int i=0;i<nCount;i++) {
		strMsg += m_GEM.GetTerminalMessageMulti(lMsgId, i);
		strMsg += L"\r\n";
	}

	strPacketBody = strMsg;
			
	nTotalPacketSize = strPacketBody.GetLength() + 8;
	strSendPacket.Format(L"TDS%04d|%s|",nTotalPacketSize,strPacketBody);

	ProcGEM_ToEQ(strSendPacket);

	m_GEM.AcceptTerminalMessage(lMsgId); //2016-05-31 추가
}
