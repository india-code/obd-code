
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
	: CDialogEx(IDD_SYNWAY_OBD_DIALOG, pParent)
	, m_SetMinLogLevel(0), aesEncryption("1234567891011121")
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

int CSpiceOBDDlg::OffSet = ROW_COUNT;
int CSpiceOBDDlg::row_count = ROW_COUNT;
int CSpiceOBDDlg::getAndUpdateRowCount = 0;

void CSpiceOBDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_TRK, m_TrkChList);
	DDX_Radio(pDX, IDC_TRACE, m_SetMinLogLevel);
}

BEGIN_MESSAGE_MAP(CSpiceOBDDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_TRACE, &CSpiceOBDDlg::OnBnClickedLogLevel)
	ON_BN_CLICKED(IDC_INFO, &CSpiceOBDDlg::OnBnClickedLogLevel)
	ON_BN_CLICKED(IDC_ERROR, &CSpiceOBDDlg::OnBnClickedLogLevel)
	ON_BN_CLICKED(IDC_FATAL, &CSpiceOBDDlg::OnBnClickedLogLevel)
	ON_BN_CLICKED(IDC_DIALLING_START, &CSpiceOBDDlg::OnBnClickedDiallingStart)
	ON_BN_CLICKED(IDC_DIALLING_STOP, &CSpiceOBDDlg::OnBnClickedDiallingStop)
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
	try {
		logger.SetMinLogLevel(m_SetMinLogLevel); //Set initial minimum log level
		if (!InitCtiBoard())  return false;
		m_TrkChList.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
		CRect rect;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
		int screen_x_size = rect.Width();
		int screen_y_size = rect.Height();

		::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, screen_x_size, screen_y_size, SWP_NOZORDER);

		InitilizeDBConnection();
		if (!InitilizeChannels()) return false;
		InitUserDialingList();
	}
	catch (...)
	{
		char errMsg1[256];
		SsmGetLastErrMsg(errMsg1);
		CString errMsg(errMsg1);
		logger.log(LOGFATAL, "%s", errMsg);
		AfxMessageBox(errMsg);
		PostQuitMessage(0);
	}
	//GetDBData();
	SetTimer(1000, 200, NULL);

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
	char CurPath[260], InitDBSettings[260];
	GetCurrentDirectoryA(200, CurPath);
	StrCpyA(InitDBSettings, CurPath);
	StrCatA(InitDBSettings, "\\DBSettings.INI");
	// Fetching all the details required for DB connection.
	GetPrivateProfileStringA("Database", "host", "localhost", host, 255, InitDBSettings);
	GetPrivateProfileStringA("Database", "DBName", "test_db", DBName, 255, InitDBSettings);
	GetPrivateProfileStringA("Database", "username", "root", username, 255, InitDBSettings);
	GetPrivateProfileStringA("Database", "password", "sdl@1234", password, 255, InitDBSettings);

	port = GetPrivateProfileIntA("Database", "Port", 3306, InitDBSettings);
	nTotalCh = GetPrivateProfileIntA("Database", "TotalChannelsCount", 90, InitDBSettings);
	logger.log(LOGINFO, "host: %s, username: %s, password: %s, dbname: %s, port: %d, TotalChannelsCount: %d", host, username, password, DBName, port, nTotalCh);

	if (mysql_real_connect(conn, host, username, password, DBName, port, NULL, 0) == 0)
	{
		AfxMessageBox(L"Connection failed to Database");
		PostQuitMessage(0);
	}
}

void CSpiceOBDDlg::SetChannelInitialStatus()
{
	for (int i = 0, j = 0; i < nTotalCh; i++)
	{
		LVITEM lvItem;
		int nItem;
		CString chNum;

		chNum.Format(_T("%04d"), i);
		lvItem.mask = LVIF_TEXT;
		lvItem.iItem = i;
		lvItem.iSubItem = 0;
		lvItem.pszText = L"";
		nItem = m_TrkChList.InsertItem(&lvItem);

		m_TrkChList.SetItemText(nItem, 0, chNum);
		/*m_TrkChList.SetItemText(nItem, 1, L"VasApps/SPC_CP_DT_81...");
		m_TrkChList.SetItemText(nItem, 2, L"Alerting");*/
		int tempCampId = ChInfo[i].CampaignID;
		if (tempCampId != -1)
		{
			CString tempStr1(ChInfo[i].pPhoNumBuf), tempStr2(Campaigns.at(tempCampId).CLI), campIdStr;
			campIdStr.Format(_T("%d"), tempCampId);

			m_TrkChList.SetItemText(i, 3, tempStr2);
			m_TrkChList.SetItemText(i, 4, tempStr1);
			m_TrkChList.SetItemText(nItem, 5, campIdStr);
		}
		/*CString chStateNum;
		chStateNum.Format(L"%d", SsmGetChState(i));
		m_TrkChList.SetItemText(nItem, 6, chStateNum);*/
	}
	UpDateATrunkChListCtrl();
}

BOOL CSpiceOBDDlg::GetDBData()
{
	/***  Read Records  ***/
	//call to decrypt values read from DB
	int query_state;
	char queryStr[256];
	
	StrCpyA(queryStr, "select campaign_id, cli, port_number, prompts_directory, obd_type from tbl_campaign_master where campaign_status = 1 and base_status = 1 and prompts_status = 1");

	//logger.log(LOGINFO, queryStr);

	query_state = mysql_query(conn, queryStr);

	if (query_state != 0)
	{
		CString err(mysql_error(conn));
		AfxMessageBox(err);
	}

	res = mysql_store_result(conn);

	CampaignData tempdata;
	int i = 1, channelsOccupied = -1;
	Campaigns.clear();
	while ((row = mysql_fetch_row(res)) != NULL)
	{
		StrCpyA(tempdata.campaign_id, row[0]);
		StrCpyA(tempdata.CLI, row[1]);
		tempdata.channelsAllocated = atoi(row[2]);
		StrCpyA(tempdata.promptsPath, row[3]);
		tempdata.obdDialPlan = (OBD_DIAL_PLAN)atoi(row[4]);

		sprintf_s(queryStr, "select ani from tbl_outdialer_base where campaign_id = '%s' and status = %d limit %d", row[0], 0, (5 * tempdata.channelsAllocated));

		//logger.log(LOGINFO, queryStr);

		query_state = mysql_query(conn, queryStr);

		if (query_state != 0)
		{
			CString err(mysql_error(conn));
			AfxMessageBox(err);
		}

		MYSQL_RES * resPhBuf = mysql_store_result(conn);
		MYSQL_ROW rowPhBuf;
		while ((rowPhBuf = mysql_fetch_row(resPhBuf)) != NULL)
		{
			char* DecryptedVal = aesEncryption.DecodeAndDecrypt(rowPhBuf[0]);
			tempdata.phnumBuf.push_back({ DecryptedVal, rowPhBuf[0] });
		}
		tempdata.minCh = channelsOccupied + 1;
		channelsOccupied = tempdata.minCh + tempdata.channelsAllocated - 1;
		tempdata.maxCh = channelsOccupied;

		Campaigns.insert(pair<int, CampaignData>(i, tempdata));
		i++;
		tempdata.phnumBuf.clear();
	}
	return true;
}

