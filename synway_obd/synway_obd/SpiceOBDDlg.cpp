
// SpiceOBDDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpiceOBD.h"
#include "SpiceOBDDlg.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifndef ROW_COUNT
#define ROW_COUNT 20
#endif // !1



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

														// Implementation
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


// CSpiceOBDDlg dialog



CSpiceOBDDlg::CSpiceOBDDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SYNWAY_OBD_DIALOG, pParent), logger(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

int CSpiceOBDDlg::chIndex = 0;
int CSpiceOBDDlg::OffSet = ROW_COUNT;
int CSpiceOBDDlg::row_count = ROW_COUNT;
int CSpiceOBDDlg::getAndUpdateRowCount = 0;

void CSpiceOBDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_TRK, m_TrkChList);
}

BEGIN_MESSAGE_MAP(CSpiceOBDDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CSpiceOBDDlg message handlers

BOOL CSpiceOBDDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

									// TODO: Add extra initialization here

	if (!InitCtiBoard())  return false;
	if (!SetCLIOnChannels()) return false;
	m_TrkChList.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	CRect rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
	int screen_x_size = rect.Width();
	int screen_y_size = rect.Height();

	::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, screen_x_size, screen_y_size, SWP_NOZORDER);

	InitUserDialingList();
	//InitilizeDBConnection();
	//GetDBData();
	SetTimer(1000, 2000, NULL);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

/*** Initilizes DB connection ***/
void CSpiceOBDDlg::InitilizeDBConnection()
{
	conn = mysql_init(NULL);
	//Data stored in DBSettings.INI file
	char host[255];
	char DBName[255];
	char username[255];
	char password[255];
	int port;

	// Fetching all the details required for DB connection.
	GetPrivateProfileStringA("Database", "host", "localhost", host, 255, "..\\SpiceOBD\\DBSettings.INI");
	GetPrivateProfileStringA("Database", "DBName", "test_db", DBName, 255, "..\\SpiceOBD\\DBSettings.INI");
	GetPrivateProfileStringA("Database", "username", "root", username, 255, "..\\SpiceOBD\\DBSettings.INI");
	GetPrivateProfileStringA("Database", "password", "sdl@1234", password, 255, "..\\SpiceOBD\\DBSettings.INI");

	port = GetPrivateProfileIntA("Database", "Port", 3306, "..\\SpiceOBD\\DBSettings.INI");


	if (mysql_real_connect(conn, host, username, password, DBName, port, NULL, 0) == 0)
	{
		AfxMessageBox(L"Connection failed to test_db");
	}
}

void CSpiceOBDDlg::SetChannelInitialStatus()
{
	for (int i = 0; i < nTotalCh; i++)
	{
		LVITEM lvItem;
		int nItem;
		CString /*tempStr1(row[0]), tempStr2(row[1]),*/ chNum;
		if (i < 9)
		{
			chNum.Format(_T("00%d"), i + 1);
		}
		else if (i < 99)
		{
			chNum.Format(_T("0%d"), i + 1);
		}
		else
		{
			chNum.Format(_T("%d"), i + 1);
		}
		lvItem.mask = LVIF_TEXT;
		lvItem.iItem = i;
		lvItem.iSubItem = 0;
		lvItem.pszText = L"";
		nItem = m_TrkChList.InsertItem(&lvItem);

		m_TrkChList.SetItemText(nItem, 0, chNum);
		/*m_TrkChList.SetItemText(nItem, 1, L"VasApps/SPC_CP_DT_81...");
		m_TrkChList.SetItemText(nItem, 2, L"Alerting");
		m_TrkChList.SetItemText(nItem, 3, tempStr1);
		m_TrkChList.SetItemText(nItem, 4, tempStr2);
		m_TrkChList.SetItemText(nItem, 5, L"");*/
		CString chStateNum;
		chStateNum.Format(L"%d", ChInfo[i].nStep);
		m_TrkChList.SetItemText(nItem, 6, chStateNum);

	}
}