BOOL CSpiceOBDDlg::UpdateDBData(/*int chState*/)
{
	try {
		int query_state, chState = 1;
		char queryStr1[131071], queryStr2[131071];
		StrCpyA(queryStr1, "update tbl_outdialer_base set status =  ( case ");
		int countNumbers = 0;
		int lastIndex;
		for (int i = 0; i < nTotalCh; i++)
		{
			if (ChInfo[i].rowTobeUpdated)
			{
				char tempStr[1024];

				sprintf_s(tempStr, "when ani = '%s' then %d ", ChInfo[i].pPhoNumBuf, chState/*ChInfo[i].nStep*/); //TODO: comented section will be used
				StrCatA(queryStr1, tempStr);
				countNumbers++;
				lastIndex = i;
			}
		}
		StrCpyA(queryStr2, "end) where ani in(");
		int i = 0;
		for (i = 0; i < nTotalCh; i++)
		{
			if (ChInfo[i].rowTobeUpdated == true && countNumbers > 1)
			{
				char tempStr[1024];
				sprintf_s(tempStr, "'%s', ", ChInfo[i].pPhoNumBuf);
				StrCatA(queryStr2, tempStr);
				countNumbers--;
			}
		}

		//For the last one no comma required

		char tempStr[48];
		sprintf_s(tempStr, "'%s'", ChInfo[lastIndex].pPhoNumBuf);
		StrCatA(queryStr2, tempStr);



		StrCatA(queryStr2, ")");
		StrCatA(queryStr1, queryStr2);
		query_state = mysql_query(conn, queryStr1);

		logger.log(LOGINFO, queryStr1);

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
	for (i = 0, nIndex = 0; i < nTotalCh; i++)
	{
		//if (SsmGetChType(i) != 2) continue;
		switch (SsmGetChState(i))
		{
		case S_CALL_STANDBY:				state = "S_CALL_STANDBY"; break;
		case S_CALL_PICKUPED:				state = "S_CALL_PICKUPED"; break;
		case S_CALL_RINGING:				state = "S_CALL_RINGING"; break;
		case S_CALL_TALKING:				state = "S_CALL_TALKING"; break;
		case S_CALL_ANALOG_WAITDIALTONE:	state = "S_CALL_ANALOG_WAITDIALTONE";	break;
		case S_CALL_ANALOG_TXPHONUM:		state = "S_CALL_ANALOG_TXPHONUM"; break;
		case S_CALL_ANALOG_WAITDIALRESULT:	state = "S_CALL_ANALOG_WAITDIALRESULT";		break;
		case S_CALL_PENDING:				state = "S_CALL_ANALOG_WAITDIALRESULT"; break;
		case S_CALL_OFFLINE:				state = "S_CALL_OFFLINE"; break;
		case S_CALL_WAIT_REMOTE_PICKUP:		state = "S_CALL_WAIT_REMOTE_PICKUP"; break;
		case S_CALL_ANALOG_CLEAR:			state = "S_CALL_ANALOG_CLEAR"; break;
		case S_CALL_UNAVAILABLE:			state = "S_CALL_UNAVAILABLE"; break;
		case S_CALL_LOCKED:					state = "S_CALL_LOCKED"; break;
		case S_CALL_RemoteBlock:			state = "S_CALL_RemoteBlock"; break;
		case S_CALL_LocalBlock:				state = "S_CALL_LocalBlock"; break;
		case S_CALL_Ss1InWaitPhoNum:		state = "S_CALL_Ss1InWaitPhoNum"; break;
		case S_CALL_Ss1InWaitFwdStop:		state = "S_CALL_Ss1InWaitFwdStop"; break;
		case S_CALL_Ss1InWaitCallerID:		state = "S_CALL_Ss1InWaitCallerID"; break;
		case S_CALL_Ss1InWaitKD:			state = "S_CALL_Ss1InWaitKD"; break;
		case S_CALL_Ss1InWaitKDStop:		state = "S_CALL_Ss1InWaitKDStop"; break;
		case S_CALL_SS1_SAYIDLE:			state = "S_CALL_SS1_SAYIDLE"; break;
		case S_CALL_SS1WaitIdleCAS:			state = "S_CALL_SS1WaitIdleCAS"; break;
		case S_CALL_SS1PhoNumHoldup:		state = "S_CALL_SS1PhoNumHoldup"; break;
		case S_CALL_Ss1InWaitStopSendA3p:	state = "S_CALL_Ss1InWaitPhoNum"; break;
		case S_CALL_Ss1OutWaitBwdAck:		state = "S_CALL_Ss1OutWaitBwdAck"; break;
		case S_CALL_Ss1OutTxPhoNum:			state = "S_CALL_Ss1OutTxPhoNum"; break;
		case S_CALL_Ss1OutWaitAppendPhoNum:	state = "S_CALL_Ss1OutWaitAppendPhoNum"; break;
		case S_CALL_Ss1OutTxCallerID:		state = "S_CALL_Ss1OutTxCallerID"; break;
		case S_CALL_Ss1OutWaitKB:			state = "S_CALL_Ss1OutWaitKB"; break;
		case S_CALL_Ss1OutDetectA3p:		state = "S_CALL_Ss1OutDetectA3p"; break;
		case S_ISDN_OUT_WAIT_NET_RESPONSE:	state = "S_ISDN_OUT_WAIT_NET_RESPONSE"; break;
		case S_ISDN_OUT_PLS_APPEND_NO:		state = "S_ISDN_OUT_PLS_APPEND_NO"; break;
		case S_ISDN_IN_CHK_CALL_IN:			state = "S_ISDN_IN_CHK_CALL_IN"; break;
		case S_ISDN_IN_RCVING_NO:			state = "S_ISDN_IN_RCVING_NO"; break;
		case S_ISDN_IN_WAIT_TALK:			state = "S_ISDN_IN_WAIT_TALK"; break;
		case S_ISDN_OUT_WAIT_ALERT:			state = "S_ISDN_OUT_WAIT_ALERT"; break;
		case S_ISDN_CALL_BEGIN:				state = "S_ISDN_CALL_BEGIN"; break;
		case S_ISDN_WAIT_HUANGUP:			state = "S_ISDN_WAIT_HUANGUP"; break;
		case S_ISDN_IN_CALL_PROCEEDING:		state = "S_ISDN_IN_CALL_PROCEEDING"; break;
		case S_CALL_SENDRING:				state = "S_CALL_SENDRING "; break;
		case S_ISUP_WaitSAM:				state = "S_ISUP_WaitSAM"; break;
		case S_ISUP_WaitRLC:				state = "S_ISUP_WaitRLC"; break;
		case S_ISUP_WaitReset:				state = "S_ISUP_WaitReset"; break;
		case S_ISUP_LocallyBlocked:			state = "S_ISUP_LocallyBlocked"; break;
		case S_ISUP_RemotelyBlocked:        state = "S_ISUP_RemotelyBlocked"; break;
		case S_ISUP_WaitDialAnswer:			state = "S_ISUP_WaitDialAnswer"; break;
		case S_ISUP_WaitINF:				state = "S_ISUP_WaitINF"; break;
		case S_ISUP_WaitSetCallerID:		state = "S_ISUP_WaitSetCallerID"; break;
		case S_DTRC_ACTIVE:					state = "S_DTRC_ACTIVE"; break;
		case S_ISUP_Suspend:				state = "S_ISUP_Suspend"; break;
		case S_CALL_DISCONECT:				state = "S_CALL_DISCONECT"; break;
		case S_CALL_SS1WaitFlashEnd:		state = "S_CALL_SS1WaitFlashEnd"; break;
		case S_CALL_FlashEnd:				state = "S_CALL_FlashEnd"; break;
		case S_CALL_SIGNAL_ERROR:			state = "S_CALL_SIGNAL_ERROR"; break;
		case S_CALL_FRAME_ERROR:			state = "S_CALL_FRAME_ERROR"; break;
		case S_IP_MEDIA_LOCK:				state = "S_IP_MEDIA_LOCK"; break;
		case S_IP_MEDIA_OPEN:				state = "S_IP_MEDIA_OPEN"; break;
		case S_SPY_RBSWAITACK:				state = "S_SPY_RBSWAITACK"; break;
		case S_SPY_RBSSENDACK:				state = "S_SPY_RBSSENDACK"; break;
		case S_IPR_USING:					state = "S_IPR_USING"; break;
		case S_IPR_COMMUNICATING:			state = "S_IPR_COMMUNICATING"; break;
		case S_ISUP_WaitCOT:				state = "S_ISUP_WaitCOT"; break;

		}

		m_TrkChList.GetItemText(nIndex, 6, tmpstr, 50);
		if (state != tmpstr)
			m_TrkChList.SetItemText(nIndex, 6, state.GetBuffer(50));

		/*m_TrkChList.GetItemText(nIndex, 4, tmpstr, 50);
		if (ChInfo[i].pPhoNumBuf != tmpstr)
		m_TrkChList.SetItemText(nIndex, 4, ChInfo[i].pPhoNumBuf);*/

		nIndex++;
	}
	if (IsUpdate)
	{
		UpdateStatusAndPickNextRecords();
	}
	char queryStr[256];
	int query_state;
	//Loop to pick next records for all campaigns.
	for (size_t tmpCmpId = 1; tmpCmpId <= Campaigns.size(); tmpCmpId++)
	{
		//int tmpCmpId = ChInfo[i].CampaignID;
		//if (tmpCmpId != -1)
		//{
		if ((Campaigns.at(tmpCmpId).phnumBuf.size() <= (2 * Campaigns.at(tmpCmpId).channelsAllocated))/* && !Campaigns.at(tmpCmpId).hasReachedThreshold*/)
		{
			sprintf_s(queryStr, "select ani from tbl_outdialer_base where campaign_id = '%s' and status = %d limit %d",
				Campaigns.at(tmpCmpId).campaign_id, 0, (5 * Campaigns.at(tmpCmpId).channelsAllocated));

			query_state = mysql_query(conn, queryStr);

			if (query_state != 0)
			{
				CString err(mysql_error(conn));
				AfxMessageBox(err);
			}
			MYSQL_RES * resPhBuf = mysql_store_result(conn);
			MYSQL_ROW rowPhBuf;
			Campaigns.at(tmpCmpId).phnumBuf.clear();
			while ((rowPhBuf = mysql_fetch_row(resPhBuf)) != NULL)
			{
				char* DecryptedVal = aesEncryption.DecodeAndDecrypt(rowPhBuf[0]);
				Campaigns.at(tmpCmpId).phnumBuf.push_back({ DecryptedVal, rowPhBuf[0] });
			}
			logger.log(LOGINFO, "vector size loaded from DB : %d for campaign: %d", Campaigns.at(tmpCmpId).phnumBuf.size(), tmpCmpId);
			/*if (Campaigns.at(tmpCmpId).phnumBuf.size() <= (2 * Campaigns.at(tmpCmpId).channelsAllocated))
			{
			Campaigns.at(tmpCmpId).hasReachedThreshold = true;
			}*/
		}

		/*if (!Campaigns.at(tmpCmpId).phnumBuf.empty())
		{
		CString tempStr2(Campaigns.at(tmpCmpId).phnumBuf.front().ani);
		StrCpyA(ChInfo[i].pPhoNumBuf, Campaigns.at(tmpCmpId).phnumBuf.front().ani);
		StrCpyA(ChInfo[i].CDRStatus.encrypted_ani, Campaigns.at(tmpCmpId).phnumBuf.front().encryptedAni);
		m_TrkChList.SetItemText(i, 4, tempStr2);
		Campaigns.at(tmpCmpId).phnumBuf.erase(Campaigns.at(tmpCmpId).phnumBuf.begin());
		logger.log(LOGINFO, "UpdateStatusAndPickNextRecords Update data Ani : %s, Encrypted Ani: %s, Channel Num: %d", ChInfo[i].pPhoNumBuf, ChInfo[i].CDRStatus.encrypted_ani, i);
		//logger.log(LOGINFO, "UpdateStatusAndPickNextRecords vector current size: %d for campaign Id: %d", Campaigns.at(tmpCmpId).phnumBuf.size(), tmpCmpId);
		if (!UpdatePhNumbersStatus(i))
		{
		logger.log(LOGINFO,"UpdateStatusAndPickNextRecords Row not updated...ph number: %s, channel number: %d", ChInfo[i].pPhoNumBuf, i);
		}
		}
		else
		{
		StrCpyA(ChInfo[i].pPhoNumBuf, "");
		StrCpyA(ChInfo[i].CDRStatus.encrypted_ani, "");
		m_TrkChList.SetItemText(i, 4, L"");
		}*/
		//}
	}

}

void CSpiceOBDDlg::UpdateStatusAndPickNextRecords()
{
	int query_state;
//	char queryStr[256];
	char queryStrInsert[1024];
	for (int i = 0; i < nTotalCh; i++)
	{
		if (ChInfo[i].EnCalled == true)
		{
			ChInfo[i].EnCalled = false;
			//Log CDR
			ChInfo[i].CDRStatus.answer_duration = (ChInfo[i].CDRStatus.answer_time != 0) ? ChInfo[i].CDRStatus.end_time - ChInfo[i].CDRStatus.answer_time : 0;
			ChInfo[i].CDRStatus.callPatch_duration = (ChInfo[i].CDRStatus.answer_time != 0) ? (ChInfo[i].CDRStatus.answer_time - ChInfo[i].CDRStatus.call_time) : (ChInfo[i].CDRStatus.end_time - ChInfo[i].CDRStatus.call_time);
			ChInfo[i].CDRStatus.total_duration = ChInfo[i].CDRStatus.callPatch_duration + ChInfo[i].CDRStatus.answer_duration;
			ChInfo[i].CDRStatus.channel = i;
			StrCpyA(ChInfo[i].CDRStatus.ani, ChInfo[i].pPhoNumBuf);
			char dateVal[25], timeVal[15], call_time[20], answer_time[20], end_time[20];
			tm ct, at, et;
			tm dateTime = logger.getTime(dateVal);
			sprintf_s(dateVal, "%04d%02d%02d", dateTime.tm_year + 1900, dateTime.tm_mon + 1, dateTime.tm_mday);
			sprintf_s(timeVal, "%02d%02d%02d", dateTime.tm_hour, dateTime.tm_min, dateTime.tm_sec);

			localtime_s(&ct, &ChInfo[i].CDRStatus.call_time);
			if (strftime(call_time, sizeof(call_time), "%X", &ct) == 0)
			{
				StrCpyA(call_time, "");
			}

			if (ChInfo[i].CDRStatus.answer_time != 0) {
				localtime_s(&at, &ChInfo[i].CDRStatus.answer_time);
				if (strftime(answer_time, sizeof(answer_time), "%X", &at) == 0)
				{
					StrCpyA(answer_time, "");
				}
			}
			else
			{
				StrCpyA(answer_time, "");
			}

			localtime_s(&et, &ChInfo[i].CDRStatus.end_time);
			if (strftime(end_time, sizeof(end_time), "%X", &et) == 0)
			{
				StrCpyA(end_time, "");
			}


			int tmpCmpId = ChInfo[i].CampaignID;

			outfile << "IDEA_HR" << "#" << systemIpAddr << "#" << dateVal << "#" << timeVal << "#" << Campaigns.at(tmpCmpId).CLI << "#" << ChInfo[i].CDRStatus.ani
				<< "#" << ChInfo[i].CDRStatus.encrypted_ani << "#" << call_time << "#" << answer_time << "#" << end_time << "#" << ChInfo[i].CDRStatus.callPatch_duration
				<< "#" << ChInfo[i].CDRStatus.answer_duration << "#" << ChInfo[i].CDRStatus.total_duration << "#" << ChInfo[i].CDRStatus.status << "#"
				<< ChInfo[i].CDRStatus.reason << "#" << ChInfo[i].CDRStatus.reason_code << "#" << ChInfo[i].CDRStatus.dtmf << "#" << ChInfo[i].CDRStatus.dtmf2 << "#"
				<< ChInfo[i].CDRStatus.channel << "#" << Campaigns.at(tmpCmpId).campaign_id << "#" << endl;

			//Check if CDR file is greater than 10 MB make a new one.
		/*	if (outfile.tellp() >= 100)
			{
				outfile.close();
				openCDRLogFile();
			}*/
			//Insert Call records in DB
			sprintf_s(queryStrInsert, "INSERT INTO tbl_obd_calls(channel, campaign_id, circle, ani, cli, dtmf, answer_duration, status, ring_duration, call_date, call_time, answer_time, end_time, reason_code,total_duration,reason,encrypted_ani, call_id) \
				VALUES(%d, '%s', 'Idea HR', '%s', '%s', '%s#%s', %d, %d, %d, '%s', '%s', '%s', '%s', '%s', '%d', '%s','%s', '%s%s')",
				ChInfo[i].CDRStatus.channel, Campaigns.at(tmpCmpId).campaign_id, ChInfo[i].CDRStatus.ani, Campaigns.at(tmpCmpId).CLI, ChInfo[i].CDRStatus.dtmf, ChInfo[i].CDRStatus.dtmf2, ChInfo[i].CDRStatus.answer_duration,
				StrCmpA(ChInfo[i].CDRStatus.status, "SUCCESS") == 0 ? 1 : 0, ChInfo[i].CDRStatus.callPatch_duration, dateVal, call_time, answer_time, end_time, ChInfo[i].CDRStatus.reason_code,
				ChInfo[i].CDRStatus.total_duration, ChInfo[i].CDRStatus.reason, ChInfo[i].CDRStatus.encrypted_ani, ChInfo[i].CDRStatus.ani, timeVal);

			logger.log(LOGINFO, queryStrInsert);
			query_state = mysql_query(conn, queryStrInsert);
			if (query_state != 0)
			{
				CString err(mysql_error(conn));
				AfxMessageBox(err);
			}
			//logging correct consent recieved
			if (StrCmpA(ChInfo[i].CDRStatus.firstConsent, "") && StrCmpA(ChInfo[i].CDRStatus.secondConsent, ""))
			{
				ConsentFile << "IDEA_HR" << "#" << systemIpAddr << "#" << dateVal << "#" << timeVal << "#" << Campaigns.at(tmpCmpId).CLI << "#" << ChInfo[i].pPhoNumBuf
					<< "#" << ChInfo[i].CDRStatus.firstConsent << "#" << ChInfo[i].CDRStatus.secondConsent << "#" << Campaigns.at(tmpCmpId).campaign_id << "#" << endl;
			}
			if (Campaigns.at(tmpCmpId).obdDialPlan == AcquisitionalOBDWith1stConsent && StrCmpA(ChInfo[i].CDRStatus.firstConsent, ""))
			{
				ConsentFile << "IDEA_HR" << "#" << systemIpAddr << "#" << dateVal << "#" << timeVal << "#" << Campaigns.at(tmpCmpId).CLI << "#" << ChInfo[i].pPhoNumBuf
					<< "#" << ChInfo[i].CDRStatus.firstConsent <<"#" << Campaigns.at(tmpCmpId).campaign_id << "#" << endl;
			}
			StrCpyA(ChInfo[i].CDRStatus.dtmf, "");
			StrCpyA(ChInfo[i].CDRStatus.dtmf2, "");
			StrCpyA(ChInfo[i].CDRStatus.firstConsent, "");
			StrCpyA(ChInfo[i].CDRStatus.secondConsent, "");
			StrCpyA(ChInfo[i].CDRStatus.reason, "");
			StrCpyA(ChInfo[i].CDRStatus.status, "");
			StrCpyA(ChInfo[i].CDRStatus.reason_code, "");
		}
	}//For loop
}

//Update record in table...
BOOL CSpiceOBDDlg::UpdatePhNumbersStatus(int ch)
{
	char queryStr[256];

	sprintf_s(queryStr, "update tbl_outdialer_base set status = 1 where campaign_id = '%s' and ani = '%s'",
		Campaigns.at(ChInfo[ch].CampaignID).campaign_id, ChInfo[ch].CDRStatus.encrypted_ani);
	//logger.log(LOGINFO, "update query for phone number: %s, Encrypted Ani: %s, Channel Num:%d", ChInfo[ch].pPhoNumBuf, ChInfo[ch].CDRStatus.encrypted_ani, ch);
	int query_state = mysql_query(conn, queryStr);

	if (query_state != 0)
	{
		CString err(mysql_error(conn));
		AfxMessageBox(err);
	}
	if (mysql_affected_rows(conn) >= 1)
	{
		return true;
	}
	logger.log(LOGERR, "UpdatePhNumbersStatus Number update failed: %s, query string: %s, channel Num: %d", ChInfo[ch].pPhoNumBuf, queryStr, ch);
	return false;
}
void CSpiceOBDDlg::CloseDBConn()
{
	mysql_free_result(res);

	// Close a MySQL connection
	mysql_close(conn);
}



void CSpiceOBDDlg::InitUserDialingList()
{
	static int ColumnWidth[7] = { 50, 50, 50, 200, 200, 50 ,750 }; //earlier it was: { 200, 200, 200, 150, 150, 200 ,250 }
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

void CSpiceOBDDlg::LogErrorCodeAndMessage(int ch)
{
	sprintf_s(ChInfo[ch].CDRStatus.reason_code, "%d", SsmGetLastErrCode()); //get error code
	SsmGetLastErrMsg(ChInfo[ch].CDRStatus.reason); //get dial failure error message
	logger.log(LOGERR, "Error code: %s And Error Reason: %s on channel Number: %d , phone number: %s", ChInfo[ch].CDRStatus.reason_code, ChInfo[ch].CDRStatus.reason, ch, ChInfo[ch].pPhoNumBuf);
}

void CSpiceOBDDlg::HangupCall(int ch)
{
	SsmClearRxDtmfBuf(ch);
	SsmStopPlayIndex(ch);
	SsmHangup(ch);
	ChInfo[ch].EnCalled = true;
	IsUpdate = true;
	ChInfo[ch].nStep = USER_IDLE;
	if (StrCmpA(ChInfo[ch].CDRStatus.status, "") == 0) StrCpyA(ChInfo[ch].CDRStatus.status, "FAIL");
	if (StrCmpA(ChInfo[ch].CDRStatus.reason, "") == 0) { char errReason[100]; SsmGetLastErrMsg(errReason); StrCpyA(ChInfo[ch].CDRStatus.reason, errReason); }
	if (StrCmpA(ChInfo[ch].CDRStatus.reason_code, "") == 0) { char errorcode[8]; sprintf_s(errorcode, "%d", SsmGetLastErrCode()); StrCpyA(ChInfo[ch].CDRStatus.reason_code, errorcode); }
}

char* CSpiceOBDDlg::GetReleaseErrorReason(WORD errorCode)
{
	switch (errorCode)
	{
	case 0: return "answered"; break;
	case 1: return "Unallocated Number"; break;
	case 2: return "No Route To Specific Transit Network"; break;
	case 3: return "No Route To Destination"; break;
	case 4: return "Send Special Information Tone"; break;
	case 5: return "Misdialled trunk Prefix"; break;
	case 6: return "Channel Unacceptable"; break;
	case 7: return "Call Awarded nd Deliever In a Established Chanl"; break;
	case 8: return "Preemption"; break;
	case 9: return "Preemption Circuit Reserved For Reuse"; break;
	case 16: return "Normal Call Clearing"; break;
	case 17: return "User Busy"; break;
	case 18: return "No User Responding"; break;
	case 19: return "No Answer"; break;
	case 20: return "Switch Off"; break;
	case 21: return "Call Rejected"; break;
	case 22: return "Number Changed"; break;
	case 25: return "Routing Error"; break;
	case 26: return "Non-Selected User Clearing"; break;
	case 27: return "Destination out of order"; break;
	case 28: return "Invalid number format"; break;
	case 29: return "Facility Rejected"; break;
	case 30: return "Response To Status Enquiry"; break;
	case 31: return "Network issue"; break;
	case 34: return "No Circuit Available"; break;
	case 38: return "Network out of order"; break;
	case 39: return "Permanent Frame Mode Connection Out Of Service"; break;
	case 40: return "Permanent Frame Mode Connection Operational"; break;
	case 41: return "Temporary failure"; break;
	case 42: return "Congestion"; break;
	case 43: return "Access Information Discarded"; break;
	case 44: return "Requested Channel Not Available"; break;
	case 46: return "Precedence Call Blocked"; break;
	case 47: return "Resource unavailable"; break;
	case 49: return "Quality Of Service Unavailable"; break;
	case 50: return "Requested Facility Not Subscribed"; break;
	case 53: return "Outgoing Calls barred"; break;
	case 55: return "Incoming calls barred"; break;
	case 57: return "Bearer Capabilty Not Authorized"; break;
	case 58: return "Bearer Capabilty Not Available"; break;
	case 62: return "Inconsistency  Designated Outgoing Access Info"; break;
	case 63: return "Service not available"; break;
	case 65: return "Bearer Capability Not Implemented"; break;
	case 66: return "Channel Type Not Implemented"; break;
	case 69: return "Requested Facility Not Implemented"; break;
	case 70: return "Only Restricted Digital Information"; break;
	case 79: return "Service Not Implemented "; break;
	case 81: return "Invalid Call Reference Value"; break;
	case 82: return "Identified Channel Not Exits"; break;
	case 83: return "Suspended Call Exists,But This Call Identity Do't"; break;
	case 84: return "Call Identity In Use"; break;
	case 85: return "No Call Suspended"; break;
	case 86: return "Call Having Requested Call Identity Has Cleared "; break;
	case 87: return "User Not Member of CUG"; break;
	case 88: return "Incompatible destination"; break;
	case 90: return "Non Existent CUG"; break;
	case 91: return "Invalid Transit Network Selection"; break;
	case 95: return "Invalid message"; break;
	case 96: return "Mandatory Information Element Missing"; break;
	case 97: return "Message Type Not Implemented"; break;
	case 98: return "Message Not Compatible With Call State"; break;
	case 99: return "Information Parameter Not Implemented"; break;
	case 100: return "Invalid Information Element"; break;
	case 101: return "Message Not Compatible With Call State"; break;
	case 102: return "Time expires"; break;
	case 103: return "Parameter Not Implemented"; break;
	case 110: return "Message With Unrecognized Parameter"; break;
	case 111: return "Protocol error                "; break;
	case 127: return "Interworking"; break;
	default:return "Interworking";	break;
	}
}

void CSpiceOBDDlg::DoUserWork()
{
	int nResult, nDirection;
	CString tempPhoneNum;
	char CampID[7];
	int tempCampId;
	IsUpdate = false;
	WORD releaseCode;

	for (int i = 0; i < nTotalCh; i++)
	{
		if (SsmGetChType(i) != 11) continue;
		nResult = SsmGetAutoCallDirection(i, &nDirection);

		if (nResult == -1 || nDirection < 1) continue;
		tempCampId = ChInfo[i].CampaignID;
		if (tempCampId == -1) continue;

		switch (ChInfo[i].nStep)
		{
		case USER_IDLE:
			ChInfo[i].lineState = SsmGetChState(i);
			if (ChInfo[i].lineState == S_CALL_STANDBY && ChInfo[i].EnCalled == false)
			{
				//Copy phone Numbers to auto dial
				if (!Campaigns.at(tempCampId).phnumBuf.empty() && IsStartDialling)
				{
					CString tempStr2(Campaigns.at(tempCampId).phnumBuf.front().ani);
					StrCpyA(ChInfo[i].pPhoNumBuf, Campaigns.at(tempCampId).phnumBuf.front().ani);
					StrCpyA(ChInfo[i].CDRStatus.encrypted_ani, Campaigns.at(tempCampId).phnumBuf.front().encryptedAni);
					m_TrkChList.SetItemText(i, 4, tempStr2);
					delete Campaigns.at(tempCampId).phnumBuf.front().ani;
					Campaigns.at(tempCampId).phnumBuf.erase(Campaigns.at(tempCampId).phnumBuf.begin());
					logger.log(LOGINFO, "DoUserWork Update data Ani : %s, Encrypted Ani: %s, Channel Num: %d", ChInfo[i].pPhoNumBuf, ChInfo[i].CDRStatus.encrypted_ani, i);
					//logger.log(LOGINFO, "UpdateStatusAndPickNextRecords vector current size: %d for campaign Id: %d", Campaigns.at(tmpCmpId).phnumBuf.size(), tmpCmpId);
					if (!UpdatePhNumbersStatus(i))
					{
						logger.log(LOGINFO, "DoUserWork Row not updated...ph number: %s, channel number: %d", ChInfo[i].pPhoNumBuf, i);
					}
				}
				else
				{
					StrCpyA(ChInfo[i].pPhoNumBuf, "");
					StrCpyA(ChInfo[i].CDRStatus.encrypted_ani, "");
					m_TrkChList.SetItemText(i, 4, L"");
				}
				if (StrCmpA(ChInfo[i].pPhoNumBuf, "") == 0) continue;
				//SsmClearRxDtmfBuf(i);
				if (SsmAutoDial(i, ChInfo[i].pPhoNumBuf) == 0) // making call
				{
					ChInfo[i].nStep = USER_WAIT_REMOTE_PICKUP;
					ChInfo[i].CDRStatus.call_time = time(0);
					ChInfo[i].CDRStatus.answer_time = 0;
					//logger.log(LOGINFO, "Number Dialed on channel: %d ", i);
				}
				else
				{
					LogErrorCodeAndMessage(i);
					HangupCall(i);
				}
				logger.log(LOGINFO, "DoUserWork Number dialed: %s Encrypted ani: %s , channel Num: %d", ChInfo[i].pPhoNumBuf, ChInfo[i].CDRStatus.encrypted_ani, i);
			}
			if (ChInfo[i].lineState == S_CALL_PENDING)
			{
				SsmStopPlayIndex(i);
				SsmClearRxDtmfBuf(i);
				SsmHangup(i);
				ChInfo[i].nStep = USER_IDLE;
			}
			break;
		case USER_WAIT_REMOTE_PICKUP:
			/*logger.log(LOGINFO, "Phone Number: %s, AutoDial State:%d Channel Number: %d, channel state: %d, Release reason: %hu , autodialFail Reason : %d, Pending reason: %d",
			ChInfo[i].pPhoNumBuf, SsmChkAutoDial(i), i, SsmGetChState(i), SsmGetReleaseReason(i), SsmGetAutoDialFailureReason(i), SsmGetPendingReason(i));*/
			if (SsmGetChState(i) == S_CALL_RINGING)
			{
				StrCpyA(ChInfo[i].CDRStatus.status, "FAIL");
				StrCpyA(ChInfo[i].CDRStatus.reason, "Missed");
				StrCpyA(ChInfo[i].CDRStatus.reason_code, "2");
			}
			switch (SsmChkAutoDial(i))
			{
			case DIAL_VOICE:
			case DIAL_VOICEF1:
			case DIAL_VOICEF2:
				sprintf_s(CampID, "%d", Campaigns.at(tempCampId).loadedIndex[1]);
				//logger.log(LOGINFO, "Camp Id: %s", CampID);
				StrCpyA(ChInfo[i].CDRStatus.status, "SUCCESS");
				StrCpyA(ChInfo[i].CDRStatus.reason, "Answered");
				//StrCpyA(ChInfo[i].CDRStatus.reason_code, "7");
				ChInfo[i].CDRStatus.answer_time = time(0);
				if (SsmPlayIndexString(i, CampID) == -1)
				{
					LogErrorCodeAndMessage(i);
					HangupCall(i);
				}
				else
				{
					//logger.log(LOGINFO, "Call picked up channel: %d, phone number: %s", i, ChInfo[i].pPhoNumBuf);
					ChInfo[i].nStep = USER_TALKING;
					ChInfo[i].DialPlanStatus = Campaigns.at(tempCampId).obdDialPlan;
					if (ChInfo[i].DialPlanStatus != Informative)
					{
						ChInfo[i].ConsentState = 1;
						SsmSetWaitDtmfExA(i, 18000, 1, "0123456789*#", true); //set the DTMF termination character
					}
				}
				break;
			case DIAL_FAILURE:
			case DIAL_NO_DIALTONE:
			case DIAL_ECHO_NOVOICE:
			case DIAL_INVALID_PHONUM:
			case DIAL_BUSYTONE:
			case DIAL_NOVOICE:
			case DIAL_NOANSWER:
				ChInfo[i].CDRStatus.end_time = time(0);
				releaseCode = SsmGetReleaseReason(i);
				StrCpyA(ChInfo[i].CDRStatus.status, "FAIL");
				sprintf_s(ChInfo[i].CDRStatus.reason_code, "%hu", releaseCode);
				StrCpyA(ChInfo[i].CDRStatus.reason, GetReleaseErrorReason(releaseCode));
				HangupCall(i);
				break;
			case DIAL_STANDBY:
				nResult = SsmGetChStateKeepTime(i);

				if (nResult > 2000)
				{
					ChInfo[i].CDRStatus.end_time = time(0);
					StrCpyA(ChInfo[i].CDRStatus.reason, "DIAL_STANDBY");
					StrCpyA(ChInfo[i].CDRStatus.status, "FAIL");
					sprintf_s(ChInfo[i].CDRStatus.reason_code, "%hu", SsmGetReleaseReason(i));
					//StrCpyA(ChInfo[i].CDRStatus.reason_code, "0");
					//LogErrorCodeAndMessage(i);
					//logger.log(LOGERR, "Channel State: %d at channel Number: %d", DIAL_STANDBY, i);
					HangupCall(i);
				}

				break;

			case DIAL_DIALING:
			case DIAL_ECHOTONE:
			default:
				break;
			}
			if (SsmGetChState(i) == S_CALL_PENDING)
			{

				//logger.log(LOGERR, "channel %d State: %d, on phone number: %s", i, SsmGetChState, ChInfo[i].pPhoNumBuf);
				ChInfo[i].CDRStatus.end_time = time(0);
				releaseCode = SsmGetReleaseReason(i);
				sprintf_s(ChInfo[i].CDRStatus.reason_code, "%hu", releaseCode);
				StrCpyA(ChInfo[i].CDRStatus.status, "FAIL");
				StrCpyA(ChInfo[i].CDRStatus.reason, GetReleaseErrorReason(releaseCode));
				HangupCall(i);
			}
			break;
		case USER_TALKING:
			switch (ChInfo[i].DialPlanStatus)
			{
			case AcquisitionalOBDWith1stAnd2ndConsent:
				switch (ChInfo[i].ConsentState) {
				case 1:
					ChInfo[i].mediaState = SsmCheckPlay(i);
					//logger.log(LOGINFO, "Media state: %d, ph Number: %s", ChInfo[i].mediaState, ChInfo[i].pPhoNumBuf);
					if (ChInfo[i].mediaState >= 0)
					{
						//StrCpyA(ChInfo[i].CDRStatus.dtmf, SsmGetDtmfStrA(i));
						if (SsmChkWaitDtmf(i, ChInfo[i].CDRStatus.dtmf) >= 1)
						{
							if (StrCmpA(ChInfo[i].CDRStatus.dtmf, "1") == 0) //Right Input
							{
								SsmStopPlayIndex(i);
								SsmClearRxDtmfBuf(i);
								ChInfo[i].ConsentState = 3;
								sprintf_s(CampID, "%d", Campaigns.at(tempCampId).loadedIndex[3]);
								SsmPlayIndexString(i, CampID);
								SsmSetWaitDtmfExA(i, 8000, 1, "9", true);
								StrCpyA(ChInfo[i].CDRStatus.firstConsent, ChInfo[i].CDRStatus.dtmf); //copying correct input only.
							}
							else if (StrCmpA(ChInfo[i].CDRStatus.dtmf, "") == 0) //No Input
							{
								SsmStopPlayIndex(i);
								ChInfo[i].ConsentState = 2;
								sprintf_s(CampID, "%d", Campaigns.at(tempCampId).loadedIndex[2]);
								SsmPlayIndexString(i, CampID);
								SsmSetWaitDtmfExA(i, 18000, 1, "1", true);
							}
							else //Wrong input
							{
								SsmStopPlayIndex(i);
								SsmClearRxDtmfBuf(i);
								ChInfo[i].ConsentState = 4;
								sprintf_s(CampID, "%d", Campaigns.at(tempCampId).loadedIndex[4]);
								SsmPlayIndexString(i, CampID);
								//SsmSetWaitDtmfExA(i, 15000, 1, "1", true);
							}
						}
					}
					break;
				case 2:
					ChInfo[i].mediaState = SsmCheckPlay(i);
					if (ChInfo[i].mediaState >= 0)
					{
						if (SsmChkWaitDtmf(i, ChInfo[i].CDRStatus.dtmf) >= 1)
						{
							if (StrCmpA(ChInfo[i].CDRStatus.dtmf, "1") == 0) //Right Input
							{
								SsmStopPlayIndex(i);
								SsmClearRxDtmfBuf(i);
								ChInfo[i].ConsentState = 3;
								sprintf_s(CampID, "%d", Campaigns.at(tempCampId).loadedIndex[3]);
								SsmPlayIndexString(i, CampID);
								SsmSetWaitDtmfExA(i, 8000, 1, "9", true);
								StrCpyA(ChInfo[i].CDRStatus.firstConsent, ChInfo[i].CDRStatus.dtmf); //copying correct input only.
							}
							else if (StrCmpA(ChInfo[i].CDRStatus.dtmf, "") == 0) //No input
							{
								SsmStopPlayIndex(i);
								ChInfo[i].ConsentState = 4;
								sprintf_s(CampID, "%d", Campaigns.at(tempCampId).loadedIndex[4]);
								SsmPlayIndexString(i, CampID);
							}
							else //Wrong input
							{
								SsmStopPlayIndex(i);
								SsmClearRxDtmfBuf(i);
								ChInfo[i].ConsentState = 4;
								sprintf_s(CampID, "%d", Campaigns.at(tempCampId).loadedIndex[4]);
								SsmPlayIndexString(i, CampID);
							}
						}
					}
					break;
				case 3:
					ChInfo[i].mediaState = SsmCheckPlay(i);
					if (ChInfo[i].mediaState >= 0)
					{
						if (SsmChkWaitDtmf(i, ChInfo[i].CDRStatus.dtmf2) >= 1)
						{
							if (StrCmpA(ChInfo[i].CDRStatus.dtmf2, "9") == 0) //Right Input
							{
								SsmStopPlayIndex(i);
								SsmClearRxDtmfBuf(i);
								ChInfo[i].ConsentState = 4;
								sprintf_s(CampID, "%d", Campaigns.at(tempCampId).loadedIndex[5]);
								SsmPlayIndexString(i, CampID);
								StrCpyA(ChInfo[i].CDRStatus.secondConsent, ChInfo[i].CDRStatus.dtmf2); //copying correct input only.
							}
							else if (StrCmpA(ChInfo[i].CDRStatus.dtmf2, "") == 0) //No input
							{
								SsmStopPlayIndex(i);
								ChInfo[i].ConsentState = 4;
								sprintf_s(CampID, "%d", Campaigns.at(tempCampId).loadedIndex[4]);
								SsmPlayIndexString(i, CampID);
							}
							else //Wrong input
							{
								SsmStopPlayIndex(i);
								SsmClearRxDtmfBuf(i);
								ChInfo[i].ConsentState = 4;
								sprintf_s(CampID, "%d", Campaigns.at(tempCampId).loadedIndex[4]);
								SsmPlayIndexString(i, CampID);
							}
						}
					}
					break;
				case 4:
					ChInfo[i].mediaState = SsmCheckPlay(i);
					if (ChInfo[i].mediaState >= 1)
					{
						HangupCall(i);
					}
					break;
				default:
					break;
				}
				break;
			case Informative:
				ChInfo[i].mediaState = SsmCheckPlay(i);
				if (ChInfo[i].mediaState == 0)
				{
					StrCpyA(ChInfo[i].CDRStatus.reason_code, "0");
					//logger.log(LOGINFO, "media state 0");
				}
				if (ChInfo[i].mediaState >= 1)
				{
					StrCpyA(ChInfo[i].CDRStatus.reason_code, "1");
					//logger.log(LOGINFO, "media state > 1");
					HangupCall(i);
				}
				break;

			case AcquisitionalOBDWith1stConsent:
				switch (ChInfo[i].ConsentState) {
				case 1:
					ChInfo[i].mediaState = SsmCheckPlay(i);
					//logger.log(LOGINFO, "Media state: %d, ph Number: %s", ChInfo[i].mediaState, ChInfo[i].pPhoNumBuf);
					if (ChInfo[i].mediaState >= 0)
					{
						//StrCpyA(ChInfo[i].CDRStatus.dtmf, SsmGetDtmfStrA(i));

						if (SsmChkWaitDtmf(i, ChInfo[i].CDRStatus.dtmf) >= 1)
						{
							if (StrCmpA(ChInfo[i].CDRStatus.dtmf, "1") == 0) //Right Input
							{
								SsmStopPlayIndex(i);
								SsmClearRxDtmfBuf(i);
								ChInfo[i].ConsentState = 3;
								sprintf_s(CampID, "%d", Campaigns.at(tempCampId).loadedIndex[3]);
								SsmPlayIndexString(i, CampID);
								//SsmSetWaitDtmfExA(i, 8000, 1, "9", true);
								StrCpyA(ChInfo[i].CDRStatus.firstConsent, ChInfo[i].CDRStatus.dtmf); //copying correct input only.
							}
							else if (StrCmpA(ChInfo[i].CDRStatus.dtmf, "") == 0) //No Input
							{
								SsmStopPlayIndex(i);
								ChInfo[i].ConsentState = 2;
								sprintf_s(CampID, "%d", Campaigns.at(tempCampId).loadedIndex[2]);
								SsmPlayIndexString(i, CampID);
								SsmSetWaitDtmfExA(i, 18000, 1, "0123456789*#", true);
							}
							else //Wrong input
							{
								SsmStopPlayIndex(i);
								SsmClearRxDtmfBuf(i);
								ChInfo[i].ConsentState = 3;
								sprintf_s(CampID, "%d", Campaigns.at(tempCampId).loadedIndex[3]);
								SsmPlayIndexString(i, CampID);
								//SsmSetWaitDtmfExA(i, 15000, 1, "1", true);
							}
						}
					}
					break;
				case 2:
					ChInfo[i].mediaState = SsmCheckPlay(i);
					if (ChInfo[i].mediaState >= 0)
					{
						if (SsmChkWaitDtmf(i, ChInfo[i].CDRStatus.dtmf) >= 1)
						{
							if (StrCmpA(ChInfo[i].CDRStatus.dtmf, "1") == 0) //Right Input
							{
								SsmStopPlayIndex(i);
								SsmClearRxDtmfBuf(i);
								ChInfo[i].ConsentState = 3;
								sprintf_s(CampID, "%d", Campaigns.at(tempCampId).loadedIndex[3]);
								SsmPlayIndexString(i, CampID);
								//SsmSetWaitDtmfExA(i, 8000, 1, "9", true);
								StrCpyA(ChInfo[i].CDRStatus.firstConsent, ChInfo[i].CDRStatus.dtmf); //copying correct input only.
							}
							else if (StrCmpA(ChInfo[i].CDRStatus.dtmf, "") == 0) //No input
							{
								SsmStopPlayIndex(i);
								ChInfo[i].ConsentState = 3;
								sprintf_s(CampID, "%d", Campaigns.at(tempCampId).loadedIndex[3]);
								SsmPlayIndexString(i, CampID);
							}
							else //Wrong input
							{
								SsmStopPlayIndex(i);
								SsmClearRxDtmfBuf(i);
								ChInfo[i].ConsentState = 3;
								sprintf_s(CampID, "%d", Campaigns.at(tempCampId).loadedIndex[3]);
								SsmPlayIndexString(i, CampID);
							}
						}
					}
					break;
				case 3:
					ChInfo[i].mediaState = SsmCheckPlay(i);
					if (ChInfo[i].mediaState >= 1)
					{
						HangupCall(i);
					}
					break;
				default:
					break;
				}
				break;

			default:
				break;
			}
			//ChInfo[i].mediaState = SsmCheckPlay(i);
			ChInfo[i].lineState = SsmGetChState(i);
			StrCpyA(ChInfo[i].CDRStatus.status, "SUCCESS");
			StrCpyA(ChInfo[i].CDRStatus.reason, "Answered");
			//SsmTalkWithEx for call patching
			ChInfo[i].CDRStatus.end_time = time(0);
			//ChInfo[i].DtmfState = SsmChkWaitDtmf(i, ChInfo[i].DtmfBuf);
			//if (ChInfo[i].DtmfState >= 1 && ChInfo[i].DtmfState <= 3)
			//{
			//	logger.log(LOGINFO, "DTMF Recieved : %s on channel: %d", ChInfo[i].DtmfBuf, i);
			//	if (StrChrA(ChInfo[i].DtmfBuf, '1'))
			//	{
			//		StrCpyA(ChInfo[i].CDRStatus.dtmf, "1");
			//	}
			//	else
			//	{
			//		StrCpyA(ChInfo[i].CDRStatus.dtmf, "");
			//	}
			//	HangupCall(i);
			//}
			//if (ChInfo[i].mediaState == 0 || ChInfo[i].DtmfState == 0)
			//{
			//	StrCpyA(ChInfo[i].CDRStatus.reason_code, "0");
			//	//if (SsmAutoDial(6, "9218580794") == 0)ChInfo[i].nStep = USER_CALL_WAIT_PATCHUP;
			//	//logger.log(LOGERR, "channel %0d Media State: Playing, on phone number: %s", i, ChInfo[i].pPhoNumBuf);
			//}
			////if (ChInfo[i].mediaState >= 1)
			////{
			////	//LogErrorCodeAndMessage(i);
			////	StrCpyA(ChInfo[i].CDRStatus.reason_code, "1");
			////	//logger.log(LOGERR, "channel %0d Media State: %d, on phone number: %s", i, ChInfo[i].mediaState, ChInfo[i].pPhoNumBuf);
			////	HangupCall(i);
			////}
			if (ChInfo[i].lineState == S_CALL_PENDING) //remote/user hangup
			{
				//LogErrorCodeAndMessage(i);
				//logger.log(LOGERR, "channel %0d State: %d, on phone number: %s", i, ChInfo[i].lineState, ChInfo[i].pPhoNumBuf);
				StrCpyA(ChInfo[i].CDRStatus.reason_code, "7");
				HangupCall(i);
			}
			break;
			/* call patchup cases
			case USER_CALL_WAIT_PATCHUP:
			if (SsmChkAutoDial(6) == DIAL_VOICE)
			{
			SsmStopPlayIndex(i);
			SsmTalkWith(i, 6);
			ChInfo[i].nStep = USER_CALL_PATCHUP;
			logger.log(LOGINFO, "call patchup picked up..");
			}
			break;
			case USER_CALL_PATCHUP:
			if (SsmGetChState(i) == S_CALL_PENDING || SsmGetChState(6) == S_CALL_PENDING)
			{
			HangupCall(i);
			HangupCall(6);
			}
			break;*/
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
	return true;
}

void CSpiceOBDDlg::ReadNumbersFromFiles()
{
	//int totalCampaigns = 2;
	//std::ifstream fp1("phoneNumbers1.txt");
	//CampaignData tempdata;
	//char *temp;
	//while (1)
	//{
	//	temp = new char[31];
	//	fp1.getline(temp, ' ');
	//	if (strcmp(temp, "") == 0) break;
	//	tempdata.phnumBuf.push_back(temp);
	//}
	//fp1.close();
	//StrCpyA(tempdata.CLI, "9814701418");
	//tempdata.minCh = 0; tempdata.maxCh = 0;
	//Campaigns.insert(pair<int, CampaignData>(1, tempdata));
	//tempdata.phnumBuf.clear();

	////Second
	//std::ifstream fp2("phoneNumbers2.txt");
	//while (1)
	//{
	//	temp = new char[31];
	//	fp2.getline(temp, ' ');
	//	if (strcmp(temp, "") == 0) break;
	//	tempdata.phnumBuf.push_back(temp);
	//}
	//fp2.close();
	//StrCpyA(tempdata.CLI, "9781001635");
	//tempdata.minCh = 15; tempdata.maxCh = 15;
	//Campaigns.insert(pair<int, CampaignData>(2, tempdata));
	//tempdata.phnumBuf.clear();
}

void CSpiceOBDDlg::openCDRLogFile()
{
	char CurPath[260];
	char fileName[50];
	char folderName[50];
	char timeValue[25];

	GetCurrentDirectoryA(200, CurPath);
	StrCatA(CurPath, "\\CDRLogs");
	if (!PathIsDirectoryA(CurPath))
	{
		CreateDirectoryA(CurPath, NULL);
	}
	tm timeVal = logger.getTime(timeValue);
	sprintf_s(folderName, "\\CDRInfo_%04d%02d", timeVal.tm_year + 1900, timeVal.tm_mon + 1);
	StrCatA(CurPath, folderName);

	if (!PathIsDirectoryA(CurPath))
	{
		CreateDirectoryA(CurPath, NULL);
	}
	sprintf_s(fileName, "\\CDRInfo_%04d%02d%02d.txt", timeVal.tm_year + 1900, timeVal.tm_mon + 1, timeVal.tm_mday);

	StrCatA(CurPath, fileName);

	outfile.open(CurPath, std::ofstream::app);
}

void CSpiceOBDDlg::openConsentLogFile()
{
	char CurPath[260];
	char fileName[50];
	char folderName[50];
	char timeValue[25];

	GetCurrentDirectoryA(200, CurPath);
	StrCatA(CurPath, "\\ConsentLogs");
	if (!PathIsDirectoryA(CurPath))
	{
		CreateDirectoryA(CurPath, NULL);
	}
	tm timeVal = logger.getTime(timeValue);
	sprintf_s(folderName, "\\ConsentInfo_%04d%02d", timeVal.tm_year + 1900, timeVal.tm_mon + 1);
	StrCatA(CurPath, folderName);

	if (!PathIsDirectoryA(CurPath))
	{
		CreateDirectoryA(CurPath, NULL);
	}
	sprintf_s(fileName, "\\ConsentInfo_%04d%02d%02d.txt", timeVal.tm_year + 1900, timeVal.tm_mon + 1, timeVal.tm_mday);

	StrCatA(CurPath, fileName);

	ConsentFile.open(CurPath, std::ofstream::app);
}

BOOL CSpiceOBDDlg::InitilizeChannels()
{
	try {
		//ReadNumbersFromFiles();
		if (GetDBData() == TRUE)
		{
			//logger.log(LOGINFO, "map size: %d, vector1 size: %d", Campaigns.size(), Campaigns.size() > 0 ? Campaigns.at(1).phnumBuf.size() : 0);
			systemIpAddr = Utils::GetIPAdd();
			IsStartDialling = false;
			openCDRLogFile();
			openConsentLogFile();
			//Initialization of channels on trunk-board
			ChInfo = new CH_INFO[nTotalCh];

			for (int i = 0; i < nTotalCh; i++)
			{
				ChInfo[i].EnCalled = false;
				ChInfo[i].rowTobeUpdated = false;
				int chType = SsmGetChType(i);
				if (chType == 11) //ISUP channel(China SS7 signaling ISUP)
				{
					if (SsmGetChState(i) == 0) //check idle
					{
						ChInfo[i].nStep = USER_IDLE;
					}
					else
					{
						//Set idle
						SsmHangup(i);
						ChInfo[i].nStep = USER_IDLE;
					}
				}
				if (ChInfo[i].nStep == USER_IDLE)
				{
					ChInfo[i].CampaignID = -1;
					StrCpyA(ChInfo[i].pPhoNumBuf, "");
					StrCpyA(ChInfo[i].CDRStatus.encrypted_ani, "");
					StrCpyA(ChInfo[i].CDRStatus.dtmf, "");
					StrCpyA(ChInfo[i].CDRStatus.dtmf2, "");
					StrCpyA(ChInfo[i].CDRStatus.firstConsent, "");
					StrCpyA(ChInfo[i].CDRStatus.secondConsent, "");

					for (size_t j = 1; j <= Campaigns.size(); j++)
					{
						if (i >= Campaigns.at(j).minCh && i <= Campaigns.at(j).maxCh)
						{
							ChInfo[i].CampaignID = j;
							/*if (!Campaigns.at(j).phnumBuf.empty())
							{
								StrCpyA(ChInfo[i].pPhoNumBuf, Campaigns.at(j).phnumBuf.front().ani);
								StrCpyA(ChInfo[i].CDRStatus.encrypted_ani, Campaigns.at(j).phnumBuf.front().encryptedAni);
								Campaigns.at(j).phnumBuf.erase(Campaigns.at(j).phnumBuf.begin());
								logger.log(LOGINFO, "InitilizeChannels Update data Ani : %s, Encrypted Ani: %s, Channel Num: %d", ChInfo[i].pPhoNumBuf, ChInfo[i].CDRStatus.encrypted_ani, i);
								if (!UpdatePhNumbersStatus(i))
								{
									logger.log(LOGINFO, "InitilizeChannels Row not updated... channel number: %d", i);
								}
							}*/
							//setting caller ID 
							if (SsmSetTxCallerId(i, Campaigns.at(j).CLI) == -1)
							{
								getErrorResult(L" SsmSetTxCallerId");
								return false;
							}
						}
					}
					StrCpyA(ChInfo[i].CDRStatus.reason, "");
					StrCpyA(ChInfo[i].CDRStatus.status, "");
					StrCpyA(ChInfo[i].CDRStatus.reason_code, "");
				}
			}
			//Loading wav file on different positions for all campaigns
			char alias[10];
			int index = 0;
			for (size_t i = 1; i <= Campaigns.size(); i++)
			{
				char tempPath[255];
				char fileName[16];
				int j = 1;
				StrCpyA(tempPath, "");
				//Campaigns.at(i).hasReachedThreshold = false;
				while (true)
				{
					StrCpyA(tempPath, Campaigns.at(i).promptsPath);
					sprintf_s(fileName, "%d.wav", j);
					StrCatA(tempPath, fileName);
					if (PathFileExistsA(tempPath))
					{
						index = i * 10 + j;
						sprintf_s(alias, "alias%d", index);
						if (SsmLoadIndexData(index, alias, 7, tempPath, 0, -1) != 0)
						{
							CString errMsg;
							errMsg.Format(L"Load Index %d Error", index);
							AfxMessageBox(errMsg);
							PostQuitMessage(0);
						}
						Campaigns.at(i).loadedIndex[j] = index;
						//logger.log(LOGINFO, "Path: %s, index: %d, alias: %s, total Index: %d", tempPath, Campaigns.at(i).loadedIndex[j], alias, SsmGetTotalIndexSeg());
						j++;
					}
					else
					{
						break;
					}
				}//End While
			}//End For
		}
	}
	catch (...)
	{
		outfile.close();
		ConsentFile.close();
		openCDRLogFile();
		openConsentLogFile();
		char errMsg[256];
		SsmGetLastErrMsg(errMsg);
		logger.log(LOGERR, "On Initialize Channels :%s", errMsg);
	}
	return true;
}

void CSpiceOBDDlg::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	try
	{
		DoUserWork();
		UpDateATrunkChListCtrl();
	}
	catch (...)
	{
		outfile.close();
		ConsentFile.close();
		openCDRLogFile();
		openConsentLogFile();
		char errMsg[256];
		SsmGetLastErrMsg(errMsg);
		logger.log(LOGERR, "On Timers :%s", errMsg);

	}
	CDialog::OnTimer(nIDEvent);
}

void CSpiceOBDDlg::OnBnClickedLogLevel()
{
	UpdateData();
	logger.SetMinLogLevel(m_SetMinLogLevel);
	logger.log(LOGFATAL, "Minimum Log level:%d", m_SetMinLogLevel);
}

void CSpiceOBDDlg::OnDestroy()
{
	CDialog::OnDestroy();
	delete[] ChInfo;
	outfile.close();
	ConsentFile.close();
	SsmCloseCti();
	CloseDBConn();
}

void CSpiceOBDDlg::OnBnClickedDiallingStart()
{
	// TODO: Add your control notification handler code here
	IsStartDialling = true;
	 
}


void CSpiceOBDDlg::OnBnClickedDiallingStop()
{
	// TODO: Add your control notification handler code here
	IsStartDialling = false;
}