void CSpiceOBDDlg::GetDBData()
{
	/***  Read Records  ***/

	/* call to decrypt values read from DB
	AESEncryption aesEncryption("1234567891011121");
	char* DecryptedVal = aesEncryption.DecodeAndDecrypt("TjakONXGDnXZ3I/XYmnG8g==");*/
	int query_state;
	char queryStr[256];

	sprintf_s(queryStr, "select * from obdtest where ChState = %d  limit %d", USER_IDLE, row_count);
	//sprintf_s(queryStr, "select * from obdtest where chstate = %d limit %d; update obdtest  set chstate = %d where chstate = %d limit %d", USER_IDLE, row_count, USER_IDLE + 2, USER_IDLE, row_count);

	query_state = mysql_query(conn, queryStr);

	if (query_state != 0)
	{
		CString err(mysql_error(conn));
		AfxMessageBox(err);
	}

	res = mysql_store_result(conn);
	i = 0;
	while ((row = mysql_fetch_row(res)) != NULL)
	{
		LVITEM lvItem;
		int nItem;
		CString tempStr1(row[0]), tempStr2(row[1]), chNum;
		if (i < 9)
		{
			chNum.Format(_T("00%d"), i + 1);
		}
		else if (i < 99)
		{
			chNum.Format(_T("0%d"), i + 1);
		}
		else
		{
			chNum.Format(_T("%d"), i + 1);
		}
		lvItem.mask = LVIF_TEXT;
		lvItem.iItem = i;
		lvItem.iSubItem = 0;
		lvItem.pszText = L"";
		nItem = m_TrkChList.InsertItem(&lvItem);

		m_TrkChList.SetItemText(nItem, 0, chNum);
		m_TrkChList.SetItemText(nItem, 1, L"VasApps/SPC_CP_DT_81...");
		m_TrkChList.SetItemText(nItem, 2, L"Alerting");
		m_TrkChList.SetItemText(nItem, 3, tempStr1);
		m_TrkChList.SetItemText(nItem, 4, tempStr2);
		m_TrkChList.SetItemText(nItem, 5, L"");
		m_TrkChList.SetItemText(nItem, 6, L"Idle");

		StrCpyA(ChInfo[i].pPhoNumBuf, row[1]);
		i++;
	}
}

BOOL CSpiceOBDDlg::UpdateDBData(/*int chState*/)
{
	try {
		int query_state, chState = 1;
		char queryStr1[131071], queryStr2[131071];
		StrCpyA(queryStr1, "update obdtest set ChState =  ( case ");

		for (int i = 0; i < row_count; i++)
		{
			if (ChInfo[i].EnCalled == true)
			{
				char tempStr[48];
				sprintf_s(tempStr, "when DNIS = '%s' then %d ", ChInfo[i].pPhoNumBuf, chState/*ChInfo[i].nStep*/); //TODO: coented section will be used
				StrCatA(queryStr1, tempStr);
			}
		}
		StrCpyA(queryStr2, "end) where DNIS in(");
		int i = 0;
		for (i = 0; i < row_count - 1; i++)
		{
			if (ChInfo[i].EnCalled == true)
			{
				char tempStr[48];
				sprintf_s(tempStr, "'%s', ", ChInfo[i].pPhoNumBuf);
				StrCatA(queryStr2, tempStr);
			}
		}

		//For the last one no comma required
		char tempStr[48];
		sprintf_s(tempStr, "'%s'", ChInfo[i].pPhoNumBuf);
		StrCatA(queryStr2, tempStr);

		StrCatA(queryStr2, ")");
		StrCatA(queryStr1, queryStr2);
		query_state = mysql_query(conn, queryStr1);

		if (query_state != 0)
		{
			CString err(mysql_error(conn));
			AfxMessageBox(err);
			return false;
		}
	}
	catch (CString msg)
	{
		AfxMessageBox(msg);
	}
	return true;
}

void CSpiceOBDDlg::GetNextUserData()
{
	int query_state;
	char queryStr[256];

	//sprintf_s(queryStr, "select * from obdtest where chstate = %d limit %d; update obdtest  set chstate = %d where chstate = %d limit %d", USER_IDLE, getAndUpdateRowCount, USER_IDLE + 2, USER_IDLE, getAndUpdateRowCount);
	sprintf_s(queryStr, "select * from obdtest where ChState = %d  limit %d", USER_IDLE, getAndUpdateRowCount);

	OffSet = OffSet + getAndUpdateRowCount;
	query_state = mysql_query(conn, queryStr);

	if (query_state != 0)
	{
		CString err(mysql_error(conn));
		AfxMessageBox(err);
	}

	res = mysql_store_result(conn);
	i = 0;
	while ((row = mysql_fetch_row(res)) != NULL)
	{
		if (ChInfo[i].EnCalled == true)
		{
			CString tempStr1(row[0]), tempStr2(row[1]);

			/*m_TrkChList.SetItemText(i, 1, L"VasApps/SPC_CP_DT_81...");
			m_TrkChList.SetItemText(i, 2, L"Alerting");*/
			m_TrkChList.SetItemText(i, 3, tempStr1);
			m_TrkChList.SetItemText(i, 4, tempStr2);
			//m_TrkChList.SetItemText(i, 5, L"");
			m_TrkChList.SetItemText(i, 6, L"Idle");
			ChInfo[i].nStep = USER_IDLE;
			ChInfo[i].EnCalled = false;
			StrCpyA(ChInfo[i].pPhoNumBuf, row[1]);
		}
		i++;
	}
}


void CSpiceOBDDlg::UpDateATrunkChListCtrl()
{
	CString state;
	wchar_t tmpstr[51];
	int i, nIndex;
	for (i = 0, nIndex = chIndex; i < nTotalCh; i++)
	{
		//if (SsmGetChType(i) != 2) continue;
		switch (ChInfo[i].nStep)
		{
		case USER_IDLE:					state = "Idle";			break;
		case USER_GET_PHONE_NUM:		state = "ReceiveNumber"; break;
		case USER_WAIT_DIAL_TONE:		state = "WaitForDialTone";	break;
		case USER_WAIT_REMOTE_PICKUP:	state = "WaitCalledPartyPickup"; break;
		case USER_TALKING:				state = "Talking";		break;
		case USER_WAIT_HANGUP:			state = "WaitStationHangup"; break;
		}

		m_TrkChList.GetItemText(nIndex, 6, tmpstr, 50);
		if (state != tmpstr)
			m_TrkChList.SetItemText(nIndex, 6, state.GetBuffer(50));

		/*m_TrkChList.GetItemText(nIndex, 4, tmpstr, 50);
		if (ChInfo[i].pPhoNumBuf != tmpstr)
		m_TrkChList.SetItemText(nIndex, 4, ChInfo[i].pPhoNumBuf);*/

		nIndex++;
	}
	//if (state == "Idle" && i == row_count)
	//{
	//
	//	//chIndex = chIndex + 30;
	//OffSet = OffSet + 30;
	/*if (IsUpdate)
	{
	if (UpdateDBData())
	{
	GetNextUserData();
	}
	}*/
	//}

}

void CSpiceOBDDlg::CloseDBConn()
{
	mysql_free_result(res);

	// Close a MySQL connection
	mysql_close(conn);
}

void CSpiceOBDDlg::InitUserDialingList()
{
	static int ColumnWidth[7] = { 200, 200, 200, 150, 150, 200 ,250 };
	LV_COLUMN lvc;
	lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	lvc.iSubItem = 0;
	lvc.pszText = L"C No";
	lvc.cx = ColumnWidth[0];
	m_TrkChList.InsertColumn(0, &lvc);

	lvc.iSubItem = 1;
	lvc.pszText = L"Application";
	lvc.cx = ColumnWidth[1];
	m_TrkChList.InsertColumn(1, &lvc);

	lvc.iSubItem = 2;
	lvc.pszText = L"Msg No";
	lvc.cx = ColumnWidth[2];
	m_TrkChList.InsertColumn(2, &lvc);

	lvc.iSubItem = 3;
	lvc.pszText = L"CLI";
	lvc.cx = ColumnWidth[3];
	m_TrkChList.InsertColumn(3, &lvc);

	lvc.iSubItem = 4;
	lvc.pszText = L"DNIS";
	lvc.cx = ColumnWidth[4];
	m_TrkChList.InsertColumn(4, &lvc);

	lvc.iSubItem = 5;
	lvc.pszText = L"Prompts";
	lvc.cx = ColumnWidth[5];
	m_TrkChList.InsertColumn(5, &lvc);

	lvc.iSubItem = 6;
	lvc.pszText = L"ChState";
	lvc.cx = ColumnWidth[6];
	m_TrkChList.InsertColumn(6, &lvc);

	//Set channels initial status in the grid
	SetChannelInitialStatus();
	//set column size to auto fit list control

	/*SetRedraw(FALSE);
	int nColumnCount = m_TrkChList.GetHeaderCtrl()->GetItemCount();

	for (int i = 0; i < nColumnCount; i++)
	{
	m_TrkChList.SetColumnWidth(i, LVSCW_AUTOSIZE);
	int nColumnWidth = m_TrkChList.GetColumnWidth(i);
	m_TrkChList.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
	int nHeaderWidth = m_TrkChList.GetColumnWidth(i);
	m_TrkChList.SetColumnWidth(i, max(nColumnWidth, nHeaderWidth));
	}
	SetRedraw(TRUE);*/
}

void CSpiceOBDDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSpiceOBDDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSpiceOBDDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int CSpiceOBDDlg::GetAnIdleChannel() // find an idle trunk channel
{
	int i;
	for (i = 0; i < nTotalCh; i++)
	{
		if (!ChInfo[i].InUse && ChInfo[i].EnCalled) break;
	}

	if (i == nTotalCh) return -1;
	return i;
}

void getErrorResult(LPCTSTR  ApiName)
{
	char errStr[256];
	SsmGetLastErrMsg(errStr);
	CString str(errStr);
	str.Append(L"- ");
	str.Append(ApiName);
	AfxMessageBox(str);
	SsmCloseCti();
	PostQuitMessage(0);
}

void CSpiceOBDDlg::DoUserWork()
{
	int i = 0;
	for (int i = 0; i < 1 /*nTotalCh*/; i++)
	{
		if (SsmGetChType(i) != 11) continue;

		switch (ChInfo[i].nStep)
		{
		case USER_IDLE:
			if (SsmGetChState(i) == 0)
			{
				SsmClearRxDtmfBuf(i);
				if (SsmAutoDial(i, ChInfo[i].pPhoNumBuf) == 0) // making call
				{
					ChInfo[i].nStep = USER_WAIT_REMOTE_PICKUP;
				}
				else
				{
					SsmHangup(i);
					ChInfo[i].nStep = USER_WAIT_HANGUP;
				}
			}
			break;
		case USER_WAIT_REMOTE_PICKUP:
			ChInfo[i].lineState = SsmChkAutoDial(i);
			if (ChInfo[i].lineState == DIAL_VOICE)
			{
				ChInfo[i].nStep = USER_TALKING;
				if (SsmPlayIndexString(i, "1") == -1)
				{
					ChInfo[i].nStep = USER_WAIT_HANGUP;
				}
			}
			else if (SsmGetHookState(i) == 0 || (ChInfo[i].lineState == DIAL_NOANSWER) ||
				(ChInfo[i].lineState == DIAL_BUSYTONE) || (ChInfo[i].lineState == DIAL_FAILURE))		//Either user hangup or 30s timeout or any such reason
			{
				SsmHangup(i);
				SsmClearRxDtmfBuf(i);
				ChInfo[i].nStep = USER_IDLE;
			}
			break;

		case USER_TALKING:
			if (SsmCheckPlay(i) == 0)
			{
				logger.log(LOGINFO, "SsmGetDtmfStr called on %s", ChInfo[i].pPhoNumBuf);
				if (SsmGetDtmfStr(i, ChInfo[i].DtmfBuf) >= 0)
				{
					/*if (!StrCmpA(ChInfo[i].DtmfBuf, "1"))
					{*/
					//TODO
					logger.log(LOGINFO, "DTMF recived: %s", ChInfo[i].DtmfBuf);
					SsmStopPlayIndex(i);
					//}
				}
			}
			else
			{
				SsmHangup(i);
				SsmClearRxDtmfBuf(i);
				ChInfo[i].nStep = USER_IDLE;
			}
			break;
		case USER_WAIT_HANGUP:
			if (SsmCheckPlay(i) >= 1 || SsmGetChState(i) == S_CALL_PENDING || SsmGetHookState(i) == 0)		//remote user hung up or user hung up
			{
				SsmHangup(i);
				//SsmStopSendTone(i);					//stop sending busy tone
				SsmClearRxDtmfBuf(i);
				ChInfo[i].nStep = USER_IDLE;
				IsUpdate = true;
			}
			break;
		default:
			ChInfo[i].nStep = USER_IDLE;
			break;
		}//end switch
	}//end for
}



BOOL CSpiceOBDDlg::InitCtiBoard()
{
	//Initialization of CTI driver
	char CurPath[260], ShIndex[260], ShConfig[260];
	GetCurrentDirectoryA(200, CurPath);
	StrCpyA(ShConfig, CurPath);
	StrCpyA(ShIndex, CurPath);
	StrCatA(ShConfig, "\\ShConfig.ini");
	StrCatA(ShIndex, "\\ShIndex.ini");
	if (SsmStartCti(ShConfig, ShIndex) != 0)
	{
		char *  str1 = new char[256];
		SsmGetLastErrMsg(str1);
		CString str2(str1);
		logger.log(LOGFATAL, "CSpiceOBDDlg -> InitCtiBoard: %s", str1);
		AfxMessageBox(str2);
		PostQuitMessage(0);
		return false;
	}
	AfxMessageBox(L"Card Initilized !!");

	//Initialization of channels on trunk-board
	nTotalCh = SsmGetMaxCh();
	ChInfo = new CH_INFO[nTotalCh];
	int totalIdleChannels = 0, callEnabledChannels = 0, setIdleCalled = 0;
	for (int i = 0; i < nTotalCh; i++)
	{
		ChInfo[i].EnCalled = false;
		int chType = SsmGetChType(i);
		if (chType == 11) //ISUP channel(China SS7 signaling ISUP)
		{
			if (SsmGetChState(i) == 0) //check idle
			{
				totalIdleChannels++;
				ChInfo[i].nStep = USER_IDLE;
			}
			else
			{
				//Set idle
				SsmHangup(i);
				setIdleCalled++;
				ChInfo[i].nStep = USER_IDLE;

			}
			int nDirection;
			if (SsmGetAutoCallDirection(i, &nDirection) == 1) //enable auto connection 
			{
				if (nDirection == 1 || nDirection == 2) //enable dial
				{
					callEnabledChannels++;

					ChInfo[i].InUse = 0;
					ChInfo[i].EnCalled = true;
				}
			}
		}
	}
	logger.log(LOGINFO, "totalIdleChannels = %d, callEnabledChannels = %d, setIdleCalled = %d", totalIdleChannels, callEnabledChannels, setIdleCalled);

	//Loading hello.wav file on different positions 
	if (SsmLoadIndexData(1, "a", 7, "Hello.wav", 0, 49997) != 0)
		AfxMessageBox(L"Load Index 1 Error");
	//Please leave word
	if (SsmLoadIndexData(2, "b", 7, "Hello.wav", 49714, 12234) != 0)
		AfxMessageBox(L"Load Index 2 Error");
	//Number
	if (SsmLoadIndexData(3, "c", 7, "Hello.wav", 65272, 2192) != 0)
		AfxMessageBox(L"Load Index 3 Error");
	//channel
	if (SsmLoadIndexData(4, "d", 7, "Hello.wav", 70364, 3960) != 0)
		AfxMessageBox(L"Load Index 4 Error");
	//0
	if (SsmLoadIndexData(5, "e", 7, "Hello.wav", 77860, 3253) != 0)
		AfxMessageBox(L"Load Index 5 Error");
	//1
	if (SsmLoadIndexData(6, "f", 7, "Hello.wav", 83305, 2475) != 0)
		AfxMessageBox(L"Load Index 6 Error");
	//2
	if (SsmLoadIndexData(7, "g", 7, "Hello.wav", 87760, 2475) != 0)
		AfxMessageBox(L"Load Index 7 Error");
	//3
	if (SsmLoadIndexData(8, "h", 7, "Hello.wav", 90447, 4243) != 0)
		AfxMessageBox(L"Load Index 8 Error");
	//message
	if (SsmLoadIndexData(9, "i", 7, "Hello.wav", 97165, 5021) != 0)
		AfxMessageBox(L"Load Index 9 Error");
	//cleared
	if (SsmLoadIndexData(10, "j", 7, "Hello.wav", 104803, 7496) != 0)
		AfxMessageBox(L"Load Index 10 Error");
	//time 10s
	if (SsmLoadIndexData(11, "k", 7, "Hello.wav", 114632, 8388) != 0)
		AfxMessageBox(L"Load Index 11 Error");
	//no message
	if (SsmLoadIndexData(12, "l", 7, "Hello.wav", 125708, 6081) != 0)
		AfxMessageBox(L"Load Index 12 Error");
	//error annoucement
	if (SsmLoadIndexData(13, "m", 7, "Hello.wav", 131931, 4000) != 0)
		AfxMessageBox(L"Load Index 13 Error");
	return true;
}

bool CSpiceOBDDlg::SetCLIOnChannels()
{
	//for (int i = 0; i< 1 /*nTotalCh*/; i++)
	//{
	int i = 0;
	if (ChInfo[i].nStep == USER_IDLE)
	{
		//setting caller ID 
		if (SsmSetTxOriginalCallerID(i, reinterpret_cast<byte*>("1400850667")) == -1) //
		{
			getErrorResult(L" SsmSetTxOriginalCallerID");
			return false;
		}
		GetPrivateProfileStringA("Database", "phoneNumber1", "9718050375", ChInfo[i].pPhoNumBuf, 31, "\\DBSettings.INI");
		//i++;


		//if (SsmSetTxOriginalCallerID(i, reinterpret_cast<byte*>("1401870901")) == -1) //
		//{
		//	getErrorResult(L" SsmSetTxOriginalCallerID");
		//	return false;
		//}
		//GetPrivateProfileStringA("Database", "phoneNumber2", "9743512890", ChInfo[i].pPhoNumBuf, 31, "\\DBSettings.INI");
		//CString number1(ChInfo[0].pPhoNumBuf), number2(ChInfo[1].pPhoNumBuf);
		//number1.Append(number2);
		//AfxMessageBox(number);
		//StrCpyA(ChInfo[i].pPhoNumBuf, "8150973438");
		//break;
		//	}
	}
	return true;
}

void CSpiceOBDDlg::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	DoUserWork();
	UpDateATrunkChListCtrl();

	CDialog::OnTimer(nIDEvent);
}

void CSpiceOBDDlg::OnDestroy()
{
	CDialog::OnDestroy();
	delete[] ChInfo;
	SsmCloseCti();
	//CloseDBConn();
}