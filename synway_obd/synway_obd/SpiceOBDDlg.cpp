
// SpiceOBDDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpiceOBD.h"
#include "SpiceOBDDlg.h"
#include "afxdialogex.h"
#include "errmsg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifndef ROW_COUNT
#define ROW_COUNT 20
#endif // !1

#ifndef COMMENTED_SECTION
#define COMMENTED_SECTION 0
#endif


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


const int CSpiceOBDDlg::MaxPromptsRepeatsCount = 3;

CSpiceOBDDlg::CSpiceOBDDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SYNWAY_OBD_DIALOG, pParent)
	, m_SetMinLogLevel(0), aesEncryption("1234567891011121")
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

int CSpiceOBDDlg::OffSet = ROW_COUNT;
int CSpiceOBDDlg::row_count = ROW_COUNT;
int CSpiceOBDDlg::loopCountForCampaignUpdate = 0;
BOOL CSpiceOBDDlg::isDeallocateProcedureCalled = false;

CStatic CSpiceOBDDlg::dailingValCtrl;
CStatic CSpiceOBDDlg::connctedValCtrl;
CStatic CSpiceOBDDlg::cgValCtrl;
CStatic CSpiceOBDDlg::nChDownCtrl;
CStatic CSpiceOBDDlg::totalChannelsAvlCtrl;
CStatic CSpiceOBDDlg::mChDownRangeVal;

void CSpiceOBDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_TRK, m_TrkChList);
	DDX_Radio(pDX, IDC_TRACE, m_SetMinLogLevel);
	DDX_Control(pDX, IDC_DAILING_VALUE, dailingValCtrl);
	DDX_Control(pDX, IDC_CONNECTED_VALUE, connctedValCtrl);
	DDX_Control(pDX, IDC_CG_VALUE, cgValCtrl);
	DDX_Control(pDX, IDC_TOTCH_VAL, totalChannelsAvlCtrl);
	DDX_Control(pDX, IDC_CHDWN_VAL, nChDownCtrl);
	DDX_Control(pDX, IDC_WAIT_ANSWER_LIST, mWaitAnswerComboCtrl);
	DDX_Control(pDX, IDC_SET_WAIT_ANSWER_TIME, mSetWaitAnswerTimeOutBtn);
	DDX_Control(pDX, IDC_RETRY_ALERT, mRetryAlertMsg);
	DDX_Control(pDX, IDC_CHDOWN_RANGE, mChDownRangeVal);
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
	ON_BN_CLICKED(IDC_SET_WAIT_ANSWER_TIME, &CSpiceOBDDlg::OnBnClickedSetWaitAnswerTime)
	ON_CBN_SELCHANGE(IDC_WAIT_ANSWER_LIST, &CSpiceOBDDlg::OnCbnSelchangeWaitAnswerList)
	ON_WM_CTLCOLOR()
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

		m_TrkChList.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
		CRect rect;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
		int screen_x_size = rect.Width();
		int screen_y_size = rect.Height();
		SetDiallingStartStopBtn(true);
		::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, screen_x_size, screen_y_size, SWP_NOZORDER);

		SetTimer(INITIALIZE_CTRL, 2 * 1000, NULL);

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
	//SetTimer(1000, 200, NULL);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

/*** Initializes DB connection ***/
void CSpiceOBDDlg::InitializeDBConnection()
{
	conn = mysql_init(NULL);
	connBase = mysql_init(NULL);
	connSelect = mysql_init(NULL);
	connInsert = mysql_init(NULL);
	connUpdate = mysql_init(NULL);
	connPort = mysql_init(NULL);

	//unsigned int connTimeOut = 365 * 24 * 3600;
	char CurPath[260], tmpWaitTimeListStr[256];
	GetCurrentDirectoryA(200, CurPath);
	StrCpyA(InitDBSettings, CurPath);
	StrCatA(InitDBSettings, "\\DBSettings.INI");
	// Fetching all the details required for DB connection.
	GetPrivateProfileStringA("Database", "host", "localhost", host, 255, InitDBSettings);
	GetPrivateProfileStringA("Database", "DBName", "test_db", DBName, 255, InitDBSettings);
	GetPrivateProfileStringA("Database", "username", "root", username, 255, InitDBSettings);
	GetPrivateProfileStringA("Database", "password", "sdl@1234", password, 255, InitDBSettings);
	GetPrivateProfileStringA("Database", "circle", "circle", circle, 20, InitDBSettings);
	GetPrivateProfileStringA("Database", "blockedchannelrange", "", blockedRangeStr, 100, InitDBSettings);
	GetPrivateProfileStringA("Database", "triggerobdchrange", "", triggerOBDChRangeStr, 100, InitDBSettings);
	GetPrivateProfileStringA("Database", "triggerobdname", "", triggerOBDName, 100, InitDBSettings);
	//Get wait dial answer tie out parameters 
	GetPrivateProfileStringA("Database", "waitdialtimerange", "30$40$50$", tmpWaitTimeListStr, 256, InitDBSettings);
	GetPrivateProfileStringA("Database", "waitdialanswertimeout", "45", curWaitTimeOutStr, 20, InitDBSettings);
	GetPrivateProfileStringA("Database", "contestch", "", contestChRange, 20, InitDBSettings);
	GetPrivateProfileStringA("Database", "obdstarttime", "09:00#", obdStartTimeStr, 50, InitDBSettings);
	GetPrivateProfileStringA("Database", "obdstoptime", "20:50#", obdStopTimeStr, 50, InitDBSettings);

	GetPrivateProfileStringA("Database", "RVCampaign", "14546", rvCampaign, 50, InitDBSettings);
	GetPrivateProfileStringA("Database", "NameTunesPrev", "dear", nameTunesPrev, 50, InitDBSettings);
	GetPrivateProfileStringA("Database", "NameTune", "name_tunes", namingTunes, 50, InitDBSettings);
	GetPrivateProfileStringA("Database", "NameTunesPost", "name_tunes", nameTunesPost, 50, InitDBSettings);

	port = GetPrivateProfileIntA("Database", "Port", 3306, InitDBSettings);
	IsSMSApiEnabled = GetPrivateProfileIntA("Database", "enablesmsapi", 0, InitDBSettings);

	waitTimeListStr = tmpWaitTimeListStr;

	//Setting title name
	char titleName[50];
	StrCpyA(titleName, "Spice OBD - ");
	StrCatA(titleName, circle);
	SetWindowTextA(m_hWnd, titleName);

	logger.log(LOGINFO, "Start time : %s, Stop Time: %s", obdStartTimeStr, obdStopTimeStr);
	if (StrCmpA(obdStartTimeStr, "") && StrCmpA(obdStopTimeStr, ""))
	{
		const char* delim = ":#";
		char *context;
		char *startTimeValueStr = strtok_s(obdStartTimeStr, delim, &context);
		startTimeHour = atoi(startTimeValueStr);
		if (startTimeValueStr)
		{
			startTimeValueStr = strtok_s(NULL, delim, &context);
			startTimeMin = atoi(startTimeValueStr);
		}

		char *stopTimeValueStr = strtok_s(obdStopTimeStr, delim, &context);
		stopTimeHour = atoi(stopTimeValueStr);
		if (stopTimeValueStr)
		{
			stopTimeValueStr = strtok_s(NULL, delim, &context);
			stopTimeMin = atoi(stopTimeValueStr);
		}
		logger.log(LOGINFO, "start hour: %d, start minute: %d, stop hour: %d, stop minute : %d",
			startTimeHour, startTimeMin, stopTimeHour, stopTimeMin);
	}

	//CGMaxCHNum = GetPrivateProfileIntA("Database", "CGMaxCHNum", 30, InitDBSettings);
	//nTotalCh = GetPrivateProfileIntA("Database", "TotalChannelsCount", 90, InitDBSettings);
	logger.log(LOGINFO, "host: %s, username: %s, password: %s, dbname: %s, port: %d", host, username, password, DBName, port);
	//mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &connTimeOut);
	if (mysql_real_connect(conn, host, username, password, DBName, port, NULL, 0) == 0)
	{
		AfxMessageBox(L"Connection failed to Database");
		PostQuitMessage(0);
	}
	//mysql_options(connBase, MYSQL_OPT_CONNECT_TIMEOUT, &connTimeOut);
	if (mysql_real_connect(connBase, host, username, password, DBName, port, NULL, 0) == 0)
	{
		AfxMessageBox(L"Connection failed to Database for Dialer base");
		PostQuitMessage(0);
	}
	//mysql_options(connSelect, MYSQL_OPT_CONNECT_TIMEOUT, &connTimeOut);
	if (mysql_real_connect(connSelect, host, username, password, DBName, port, NULL, 0) == 0)
	{
		AfxMessageBox(L"Connection failed to Database for Dialer base");
		PostQuitMessage(0);
	}
	//mysql_options(connInsert, MYSQL_OPT_CONNECT_TIMEOUT, &connTimeOut);
	if (mysql_real_connect(connInsert, host, username, password, DBName, port, NULL, 0) == 0)
	{
		AfxMessageBox(L"Connection failed to Database for Dialer base");
		PostQuitMessage(0);
	}
	//mysql_options(connUpdate, MYSQL_OPT_CONNECT_TIMEOUT, &connTimeOut);
	if (mysql_real_connect(connUpdate, host, username, password, DBName, port, NULL, CLIENT_FOUND_ROWS) == 0)
	{
		AfxMessageBox(L"Connection failed to Database for Dialer base");
		PostQuitMessage(0);
	}
	//mysql_options(connPort, MYSQL_OPT_CONNECT_TIMEOUT, &connTimeOut);
	if (mysql_real_connect(connPort, host, username, password, DBName, port, NULL, 0) == 0)
	{
		AfxMessageBox(L"Connection failed to Database for Dialer base");
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
		/*int tempCampId = ChInfo[i].CampaignID;
		if (tempCampId != -1)
		{
		CString tempStr1(ChInfo[i].pPhoNumBuf), tempStr2(Campaigns.at(tempCampId).CLI), campIdStr;
		campIdStr.Format(_T("%d"), tempCampId);

		m_TrkChList.SetItemText(i, 3, tempStr2);
		m_TrkChList.SetItemText(i, 4, tempStr1);
		m_TrkChList.SetItemText(nItem, 5, campIdStr);
		}*/
		/*CString chStateNum;
		chStateNum.Format(L"%d", SsmGetChState(i));
		m_TrkChList.SetItemText(nItem, 6, chStateNum);*/
	}
	UpDateATrunkChListCtrl();
}

void CSpiceOBDDlg::LogOBDAlerts(const char* alertMessage, const char* messageType = "EVENT", const char* alertLevel = "L1")
{
	MYSQL *connAlert = mysql_init(NULL);
	char queryStr[1024];

	if (mysql_real_connect(connAlert, host, username, password, DBName, port, NULL, 0) == NULL)
	{
		logger.log(LOGERR, "Connection failed to Database for alert logging Mysql Error: %s", mysql_error(connAlert));
		AfxMessageBox(L"Connection failed to Database for alert logging");
		PostQuitMessage(0);
	}

	sprintf_s(queryStr, "insert into tbl_obd_dialer_alerts (circle, machine_ip, alert_message, insert_datetime, message_type, alert_level)  values('%s', '%s', '%s', now(), '%s', '%s')",
		circle, systemIpAddr, alertMessage, messageType, alertLevel);

	logger.log(LOGINFO, queryStr);

	int query_state = mysql_query(connAlert, queryStr);
	if (query_state != 0)
	{
		logger.log(LOGERR, "Procedure calling Mysql Error: %s", mysql_error(connAlert));
	}
	if (connAlert != NULL)
	{
		mysql_close(connAlert);
		connAlert = NULL;
	}
}


UINT CSpiceOBDDlg::CallProcedure(LPVOID deallocateProcParam)
{
	//MYSQL_BIND bind[1];
	//MYSQL_STMT *stmt;
	//DWORD str_length;
	char queryProc[1024];
	char campaignId[100];

	MYSQL_RES* resProc;
	int status;

	//char PROC_SAMPLE[1024];
	isDeallocateProcedureCalled = true;
	DeallocateProcParam * deallocateParams = (DeallocateProcParam*)deallocateProcParam;
	CSpiceOBDDlg* self = deallocateParams->spiceDlg;
	try
	{
		self->logger.log(LOGINFO, "CallProcedure function start campaign_id : %s", deallocateParams->campaign_id);
		self->mRetryAlertMsg.SetWindowTextW(_T("RETRY IN PROGRESS..."));
		self->connCallProc = mysql_init(NULL);

		if (mysql_real_connect(self->connCallProc, self->host, self->username, self->password, self->DBName, self->port, NULL, CLIENT_MULTI_STATEMENTS) == 0)
		{
			self->logger.log(LOGERR, "Connection failed to Database for Retry Mysql Error: %s", mysql_error(self->connCallProc));
			AfxMessageBox(L"Connection failed to Database for Retry");
			PostQuitMessage(0);
		}
		StrCpyA(campaignId, deallocateParams->campaign_id);

		sprintf_s(queryProc, "CALL procDeallocateChannel('%s')", campaignId);

		int query_state = mysql_query(self->connCallProc, queryProc);
		if (query_state != 0)
		{
			self->logger.log(LOGERR, "Procedure calling Mysql Error: %s", mysql_error(self->connCallProc));
		}

		do {
			resProc = mysql_store_result(self->connCallProc);
			if (resProc)
			{
				mysql_free_result(resProc);
			}
			else /* no result set or error */
			{
				if (mysql_field_count(self->connCallProc) == 0)
				{
					self->logger.log(LOGINFO, "%lld rows affected", mysql_affected_rows(self->connCallProc));
				}
				else  /* some error occurred */
				{
					self->logger.log(LOGERR, "Could not retrieve result set");
					break;
				}
			}
			/* more results? -1 = no, >0 = error, 0 = yes (keep looping) */
			if ((status = mysql_next_result(self->connCallProc)) > 0)
				self->logger.log(LOGERR, "Could not execute statement");
		} while (status == 0);

#if COMMENTED_SECTION
		//StrCpyA(PROC_SAMPLE, "CALL procDeallocateChannel(?)");

		//self->logger.log(LOGINFO, "CallProcedure function start campaign_id : %s", deallocateParams->campaign_id);
		//stmt = mysql_stmt_init(self->connCallProc);
		//if (!stmt)
		//{
		//	self->logger.log(LOGERR, " mysql_stmt_init(), out of memory");
		//}
		//if (mysql_stmt_prepare(stmt, PROC_SAMPLE, strlen(PROC_SAMPLE)))
		//{
		//	self->logger.log(LOGERR, " mysql_stmt_prepare(), procedure failed");
		//	self->logger.log(LOGERR, " %s", mysql_stmt_error(stmt));
		//}
		//int param_count = mysql_stmt_param_count(stmt);

		//if (param_count != 1) /* validate parameter count */
		//{
		//	self->logger.log(LOGERR, " invalid parameter count returned by MySQL");
		//}

		//memset(bind, 0, sizeof(bind));

		///* STRING PARAM */
		///* This is a number type, so there is no need
		//to specify buffer_length */
		//bind[0].buffer_type = MYSQL_TYPE_STRING;
		//bind[0].buffer = (char*)str_data;
		//bind[0].buffer_length = 1024;
		//bind[0].is_null = 0;
		//bind[0].length = &str_length;

		//if (mysql_stmt_bind_param(stmt, bind))
		//{
		//	self->logger.log(LOGERR, " mysql_stmt_bind_param() failed");
		//	self->logger.log(LOGERR, " %s", mysql_stmt_error(stmt));
		//}

		//strncpy_s(str_data, deallocateParams->campaign_id, 100); /* string  */
		//str_length = strlen(str_data);


		//if (mysql_stmt_execute(stmt))
		//{
		//	self->logger.log(LOGERR, " mysql_stmt_execute(), 1 failed");
		//	self->logger.log(LOGERR, " %s", mysql_stmt_error(stmt));
		//}
		//mysql_stmt_close(stmt);
#endif // COMMENTED_SECTION
		if (self->connCallProc != NULL)
		{
			mysql_close(self->connCallProc);
			self->connCallProc = NULL;
		}
		self->logger.log(LOGINFO, "CallProcedure function end campaign_id : %s", deallocateParams->campaign_id);
		isDeallocateProcedureCalled = false;
		self->mRetryAlertMsg.SetWindowTextW(_T(""));
	}
	catch (...)
	{
		self->logger.log(LOGERR, "Unhandled exception caught in call procedure...");
	}
	return 0;
}

void CSpiceOBDDlg::RefreshDBConnection(MYSQL* dbConn, const char* dbQuery)
{
	logger.log(LOGERR, "RefreshDBConnection->Mysql Error: %s", mysql_error(dbConn));
	unsigned int errorCode = mysql_errno(dbConn);
	if (errorCode == CR_SERVER_LOST || errorCode == CR_SERVER_GONE_ERROR)
	{
		mysql_close(dbConn);
		dbConn = mysql_init(NULL);
		if (mysql_real_connect(dbConn, host, username, password, DBName, port, NULL, 0) == 0)
		{
			AfxMessageBox(L"Connection failed to Database");
			PostQuitMessage(0);
		}
		int query_state = mysql_query(dbConn, dbQuery);
		if (query_state != 0)
		{
			logger.log(LOGERR, "RefreshDBConnection failed->Mysql Error: %s", mysql_error(dbConn));
		}
	}
}

void CSpiceOBDDlg::GetSongsMasterData(char* campaignId, size_t campaignKey)
{
	char songQuery[1024];
	int queryState;

	sprintf_s(songQuery, "select GROUP_CONCAT(concat_ws('$', dtmf, promo_code, lang_code) SEPARATOR ', ') as dtmf_promo_code, getSecondDnis('%s') as DNIS, repeat_level, level_type, cg_level, no_key, invalid_key \
		from tbl_songs_master where campaign_id = '%s' group by repeat_level", campaignId, campaignId);

	queryState = mysql_query(connSelect, songQuery);
	logger.log(LOGINFO, "Song master date query: %s", songQuery);
	if (queryState != 0)
	{
		CString err(mysql_error(connSelect));
		AfxMessageBox(err);
	}

	MYSQL_RES *resPromo = mysql_store_result(connSelect);
	MYSQL_ROW rowPromo;
	map<int, SongsRepeatLevelInfo>().swap(Campaigns[campaignKey].tblSongsMaster);//Clear all data
	while ((rowPromo = mysql_fetch_row(resPromo)) != NULL)
	{
		SongsRepeatLevelInfo songsRepeatLevelInfo;
		//std::map<std::string, std::string> dtmfPromoCodePairList;
		char dtmfPromoCodePair[1024];
		StrCpyA(dtmfPromoCodePair, rowPromo[0]);
		const char* commaDelim = ", ";
		char *firstContext;
		//Fetch total channel for dialout IVR
		map<std::string, DTMFWiseData>().swap(songsRepeatLevelInfo.dtmfWiseData); //Clear all data
		char *dtmfPromoCodePairListPtr = strtok_s(dtmfPromoCodePair, commaDelim, &firstContext);
		while (dtmfPromoCodePairListPtr)
		{
			char* dtmfKey;
			DTMFWiseData tmpDTMFWiseData;
			const char* dollarDelim = "$";
			char *nextContext;
			dtmfKey = strtok_s(dtmfPromoCodePairListPtr, dollarDelim, &nextContext);
			tmpDTMFWiseData.dtmfPromoCode = strtok_s(NULL, dollarDelim, &nextContext);
			char *tmpLangCode = strtok_s(NULL, dollarDelim, &nextContext);
			tmpDTMFWiseData.langCode = tmpLangCode ? tmpLangCode : DEFAULT_LANG_CODE; //deafult language code
			songsRepeatLevelInfo.dtmfWiseData.insert(pair<std::string, DTMFWiseData>(dtmfKey, tmpDTMFWiseData));
			dtmfPromoCodePairListPtr = strtok_s(NULL, commaDelim, &firstContext);
		}
		StrCpyA(songsRepeatLevelInfo.patchDnis, rowPromo[1]);
		StrCpyA(songsRepeatLevelInfo.levelType, rowPromo[3]);
		StrCpyA(songsRepeatLevelInfo.cgLevel, rowPromo[4]);
		StrCpyA(songsRepeatLevelInfo.noKeyLevel, rowPromo[5]);
		StrCpyA(songsRepeatLevelInfo.invalidKeyLevel, rowPromo[6]);

		Campaigns[campaignKey].tblSongsMaster.insert(pair<int, SongsRepeatLevelInfo>(atoi(rowPromo[2]), songsRepeatLevelInfo));
	}
	mysql_free_result(resPromo);
}


BOOL CSpiceOBDDlg::GetDBData()
{
	/***  Read Records  ***/
	try
	{
		//call to decrypt values read from DB
		int query_state;
		char queryStr[1024];

		StrCpyA(queryStr, "select tcm.campaign_id, tcm.cli, tcm.port_number, tcm.prompts_directory, tcm.obd_type, tcm.circle, tcm.zone, tcm.campaign_name, \
			tcm.first_consent_digit,tcm.test_callnumber, tcm.test_callctr, tcm.test_callflag, tcm.current_retry, tsm.cgserviceid, tsm.enable_sms \
			from tbl_campaign_master as tcm inner join tbl_service_master as tsm on tcm.service_name = tsm.service_name \
		where(tcm.campaign_status = 1 or tcm.campaign_status = 2) and (tcm.base_status = 1 or tcm.test_callflag = 1) and tcm.prompts_status = 1 order by camp_seqId");

		logger.log(LOGINFO, queryStr);

		query_state = mysql_query(conn, queryStr);

		if (query_state != 0)
		{
			//RefreshDBConnection(conn, queryStr);
			CString err(mysql_error(conn));
			AfxMessageBox(err);
		}

		res = mysql_store_result(conn);

		CampaignData tempdata;
		size_t campKey;
		int channelsOccupied = -1;

		//StrCpyA(circle, "");
		//StrCpyA(zone, "");

		for (size_t campaignKey = 1; campaignKey <= Campaigns.size(); campaignKey++)
		{
			Campaigns.at(campaignKey).channelsAllocated = 0;
			Campaigns.at(campaignKey).maxCh = Campaigns.at(campaignKey).minCh = -1;
		}
		while ((row = mysql_fetch_row(res)) != NULL)
		{
			BOOL isNewCampaign = true;
			BOOL isTriggerOBD = false;
			for (size_t campaignKey = 1; campaignKey <= Campaigns.size(); campaignKey++)
			{
				if (StrCmpA(Campaigns.at(campaignKey).campaign_id, row[0]) == 0)
				{
					isNewCampaign = false;
					//StrCpyA(Campaigns.at(campaignKey).CLI, row[1]);
					std::string strBuf = row[1];
					size_t offset;
					logger.log(LOGINFO, "Repeat Campaign ID: %s, CLI: %s, CLI vector Size: %d",
						Campaigns.at(campaignKey).campaign_id, strBuf.c_str(), Campaigns.at(campaignKey).cliList.size());
					//Campaigns.at(campaignKey).cliList.clear();
					vector<std::string>().swap(Campaigns.at(campaignKey).cliList);
					while ((offset = strBuf.find("#")) != std::string::npos)
					{
						Campaigns.at(campaignKey).cliList.push_back(strBuf.substr(0, offset));
						strBuf.erase(0, offset + 1);
					}

					if (Campaigns.at(campaignKey).cliList.empty())
					{
						Campaigns.at(campaignKey).cliList.push_back(strBuf);
					}
					Campaigns.at(campaignKey).channelsAllocated = atoi(row[2]);
					StrCpyA(Campaigns.at(campaignKey).promptsDirectory, row[3]);
					Campaigns.at(campaignKey).obdDialPlan = (OBD_DIAL_PLAN)atoi(row[4]);
					//StrCpyA(tempdata.campaign_name, row[7]);
					StrCpyA(Campaigns.at(campaignKey).first_consent_digit, row[8]);

					StrCpyA(Campaigns.at(campaignKey).testCallNumber, row[9]);
					Campaigns.at(campaignKey).testCallCounter = atoi(row[10]);
					Campaigns.at(campaignKey).testCallflag = atoi(row[11]);
					Campaigns.at(campaignKey).curRetryCount = atoi(row[12]);
					StrCpyA(Campaigns.at(campaignKey).cgShortCode, row[13]);
					Campaigns.at(campaignKey).enableSMSFlag = atoi(row[14]);

					if (!StrCmpA(Campaigns.at(campaignKey).campaign_name, triggerOBDName)) //trigger obd found
					{
						isTriggerOBD = true;
						Campaigns.at(campaignKey).minCh = triggerOBDRange[0];
						Campaigns.at(campaignKey).channelsAllocated = triggerOBDRange[1] - triggerOBDRange[0] + 1;
						Campaigns.at(campaignKey).maxCh = triggerOBDRange[1];
					}
					else
					{
						Campaigns.at(campaignKey).minCh = channelsOccupied + 1;
						if (Campaigns.at(campaignKey).minCh >= nIVRMinCh && Campaigns.at(campaignKey).minCh <= nIVRMaxCh)
						{
							Campaigns.at(campaignKey).minCh = nIVRMaxCh + 1;
						}
						channelsOccupied = Campaigns.at(campaignKey).minCh + Campaigns.at(campaignKey).channelsAllocated - 1;
						if (channelsOccupied > Campaigns.at(campaignKey).minCh && ((channelsOccupied >= nIVRMinCh && channelsOccupied <= nIVRMaxCh) ||
							(Campaigns.at(campaignKey).minCh < nIVRMinCh && nIVRMaxCh < channelsOccupied)))
						{
							channelsOccupied = channelsOccupied + (nIVRMaxCh - nIVRMinCh) + 1;
						}
						Campaigns.at(campaignKey).maxCh = channelsOccupied;
					}
					campKey = campaignKey;
				}
			}
			if (isNewCampaign)
			{
				StrCpyA(tempdata.campaign_id, row[0]);
				//StrCpyA(tempdata.CLI, row[1]);
				std::string strBuf = row[1];
				size_t offset;
				logger.log(LOGINFO, "Start Campaign ID: %s, CLI: %s, CLI vector Size: %d",
					tempdata.campaign_id, strBuf.c_str(), tempdata.cliList.size());
				//Campaigns.at(campaignKey).cliList.clear();
				vector<std::string>().swap(tempdata.cliList); //Clear Vector

				while ((offset = strBuf.find("#")) != std::string::npos)
				{
					tempdata.cliList.push_back(strBuf.substr(0, offset));
					strBuf.erase(0, offset + 1);
				}

				if (tempdata.cliList.empty())
				{
					tempdata.cliList.push_back(strBuf);
				}
				tempdata.channelsAllocated = atoi(row[2]);
				StrCpyA(tempdata.promptsDirectory, row[3]);
				tempdata.obdDialPlan = (OBD_DIAL_PLAN)atoi(row[4]);
				StrCpyA(tempdata.campaign_name, row[7]);
				StrCpyA(tempdata.first_consent_digit, row[8]);

				//Initialize call counter for each campaign;
				StrCpyA(tempdata.testCallNumber, row[9]);
				tempdata.testCallCounter = atoi(row[10]);
				tempdata.testCallflag = atoi(row[11]);
				tempdata.curRetryCount = atoi(row[12]);
				StrCpyA(tempdata.cgShortCode, row[13]); //would not change throughout
				tempdata.enableSMSFlag = atoi(row[14]); //check whether sms is to be sent

				tempdata.tmpCallCounter = 0;

				if (!StrCmpA(tempdata.campaign_name, triggerOBDName)) //trigger obd found
				{
					isTriggerOBD = true;
					tempdata.minCh = triggerOBDRange[0]; //TODO
					tempdata.maxCh = triggerOBDRange[1];
					tempdata.channelsAllocated = triggerOBDRange[1] - triggerOBDRange[0] + 1;
				}
				else
				{
					tempdata.minCh = channelsOccupied + 1;
					if (tempdata.minCh >= nIVRMinCh && tempdata.minCh <= nIVRMaxCh)
					{
						tempdata.minCh = nIVRMaxCh + 1;
					}
					channelsOccupied = tempdata.minCh + tempdata.channelsAllocated - 1;
					if (channelsOccupied > tempdata.minCh && ((channelsOccupied >= nIVRMinCh && channelsOccupied <= nIVRMaxCh) ||
						(tempdata.minCh < nIVRMinCh && nIVRMaxCh < channelsOccupied)))
					{
						channelsOccupied = channelsOccupied + (nIVRMaxCh - nIVRMinCh) + 1;
					}
					tempdata.maxCh = channelsOccupied;
				}
				logger.log(LOGINFO, "Campaign Id: %s,  campaign Min ch: %d, campaign Max ch: %d", tempdata.campaign_id, tempdata.minCh, tempdata.maxCh);
				campKey = Campaigns.size() + 1;
				Campaigns.insert(pair<int, CampaignData>(campKey, tempdata));
			}
			if ((Campaigns.at(campKey).phnumBuf.size() < (2 * Campaigns.at(campKey).channelsAllocated)))
			{
				GetSongsMasterData(row[0], campKey);
				//Get out dialer numbers 5 times to the allocated channel numbers to the campaign
				sprintf_s(queryStr, "select ani, priority, prompts_name, obd_type from tbl_outdialer_base \
				where date(insert_date_time) = date(now()) and campaign_id = '%s' and status = %d order by priority,insert_date_time limit %d",
					row[0], 0, (5 * Campaigns.at(campKey).channelsAllocated));

				logger.log(LOGINFO, queryStr);

				query_state = mysql_query(connBase, queryStr);

				if (query_state != 0)
				{
					CString err(mysql_error(connBase));
					AfxMessageBox(err);
				}
				MYSQL_RES * resPhBuf = mysql_store_result(connBase);
				MYSQL_ROW rowPhBuf;
				BOOL isCampaignCompleted = true;
				//Campaigns.at(campKey).phnumBuf.clear();
				vector<pnNumWithEncryptedAni>().swap(Campaigns.at(campKey).phnumBuf);
				while ((rowPhBuf = mysql_fetch_row(resPhBuf)) != NULL)
				{
					size_t curIndex = Campaigns.at(campKey).phnumBuf.size();
					//logger.log(LOGINFO, "Going for decryption of the string buffer:  %s size of campaigns is:%d", rowPhBuf[0], Campaigns.size());
					//std::string DecryptedVal = aesEncryption.DecodeAndDecrypt(rowPhBuf[0]);
					Campaigns.at(campKey).phnumBuf.reserve(5 * Campaigns.at(campKey).channelsAllocated);
					Campaigns.at(campKey).phnumBuf.push_back({/* DecryptedVal,*/"", "", atoi(rowPhBuf[1]), atoi(rowPhBuf[3]) });
					StrCpyA(Campaigns.at(campKey).phnumBuf.at(curIndex).encryptedAni, rowPhBuf[0]);
					StrCpyA(Campaigns.at(campKey).phnumBuf.at(curIndex).promptsName, rowPhBuf[2]);
					isCampaignCompleted = false;
				}
				mysql_free_result(resPhBuf);
				//checks if this campaign completed.
				if (!isTriggerOBD && isCampaignCompleted)
				{
					/*std::string tmpCampaignID = Campaigns.at(campKey).campaign_id;
					logger.log(LOGINFO, "CallProcedure called campaign_id : %s", tmpCampaignID.c_str());
					CallProcedure(tmpCampaignID);*/
					//retry or exhaust of base in alert table
					/*char OBDAlertMsg[256];
					sprintf_s(OBDAlertMsg, "Campaign-%s base is either in retry or exhausted", Campaigns.at(campKey).campaign_name);
					LogOBDAlerts(OBDAlertMsg);*/
					logger.log(LOGINFO, "CallProcedure called campaign_id : %s", Campaigns.at(campKey).campaign_id);
					CWinThread* procThread = AfxBeginThread(CallProcedure, new DeallocateProcParam{ this, Campaigns.at(campKey).campaign_id });
				}
			}
#if COMMENTED_SECTION
			//copying circle and zone to the global variables
			/*if (!StrCmpA(circle, "") && !StrCmpA(zone, ""))
			{
			//StrCpyA(circle, row[5]);
			StrCpyA(zone, row[6]);
			}*/
			/*for (size_t i = 1; i <= Campaigns.size(); i++)
			{*/
			//if (Campaigns.at(campKey).obdDialPlan != Informative)
			//{
			//	char tempPath[100];
			//	char fileName[16];
			//	int j = 1;
			//	StrCpyA(tempPath, "");
			//	//Campaigns.at(i).isCampaignCompleted = false;
			//	while (true)
			//	{
			//		StrCpyA(tempPath, Campaigns.at(campKey).promptsDirectory);
			//		sprintf_s(fileName, "%d.wav", j);
			//		StrCatA(tempPath, fileName);
			//		//logger.log(LOGINFO, "tempPath: %s", tempPath);
			//		if (PathFileExistsA(tempPath))
			//		{
			//			/*index = i * 10 + j;
			//			sprintf_s(alias, "alias%d", index);
			//			if (SsmLoadIndexData(index, alias, 7, tempPath, 0, -1) != 0)
			//			{
			//			CString errMsg;
			//			errMsg.Format(L"Load Index %d Error", index);
			//			AfxMessageBox(errMsg);
			//			PostQuitMessage(0);
			//			}*/
			//			StrCpyA(Campaigns.at(campKey).promptsPath[j], tempPath);
			//			//logger.log(LOGINFO, "Path: %s, index: %s, total Index: %d", tempPath, Campaigns.at(i).promptsPath[j], SsmGetTotalIndexSeg());
			//			j++;
			//		}
			//		else
			//		{
			//			break;
			//		}
			//	}//End While
			//}
			//update campaign status while it is in execution
			/*int query_state;
			char queryStr[256];
			sprintf_s(queryStr, "update tbl_campaign_master set execution_status=1,campaign_status=2 where campaign_id='%s' and execution_status=0", Campaigns.at(i).campaign_id);
			query_state = mysql_query(conn, queryStr);
			if (query_state != 0)
			{
			CString err(mysql_error(conn));
			AfxMessageBox(err);
			}*/

			//}//End For
#endif // COMMENTED_SECTION
		}
		mysql_free_result(res);
		logger.log(LOGINFO, "Total available campaigns: %d", Campaigns.size());
	}
	catch (CException* ex)
	{
		CRuntimeClass* curClass = ex->GetRuntimeClass();
		/*CString Cstr(curClass->m_lpszClassName);
		Cstr.Append(L" Exception found of type");
		*/
		logger.log(LOGERR, "Exception found of type in class: %s", ex->GetRuntimeClass());
		//PostQuitMessage(0);
	}
	catch (...)
	{
		logger.log(LOGERR, "Unhandled Exception occured in GetDBData()");
		//AfxMessageBox(L"");
		//PostQuitMessage(0);
	}
	return true;
}

BOOL CSpiceOBDDlg::UpdateDBData(/*int chState*/)
{
	//try {
	//	int query_state, chState = 1;
	//	char queryStr1[131071], queryStr2[131071];
	//	StrCpyA(queryStr1, "update tbl_outdialer_base set status =  ( case ");
	//	int countNumbers = 0;
	//	int lastIndex;
	//	for (int i = 0; i < nTotalCh; i++)
	//	{
	//		if (ChInfo[i].rowTobeUpdated)
	//		{
	//			char tempStr[1024];

	//			sprintf_s(tempStr, "when ani = '%s' then %d ", ChInfo[i].pPhoNumBuf, chState/*ChInfo[i].nStep*/); //TODO: comented section will be used
	//			StrCatA(queryStr1, tempStr);
	//			countNumbers++;
	//			lastIndex = i;
	//		}
	//	}
	//	StrCpyA(queryStr2, "end) where ani in(");
	//	int i = 0;
	//	for (i = 0; i < nTotalCh; i++)
	//	{
	//		if (ChInfo[i].rowTobeUpdated == true && countNumbers > 1)
	//		{
	//			char tempStr[1024];
	//			sprintf_s(tempStr, "'%s', ", ChInfo[i].pPhoNumBuf);
	//			StrCatA(queryStr2, tempStr);
	//			countNumbers--;
	//		}
	//	}

	//	//For the last one no comma required

	//	char tempStr[48];
	//	sprintf_s(tempStr, "'%s'", ChInfo[lastIndex].pPhoNumBuf);
	//	StrCatA(queryStr2, tempStr);



	//	StrCatA(queryStr2, ")");
	//	StrCatA(queryStr1, queryStr2);
	//	query_state = mysql_query(conn, queryStr1);

	//	logger.log(LOGINFO, queryStr1);

	//	if (query_state != 0)
	//	{
	//		CString err(mysql_error(conn));
	//		AfxMessageBox(err);
	//		return false;
	//	}
	//}
	//catch (CString msg)
	//{
	//	AfxMessageBox(msg);
	//}
	return true;
}

void CSpiceOBDDlg::GetNextUserData()
{
	//int query_state;
	//char queryStr[256];

	////sprintf_s(queryStr, "select * from obdtest where chstate = %d limit %d; update obdtest  set chstate = %d where chstate = %d limit %d", USER_IDLE, getAndUpdateRowCount, USER_IDLE + 2, USER_IDLE, getAndUpdateRowCount);
	//sprintf_s(queryStr, "select * from obdtest where ChState = %d  limit %d", USER_IDLE, getAndUpdateRowCount);

	//OffSet = OffSet + getAndUpdateRowCount;
	//query_state = mysql_query(conn, queryStr);

	//if (query_state != 0)
	//{
	//	CString err(mysql_error(conn));
	//	AfxMessageBox(err);
	//}

	//res = mysql_store_result(conn);
	//i = 0;
	//while ((row = mysql_fetch_row(res)) != NULL)
	//{
	//	if (ChInfo[i].EnCalled == true)
	//	{
	//		CString tempStr1(row[0]), tempStr2(row[1]);

	//		/*m_TrkChList.SetItemText(i, 1, L"VasApps/SPC_CP_DT_81...");
	//		m_TrkChList.SetItemText(i, 2, L"Alerting");*/
	//		m_TrkChList.SetItemText(i, 3, tempStr1);
	//		m_TrkChList.SetItemText(i, 4, tempStr2);
	//		//m_TrkChList.SetItemText(i, 5, L"");
	//		m_TrkChList.SetItemText(i, 6, L"Idle");
	//		ChInfo[i].nStep = USER_IDLE;
	//		ChInfo[i].EnCalled = false;
	//		StrCpyA(ChInfo[i].pPhoNumBuf, row[1]);
	//	}
	//	i++;
	//}
}

void CSpiceOBDDlg::UpDateATrunkChListCtrl()
{
	CString state;
	wchar_t tmpstr[51];
	int i, nIndex;
	logger.log(LOGINFO, "UpDateATrunkChListCtrl Start");

	for (i = 0, nIndex = 0; i < nTotalCh; i++)
	{
		if (!ChInfo[i].InUse && ++ChInfo[i].InUseCount < 5)
		{
			nIndex++;
			continue;
		}
		else
		{
			ChInfo[i].InUseCount = 0;
		}
		//if (SsmGetChType(i) != 2) continue;
		switch (SsmGetChState(i)) {
		case S_CALL_STANDBY:				state = "S_CALL_STANDBY"; break;
		case S_CALL_PICKUPED:				state = "S_CALL_PICKUPED"; break;
		case S_CALL_RINGING:				state = "S_CALL_RINGING"; break;
		case S_CALL_TALKING:				state = "S_CALL_TALKING"; break;
		case S_CALL_ANALOG_WAITDIALTONE:	state = "S_CALL_ANALOG_WAITDIALTONE";	break;
		case S_CALL_ANALOG_TXPHONUM:		state = "S_CALL_ANALOG_TXPHONUM"; break;
		case S_CALL_ANALOG_WAITDIALRESULT:	state = "S_CALL_ANALOG_WAITDIALRESULT";		break;
		case S_CALL_PENDING:				state = "S_CALL_PENDING"; break;
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

	if (isDeallocateProcedureCalled) //don't insert any record/update reason till base retry is completed. 
		return;

	if (IsUpdate)
	{
		UpdateStatusAndPickNextRecords();
		//nextCallGapDuration = GetPrivateProfileIntA("Database", "nextcallgapduration", 1, InitDBSettings);
		//Sleep(nextCallGapDuration);
	}
	//	char queryStr[256];
	//	int query_state;
	////Loop to pick next records for all campaigns.
	//for (size_t tmpCmpId = 1; tmpCmpId <= Campaigns.size(); tmpCmpId++)
	//{
	//	//int tmpCmpId = ChInfo[i].CampaignID;
	//	//if (tmpCmpId != -1)
	//	//{
	//	if ((Campaigns.at(tmpCmpId).phnumBuf.size() <= (2 * Campaigns.at(tmpCmpId).channelsAllocated))/* && !Campaigns.at(tmpCmpId).hasReachedThreshold*/)
	//	{
	if (++loopCountForCampaignUpdate >= 5) //to lower the frequency of dip into tbl_campaign_master
	{
		GetDBData();
		loopCountForCampaignUpdate = 0;
	}
	for (int ch = 0; ch < nTotalCh; ch++)
	{
		if (ChInfo[ch].isAvailable && !ChInfo[ch].isIVRChannel)
		{
			ChInfo[ch].CampaignID = -1;
			for (size_t j = 1; j <= Campaigns.size(); j++)
			{
				if (ch >= Campaigns.at(j).minCh && ch <= Campaigns.at(j).maxCh)
				{
					ChInfo[ch].CampaignID = j;
				}
			}
		}
		//logger.log(LOGINFO, "changed ch no: %d, campaign Id: %d", ch, ChInfo[ch].CampaignID);
	}
# if COMMENTED_SECTION
	////check if no more number to be dialed and all channels are free for specific campaign
	//if (!Campaigns.at(tmpCmpId).isCampaignCompleted && Campaigns.at(tmpCmpId).phnumBuf.empty() && isCampaignChannelsCleared(tmpCmpId))
	//{
	//	Campaigns.at(tmpCmpId).isCampaignCompleted = true;
	//	//sprintf_s(queryStr, "update tbl_campaign_master set campaign_status=3 where campaign_id='%s'", Campaigns.at(tmpCmpId).campaign_id);
	//	sprintf_s(queryStr, "call procDeallocateChannel('%s', )", Campaigns.at(tmpCmpId).campaign_id);
	//	query_state = mysql_query(conn, queryStr);

	//	if (query_state != 0)
	//	{
	//		CString err(mysql_error(conn));
	//		AfxMessageBox(err);
	//	}
	//	/*if (mysql_affected_rows(conn) >= 1)
	//	{
	//		logger.log(LOGINFO, "Campaign : '%s' completed !!!", Campaigns.at(tmpCmpId).campaign_id);
	//	}*/
	//}
	/*}
	}*/
	//		sprintf_s(queryStr, "select ani from tbl_outdialer_base where campaign_id = '%s' and status = %d order by priority,insert_date_time limit %d",
	//			Campaigns.at(tmpCmpId).campaign_id, 0, (5 * Campaigns.at(tmpCmpId).channelsAllocated));

	//		query_state = mysql_query(conn, queryStr);

	//		if (query_state != 0)
	//		{
	//			CString err(mysql_error(conn));
	//			AfxMessageBox(err);
	//		}
	//		MYSQL_RES * resPhBuf = mysql_store_result(conn);
	//		MYSQL_ROW rowPhBuf;
	//		Campaigns.at(tmpCmpId).phnumBuf.clear();
	//		while ((rowPhBuf = mysql_fetch_row(resPhBuf)) != NULL)
	//		{
	//			char* DecryptedVal = aesEncryption.DecodeAndDecrypt(rowPhBuf[0]);
	//			Campaigns.at(tmpCmpId).phnumBuf.push_back({ DecryptedVal, rowPhBuf[0] });
	//		}
	//}

	//	/*if (!Campaigns.at(tmpCmpId).phnumBuf.empty())
	//	{
	//	CString tempStr2(Campaigns.at(tmpCmpId).phnumBuf.front().ani);
	//	StrCpyA(ChInfo[i].pPhoNumBuf, Campaigns.at(tmpCmpId).phnumBuf.front().ani);
	//	StrCpyA(ChInfo[i].CDRStatus.encrypted_ani, Campaigns.at(tmpCmpId).phnumBuf.front().encryptedAni);
	//	m_TrkChList.SetItemText(i, 4, tempStr2);
	//	Campaigns.at(tmpCmpId).phnumBuf.erase(Campaigns.at(tmpCmpId).phnumBuf.begin());
	//	logger.log(LOGINFO, "UpdateStatusAndPickNextRecords Update data Ani : %s, Encrypted Ani: %s, Channel Num: %d", ChInfo[i].pPhoNumBuf, ChInfo[i].CDRStatus.encrypted_ani, i);
	//	//logger.log(LOGINFO, "UpdateStatusAndPickNextRecords vector current size: %d for campaign Id: %d", Campaigns.at(tmpCmpId).phnumBuf.size(), tmpCmpId);
	//	if (!UpdatePhNumbersStatus(i))
	//	{
	//	logger.log(LOGINFO,"UpdateStatusAndPickNextRecords Row not updated...ph number: %s, channel number: %d", ChInfo[i].pPhoNumBuf, i);
	//	}
	//	}
	//	else
	//	{
	//	StrCpyA(ChInfo[i].pPhoNumBuf, "");
	//	StrCpyA(ChInfo[i].CDRStatus.encrypted_ani, "");
	//	m_TrkChList.SetItemText(i, 4, L"");
	//	}*/
	//	//}
	//}
#endif //COMMENTED_SECTION
	logger.log(LOGINFO, "UpDateATrunkChListCtrl End");
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
			tm dateTime = logger.getTime(std::string());
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

			if (!StrCmpA(ChInfo[i].CDRStatus.dtmfBuf, ""))
			{
				StrCpyA(ChInfo[i].CDRStatus.dtmfBuf, ChInfo[i].CDRStatus.dtmf);
				StrCatA(ChInfo[i].CDRStatus.dtmfBuf, "|");
				StrCatA(ChInfo[i].CDRStatus.dtmfBuf, ChInfo[i].CDRStatus.dtmf2);
			}
			if (!StrCmpA(ChInfo[i].CDRStatus.DNISBuf, ""))
			{
				StrCpyA(ChInfo[i].CDRStatus.DNISBuf, ChInfo[i].CDRStatus.DNIS);
			}

			outfile << Campaigns.at(tmpCmpId).campaign_name << "#" << systemIpAddr << "#" << ChInfo[i].CDRStatus.ani << timeVal
				<< "#" << ChInfo[i].CDRStatus.channel << "#" << "" << "#" << Campaigns.at(tmpCmpId).campaign_id << "#" << circle
				<< "#" << dateVal << "#" << ChInfo[i].CDRStatus.ani << "#" << ChInfo[i].CDRStatus.cli << "#"
				<< call_time << "#" << answer_time << "#" << end_time << "#" << ChInfo[i].CDRStatus.callPatch_duration
				<< "#" << ChInfo[i].CDRStatus.answer_duration << "#" << ChInfo[i].CDRStatus.total_duration << "#" << ChInfo[i].CDRStatus.reason_code
				<< "#" << ChInfo[i].CDRStatus.reason << "#" << "" << "#" << ChInfo[i].CDRStatus.encrypted_ani << "#" << ChInfo[i].CDRStatus.dtmfBuf << "#" << "" << "#"
				<< ChInfo[i].CDRStatus.status << "#" << ChInfo[i].CDRStatus.songName << "#" << ChInfo[i].CDRStatus.DNISBuf << "#" << endl;

			//Check if CDR file is greater than 10 MB make a new one.
			/*	if (outfile.tellp() >= 100)
			{
			outfile.close();
			openCDRLogFile();
			}*/

			if (!UpdateReasonInDialerBase(i))
			{
				logger.log(LOGINFO, "Reason not updated...ph number: %s, encrypted ani: %s", ChInfo[i].pPhoNumBuf, ChInfo[i].CDRStatus.encrypted_ani);
			}

			if (StrCmpIA(ChInfo[i].CDRStatus.reason, "Answered") == 0)
			{
				//Insert Call records in DB
				sprintf_s(queryStrInsert, "INSERT INTO tbl_obd_answered_calls(channel, campaign_id, campaign_name, circle, ani, cli, dtmf, answer_duration, status, ring_duration, call_date, call_time, answer_time, end_time, reason_code,total_duration,reason,encrypted_ani, call_id, retry_status ,song_name, patch_dnis) \
				VALUES(%d, '%s', '%s', '%s', '%s', '%s', '%s', %d, %d, %d, '%s', '%s', '%s', '%s', '%s', '%d', '%s','%s', '%s%s', '%d', '%s', '%s')",
					ChInfo[i].CDRStatus.channel, Campaigns.at(tmpCmpId).campaign_id, Campaigns.at(tmpCmpId).campaign_name, circle, ChInfo[i].CDRStatus.ani, ChInfo[i].CDRStatus.cli, ChInfo[i].CDRStatus.dtmfBuf, ChInfo[i].CDRStatus.answer_duration,
					StrCmpA(ChInfo[i].CDRStatus.status, "SUCCESS") == 0 ? 1 : 0, ChInfo[i].CDRStatus.callPatch_duration, dateVal, call_time, answer_time, end_time, ChInfo[i].CDRStatus.reason_code,
					ChInfo[i].CDRStatus.total_duration, ChInfo[i].CDRStatus.reason, ChInfo[i].CDRStatus.encrypted_ani, ChInfo[i].CDRStatus.ani, timeVal, Campaigns.at(tmpCmpId).curRetryCount, ChInfo[i].CDRStatus.songName, ChInfo[i].CDRStatus.DNISBuf);
				logger.log(LOGINFO, queryStrInsert);
				query_state = mysql_query(connInsert, queryStrInsert);
				if (query_state != 0)
				{
					CString err(mysql_error(connInsert));
					AfxMessageBox(err);
				}
			}
			//else
			//{
			sprintf_s(queryStrInsert, "INSERT INTO tbl_obd_calls(channel, campaign_id, campaign_name, circle, ani, cli, dtmf, answer_duration, status, ring_duration, call_date, call_time, answer_time, end_time, reason_code,total_duration,reason,encrypted_ani, call_id, retry_status ,song_name, patch_dnis) \
			VALUES(%d, '%s', '%s', '%s', '%s', '%s', '%s', %d, %d, %d, '%s', '%s', '%s', '%s', '%s', '%d', '%s','%s', '%s%s', '%d', '%s', '%s')",
				ChInfo[i].CDRStatus.channel, Campaigns.at(tmpCmpId).campaign_id, Campaigns.at(tmpCmpId).campaign_name, circle, ChInfo[i].CDRStatus.ani, ChInfo[i].CDRStatus.cli, ChInfo[i].CDRStatus.dtmfBuf, ChInfo[i].CDRStatus.answer_duration,
				StrCmpA(ChInfo[i].CDRStatus.status, "SUCCESS") == 0 ? 1 : 0, ChInfo[i].CDRStatus.callPatch_duration, dateVal, call_time, answer_time, end_time, ChInfo[i].CDRStatus.reason_code,
				ChInfo[i].CDRStatus.total_duration, ChInfo[i].CDRStatus.reason, ChInfo[i].CDRStatus.encrypted_ani, ChInfo[i].CDRStatus.ani, timeVal, Campaigns.at(tmpCmpId).curRetryCount, ChInfo[i].CDRStatus.songName, ChInfo[i].CDRStatus.DNISBuf);
			//}
			logger.log(LOGINFO, queryStrInsert);
			query_state = mysql_query(connInsert, queryStrInsert);
			if (query_state != 0)
			{
				CString err(mysql_error(connInsert));
				AfxMessageBox(err);
			}
			//logging correct consent recieved
			if (strlen(ChInfo[i].CDRStatus.dtmfBuf) >= 3)
			{
				ConsentFile << circle << "#" << systemIpAddr << "#" << dateVal << "#" << timeVal << "#" << ChInfo[i].CDRStatus.cli << "#" << ChInfo[i].pPhoNumBuf
					<< "#" << ChInfo[i].CDRStatus.dtmfBuf << "#" << ChInfo[i].CDRStatus.DNISBuf << "#" << Campaigns.at(tmpCmpId).campaign_id << "#" << endl;
			}
			/*if (Campaigns.at(tmpCmpId).obdDialPlan == AcquisitionalOBDWith1stConsent && StrCmpA(ChInfo[i].CDRStatus.firstConsent, ""))
			{
			ConsentFile << circle << "#" << systemIpAddr << "#" << dateVal << "#" << timeVal << "#" << ChInfo[i].CDRStatus.cli << "#" << ChInfo[i].pPhoNumBuf
			<< "#" << ChInfo[i].CDRStatus.firstConsent << "#" << Campaigns.at(tmpCmpId).campaign_id << "#" << endl;
			}*/
			StrCpyA(ChInfo[i].CDRStatus.dtmf, EMPTY_STRING);
			StrCpyA(ChInfo[i].CDRStatus.dtmf2, EMPTY_STRING);
			StrCpyA(ChInfo[i].CDRStatus.firstConsent, EMPTY_STRING);
			StrCpyA(ChInfo[i].CDRStatus.secondConsent, EMPTY_STRING);
			StrCpyA(ChInfo[i].CDRStatus.reason, EMPTY_STRING);
			StrCpyA(ChInfo[i].CDRStatus.status, EMPTY_STRING);
			StrCpyA(ChInfo[i].CDRStatus.reason_code, EMPTY_STRING);
			StrCpyA(ChInfo[i].CDRStatus.songName, EMPTY_STRING);
			StrCpyA(ChInfo[i].CDRStatus.DNIS, EMPTY_STRING);
			StrCpyA(ChInfo[i].CDRStatus.dtmfBuf, EMPTY_STRING);
			StrCpyA(ChInfo[i].CDRStatus.DNISBuf, EMPTY_STRING);
			StrCpyA(ChInfo[i].promptsName, EMPTY_STRING);
			StrCpyA(ChInfo[i].pPhoNumBuf, EMPTY_STRING);
			StrCpyA(ChInfo[i].CDRStatus.encrypted_ani, EMPTY_STRING);
			ChInfo[i].isAvailable = true;
		}
	}//For loop
}

//Update ph number status in table...
BOOL CSpiceOBDDlg::UpdatePhNumbersStatus(const char* campaignId, const char* encryptedAni)
{
	char queryStr[1024];

	sprintf_s(queryStr, "update tbl_outdialer_base set status = 1 where campaign_id = '%s' and ani = '%s'", campaignId, encryptedAni);
	logger.log(LOGINFO, "UpdatePhNumbersStatus query string: %s", queryStr);
	int query_state = mysql_query(connUpdate, queryStr);

	if (query_state != 0)
	{
		CString err(mysql_error(connUpdate));
		AfxMessageBox(err);
	}
	if (mysql_affected_rows(connUpdate) >= 1)
	{
		return true;
	}
	else
	{
		sprintf_s(queryStr, "update tbl_outdialer_base set status = 1 where ani = '%s'", encryptedAni);
		//logger.log(LOGINFO, "update query for phone number: %s, Encrypted Ani: %s, Channel Num:%d", ChInfo[ch].pPhoNumBuf, ChInfo[ch].CDRStatus.encrypted_ani, ch);
		int query_state = mysql_query(connUpdate, queryStr);

		if (query_state != 0)
		{
			CString err(mysql_error(connUpdate));
			AfxMessageBox(err);
		}
		if (mysql_affected_rows(connUpdate) >= 1)
		{
			return true;
		}
	}
	return false;
}

//Update record in table...
BOOL CSpiceOBDDlg::UpdateReasonInDialerBase(int ch)
{
	char queryStr[1024];

	sprintf_s(queryStr, "update tbl_outdialer_base set reason = '%s' where campaign_id = '%s' and ani = '%s'",
		ChInfo[ch].CDRStatus.reason, Campaigns.at(ChInfo[ch].CampaignID).campaign_id, ChInfo[ch].CDRStatus.encrypted_ani);
	logger.log(LOGINFO, "update query for phone number: %s, Encrypted Ani: %s, Channel Num:%d", ChInfo[ch].pPhoNumBuf, ChInfo[ch].CDRStatus.encrypted_ani, ch);
	int query_state = mysql_query(connUpdate, queryStr);

	if (query_state != 0)
	{
		CString err(mysql_error(connUpdate));
		AfxMessageBox(err);
	}
	if (mysql_affected_rows(connUpdate) >= 1)
	{
		return true;
	}
	else
	{
		sprintf_s(queryStr, "update tbl_outdialer_base set reason = '%s' where ani = '%s'",
			ChInfo[ch].CDRStatus.reason, ChInfo[ch].CDRStatus.encrypted_ani);
		//logger.log(LOGINFO, "update query for phone number: %s, Encrypted Ani: %s, Channel Num:%d", ChInfo[ch].pPhoNumBuf, ChInfo[ch].CDRStatus.encrypted_ani, ch);
		int query_state = mysql_query(connUpdate, queryStr);

		if (query_state != 0)
		{
			CString err(mysql_error(connUpdate));
			AfxMessageBox(err);
		}
		if (mysql_affected_rows(connUpdate) >= 1)
		{
			return true;
		}
	}
	logger.log(LOGERR, "UpdateReasonInDialerBase Number update failed: %s, query string: %s, channel Num: %d", ChInfo[ch].pPhoNumBuf, queryStr, ch);
	return false;
}


//checks if all channels are cleared (dialled call are finished) for the perticular campaign
BOOL CSpiceOBDDlg::isCampaignChannelsCleared(int campaignKey)
{
	if (Campaigns.at(campaignKey).channelsAllocated == 0)
	{
		return true;
	}
	for (int chNo = Campaigns.at(campaignKey).minCh; chNo <= Campaigns.at(campaignKey).maxCh; chNo++)
	{
		if (StrCmpA(ChInfo[chNo].pPhoNumBuf, ""))
		{
			return false;
		}
	}
	return true;
}

void CSpiceOBDDlg::CloseDBConn()
{
	// Close a MySQL connection
	mysql_free_result(res);
	if (conn != NULL)
	{
		mysql_close(conn);
		conn = NULL;
	}
	if (connCallProc != NULL)
	{
		mysql_close(connCallProc);
		connCallProc = NULL;
	}
	if (connBase != NULL)
	{
		mysql_close(connBase);
		connBase = NULL;
	}
	if (connSelect != NULL)
	{
		mysql_close(connSelect);
		connSelect = NULL;
	}
	if (connInsert != NULL)
	{
		mysql_close(connInsert);
		connInsert = NULL;
	}
	if (connUpdate != NULL)
	{
		mysql_close(connUpdate);
		connUpdate = NULL;
	}
	if (connPort != NULL)
	{
		mysql_close(connPort);
		connPort = NULL;
	}
}



void CSpiceOBDDlg::InitUserDialingList()
{
	static int ColumnWidth[7] = { 50, 150, 50, 100, 160, 80 ,650 }; //earlier it was: { 50, 50, 50, 200, 200, 50 ,750 };
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

int CSpiceOBDDlg::GetAnIdleChannel(BOOL isContest) // Find an idle trunk channel for IVR call Patchup
{
	int i;
	//contestMinCh, contestMaxCh, tmpContestMinCh;
	if (isContest)
	{
		for (i = tmpContestMinCh; i < contestMaxCh; i++)
		{
			if (SsmGetChState(i) == S_CALL_STANDBY)
			{
				if (tmpContestMinCh < contestMaxCh - 1)
				{
					tmpContestMinCh = i + 1;
				}
				else
				{
					tmpContestMinCh = contestMinCh;
				}
				break;
			}
		}
		if (i >= contestMaxCh)
		{
			tmpContestMinCh = contestMinCh;
			return -1;
		}
	}
	else
	{
		for (i = tempIVRMinCh; i < nIVRMaxChNew; i++)
		{
			if (SsmGetChState(i) == S_CALL_STANDBY)
			{
				if (tempIVRMinCh < nIVRMaxChNew - 1)
				{
					tempIVRMinCh = i + 1;
				}
				else
				{
					tempIVRMinCh = nIVRMinChNew;
				}
				break;
			}
		}
		if (i >= nIVRMaxChNew)
		{
			tempIVRMinCh = nIVRMinChNew;
			return -1;
		}
	}
	return i;
}

void CSpiceOBDDlg::getErrorResult(LPCTSTR  ApiName)
{
	char errStr[256];
	SsmGetLastErrMsg(errStr);
	//CString str(errStr);
	//str.Append(L"- ");
	//str.Append(ApiName);
	logger.log(LOGERR, "%s", errStr);
	//AfxMessageBox(str);
	/*SsmCloseCti();
	PostQuitMessage(0);*/
}

void CSpiceOBDDlg::LogErrorCodeAndMessage(int ch)
{
	sprintf_s(ChInfo[ch].CDRStatus.reason_code, "%d", SsmGetLastErrCode()); //get error code
	SsmGetLastErrMsg(ChInfo[ch].CDRStatus.reason); //get dial failure error message
	logger.log(LOGINFO, "Error code: %s And Error Reason: %s on channel Number: %d , phone number: %s", ChInfo[ch].CDRStatus.reason_code, ChInfo[ch].CDRStatus.reason, ch, ChInfo[ch].pPhoNumBuf);
}

UINT CSpiceOBDDlg::ThreadProcApiCall(LPVOID threadParam)
{
	while (true)
	{
		ApiHttpURL apiURL;
		std::string tmpUrl = apiURL.PopFrontURL();
		if (!tmpUrl.empty())
		{
			CURLcode result = apiURL.callApi(tmpUrl.c_str());
			((CLogger*)threadParam)->log(LOGINFO, "PrepareAndHitURLApi response : %s", ApiHttpURL::readBuffer.c_str());
		}
		Sleep(100);
	}
}

void CSpiceOBDDlg::PrepareAndHitURLApi(int ch)
{
	if (ChInfo[ch].isApiToBeCalled)
	{
		logger.log(LOGINFO, "PrepareAndHitURLApi start");
		ChInfo[ch].isApiToBeCalled = false;
		char * dnisBuffer = ChInfo[ch].CDRStatus.DNISBuf;
		const char* pipeDelim = "|";
		char* context;
		std::string lastPatchDnis("");
		char productCode[10];
		char promoCode[20];

		dnisBuffer = strtok_s(dnisBuffer, pipeDelim, &context);
		while (dnisBuffer)
		{
			lastPatchDnis = dnisBuffer;
			dnisBuffer = strtok_s(NULL, pipeDelim, &context);
		}
		StrCpyA(promoCode, "0");
		if (lastPatchDnis.empty())
		{
			StrCpyA(productCode, Campaigns.at(ChInfo[ch].CampaignID).cgShortCode);
		}
		else if (lastPatchDnis.length() > 8)
		{
			StrCpyA(productCode, lastPatchDnis.substr(8, 4).c_str());
			if (lastPatchDnis.length() > 12)
			{
				StrCpyA(promoCode, lastPatchDnis.substr(12, 7).c_str());
			}
		}
		char prepareUrlStr[1024];
		sprintf_s(prepareUrlStr, "http://172.27.32.46/SynwayObd_API/obdSmsApi?msisdn=%s&circle=%s&civrId=%s&promoCode=%s",
			ChInfo[ch].pPhoNumBuf, circle, productCode, promoCode);
		logger.log(LOGINFO, "PrepareAndHitURLApi prepared URL : %s", prepareUrlStr);
		ApiHttpURL apiURL;
		apiURL.PushBackURL(prepareUrlStr);
	}
}

void CSpiceOBDDlg::HangupCall(int ch)
{
	//Addional logging done by sandeep rajan - 24th May, 2017 to check the flow
	try
	{
		GetDTMFandDNISBuffer(ch);
		if (IsSMSApiEnabled)
		{
			PrepareAndHitURLApi(ch);
		}
		logger.log(LOGINFO, "Hangup Called for channel: %d, outbound IVRChannelNumber: %d", ch, ChInfo[ch].IVRChannelNumber);
		if (ChInfo[ch].IVRChannelNumber != -1)
		{
			char errReason[100];
			int code = SsmStopTalkWith(ChInfo[ch].IVRChannelNumber, ch); SsmGetLastErrMsg(errReason);
			logger.log(LOGINFO, "Return code for SsmStopTalkWith(%d,%d) = %d reason = %s", ch, ChInfo[ch].IVRChannelNumber, code, errReason); //Addional logging done by sandeep rajan - 24th May, 2017 to check the flow
			if (code != -1)
			{
				code = -1;
				code = SsmHangup(ChInfo[ch].IVRChannelNumber); SsmGetLastErrMsg(errReason);
				logger.log(LOGINFO, "Return code for SsmHangup(%d) = %d reason = %s", ChInfo[ch].IVRChannelNumber, code, errReason); //Addional logging done by sandeep rajan - 24th May, 2017 to check the flow
																																	 //ChInfo[ChInfo[ch].IVRChannelNumber].InUse = false;
				m_TrkChList.SetItemText(ChInfo[ch].IVRChannelNumber, 1, L"");
				m_TrkChList.SetItemText(ChInfo[ch].IVRChannelNumber, 3, L"");
				m_TrkChList.SetItemText(ChInfo[ch].IVRChannelNumber, 4, L"");
				ChInfo[ch].IVRChannelNumber = -1;
			}
			if (!StrCmpA(ChInfo[ch].CDRStatus.dtmf2, ""))
			{
				StrCpyA(ChInfo[ch].CDRStatus.dtmf2, SsmGetDtmfStrA(ch));
			}
		}
		SsmClearRxDtmfBuf(ch);
		SsmStopPlay(ch);
		SsmHangup(ch);
		ChInfo[ch].EnCalled = true;
		IsUpdate = true;
		ChInfo[ch].nStep = USER_IDLE;
		if (StrCmpA(ChInfo[ch].CDRStatus.status, "") == 0) StrCpyA(ChInfo[ch].CDRStatus.status, "FAIL");
		if (StrCmpA(ChInfo[ch].CDRStatus.reason, "") == 0) { char errReason[100]; SsmGetLastErrMsg(errReason); StrCpyA(ChInfo[ch].CDRStatus.reason, errReason); }
		if (StrCmpA(ChInfo[ch].CDRStatus.reason_code, "") == 0) { char errorcode[8]; sprintf_s(errorcode, "%d", SsmGetLastErrCode()); StrCpyA(ChInfo[ch].CDRStatus.reason_code, errorcode); }
	}
	catch (...)
	{
		logger.log(LOGERR, "Exception Unhandled in Hangup call");
	}
}

void CSpiceOBDDlg::HangupIVRCall(int ch)
{
	if (ch >= 0) {
		char errReason[100];
		int code = SsmHangup(ch); SsmGetLastErrMsg(errReason);
		//Addional logging done by sandeep rajan - 24th May, 2017 to check the flow
		logger.log(LOGINFO, "Return code for SsmHangup(%d) = %d reason = %s", ch, code, errReason);
		//Added by sandeep rajan - 24th May, 2017 to clear the CG ANI and BNI columns after call completes on CG channels.
		try {
			m_TrkChList.SetItemText(ch, 1, L"");
			m_TrkChList.SetItemText(ch, 3, L"");
			m_TrkChList.SetItemText(ch, 4, L"");
		}
		catch (...) {
			logger.log(LOGERR, "Error in clearing the columns of the outgoing channel %d", ch);
		}
		//ChInfo[ch].InUse = false;
	}
	else logger.log(LOGERR, "Got invalid channel %d for HangupIVRCall", ch);
}

char* CSpiceOBDDlg::GetReleaseErrorReason(WORD errorCode)
{
	switch (errorCode)
	{
	case 0: return "Unknown reason"; break;
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


UINT CSpiceOBDDlg::SetChannelsStateCount(LPVOID  chCount)
{
	int totCount = SsmGetMaxCh();
	vector<int> downChannels;
	if (totCount > 0)
	{
		CString totChstr;
		totChstr.Format(_T("%d"), totCount);
		totalChannelsAvlCtrl.SetWindowTextW(totChstr);
	}

	ChCount* chnlCount = (ChCount*)chCount;
	while (true)
	{
		Sleep(2000);

		int callsDialling = 0, callsConnected = 0, callsPatchedUp = 0, channelsDown = 0;
		CString DailStr, ConnStr, CgStr, chDownStr, downChannelsStr("Range: (");
		int start, end;  // track start and end

		vector<int>().swap(downChannels);

		for (int i = 0; i < chnlCount->nTotalCh; i++)
		{
			int chState = SsmGetChState(i);
			if (chState == S_CALL_TALKING)
			{
				if (i >= chnlCount->nIVRMinCh && i <= chnlCount->nIVRMaxCh)
				{
					callsPatchedUp++;
				}
				else
				{
					callsConnected++;
				}
			}
			else if (chState == S_CALL_WAIT_REMOTE_PICKUP || chState == S_ISUP_WaitDialAnswer)
			{
				callsDialling++;
			}
			else if (chState == S_CALL_UNAVAILABLE || chState == S_ISUP_RemotelyBlocked || chState == S_ISUP_WaitReset)
			{
				downChannels.push_back(i);
				channelsDown++;
			}
		}

		DailStr.Format(_T("%d"), callsDialling);
		dailingValCtrl.SetWindowTextW(DailStr);

		ConnStr.Format(_T("%d"), callsConnected);
		connctedValCtrl.SetWindowTextW(ConnStr);

		CgStr.Format(_T("%d"), callsPatchedUp);
		cgValCtrl.SetWindowTextW(CgStr);

		chDownStr.Format(_T("%d"), channelsDown);
		nChDownCtrl.SetWindowTextW(chDownStr);

		if (channelsDown > 0) //show vector element in groups
		{
			end = start = downChannels[0];
			for (int i = 1; i < downChannels.size(); i++)
			{
				// as long as entries are consecutive, move end forward
				if (downChannels[i] == (downChannels[i - 1] + 1))
				{
					end = downChannels[i];
				}
				else
				{
					// when no longer consecutive, add group to result
					if (start == end)
						downChannelsStr.AppendFormat(_T("%d, "), start);
					else
						downChannelsStr.AppendFormat(_T("%d-%d, "), start, end);

					start = end = downChannels[i];
				}
			}
			// handle the final group
			if (start == end)
				downChannelsStr.AppendFormat(_T("%d)"), start);
			else
				downChannelsStr.AppendFormat(_T("%d-%d)"), start, end);

			mChDownRangeVal.SetWindowTextW(downChannelsStr);
		}
		else
		{
			mChDownRangeVal.SetWindowTextW(L"");
		}
	}
}

void CSpiceOBDDlg::GetDTMFandDNISBuffer(int ch)
{
	if (StrCmpA(ChInfo[ch].CDRStatus.dtmf2, "") == 0) //check if empty
	{
		StrCpyA(ChInfo[ch].CDRStatus.dtmf2, SsmGetDtmfStrA(ch));
	}
	SsmClearRxDtmfBuf(ch);
	if (Campaigns.at(ChInfo[ch].CampaignID).enableSMSFlag && ((StrCmpA(ChInfo[ch].CDRStatus.dtmf2, "") == 0 && StrCmpA(ChInfo[ch].CDRStatus.DNIS, "")) ||
		(Campaigns.at(ChInfo[ch].CampaignID).obdDialPlan == Informative && StrCmpA(ChInfo[ch].CDRStatus.reason, "Answered") == 0)))
	{
		ChInfo[ch].isApiToBeCalled = true;
	}
	if (StrCmpA(ChInfo[ch].CDRStatus.dtmf, "") || StrCmpA(ChInfo[ch].CDRStatus.dtmf2, "")) //if either 1st consent or patch dtmf not empty
	{
		StrCatA(ChInfo[ch].CDRStatus.dtmfBuf, ChInfo[ch].CDRStatus.dtmf);
		StrCatA(ChInfo[ch].CDRStatus.dtmfBuf, "|");
		StrCpyA(ChInfo[ch].CDRStatus.dtmf, "");
		StrCatA(ChInfo[ch].CDRStatus.dtmfBuf, ChInfo[ch].CDRStatus.dtmf2);
		StrCatA(ChInfo[ch].CDRStatus.dtmfBuf, "$");
		StrCpyA(ChInfo[ch].CDRStatus.dtmf2, "");
	}
	if (StrCmpA(ChInfo[ch].CDRStatus.DNIS, ""))
	{
		StrCatA(ChInfo[ch].CDRStatus.DNISBuf, ChInfo[ch].CDRStatus.DNIS);
		StrCatA(ChInfo[ch].CDRStatus.DNISBuf, "|");
		StrCpyA(ChInfo[ch].CDRStatus.DNIS, "");
	}
}

int CSpiceOBDDlg::PlayMediaFile(int ch, int promptsNumber)
{
	char tmpMediaFile[31], tmpMediaPath[100];
	StrCpyA(tmpMediaPath, Campaigns.at(ChInfo[ch].CampaignID).promptsDirectory);

	if (promptsNumber)
	{
		sprintf_s(tmpMediaFile, "%d.wav", promptsNumber);
	}
	else
	{
		sprintf_s(tmpMediaFile, "%s.wav", ChInfo[ch].promptsName);
	}

	StrCatA(tmpMediaPath, tmpMediaFile);

	logger.log(LOGINFO, "Media file Path to be played: %s", tmpMediaPath);

	if (PathFileExistsA(tmpMediaPath))
	{
		if (SsmPlayFile(ch, tmpMediaPath, -1, 0, -1) == -1)
		{
			return -1;
		}
		CString promptName(tmpMediaFile);
		m_TrkChList.SetItemText(ch, 5, promptName);
	}
	else
	{
		CString promptsFileName(tmpMediaFile);
		logger.log(LOGERR, "%s file missing on path", tmpMediaPath);
		promptsFileName.Append(_T(" Prompt Missing!!!"));
		AfxMessageBox(promptsFileName);
		OnBnClickedDiallingStop();
	}
	return 0;
}

void CSpiceOBDDlg::ContinuePlayingPrompts(int ch)
{
	int pnFormat; long pnTime;
	if (SsmGetPlayingFileInfo(ch, &pnFormat, &pnTime) == 0)
	{
		long playedTime = SsmGetPlayedTime(ch);
		if (playedTime >= 0)
		{
			pnTime = pnTime - playedTime;
		}
		logger.log(LOGINFO, "Remaining File length: %ld ", pnTime);
		if (pnTime < 1000)
		{
			SsmSetWaitDtmf(ch, 0, 1, ' ', true); //set the DTMF termination character
		}
		else
		{
			WORD wTimeOut = (WORD)(pnTime / 1000 + 5);
			SsmSetWaitDtmf(ch, wTimeOut, 1, ' ', true); //set the DTMF termination character
		}
	}
}

BOOL CSpiceOBDDlg::IsChannelBlocked(int chVal)
{
	for (vector<BlockedChannelRange>::iterator it = blockedChannelsRange.begin(); it != blockedChannelsRange.end(); it++)
	{
		if (chVal >= it->minChVal && chVal <= it->maxChVal)
		{
			return true;
		}
	}
	return false;
}

BOOL CSpiceOBDDlg::IsPhNumCalledSuccess(char* encrypted_ani)
{
	char phQuery[1024];
	int phCount;
	sprintf_s(phQuery, "select count(1) as countNum  from tbl_obd_answered_calls where encrypted_ani ='%s' and reason = 'Answered' and date(call_date) = date(now())",
		encrypted_ani);

	int queryState = mysql_query(connSelect, phQuery);
	logger.log(LOGINFO, "IsPhNumCalledSuccess Query: %s", phQuery);
	if (queryState != 0)
	{
		CString err(mysql_error(connSelect));
		AfxMessageBox(err);
	}

	MYSQL_RES *resPhNum = mysql_store_result(connSelect);
	MYSQL_ROW rowPhNum;
	if ((rowPhNum = mysql_fetch_row(resPhNum)) != NULL)
	{
		phCount = atoi(rowPhNum[0]);
	}
	mysql_free_result(resPhNum);
	if (phCount)return true;

	return false;
}

void CSpiceOBDDlg::DialToIVR(int ch, BOOL isContest)
{
	SsmStopPlay(ch);
	SsmClearRxDtmfBuf(ch);
	ChInfo[ch].DtServiceState = USER_CALL_WAIT_PATCHUP;
	//sprintf_s(CampID, "%d", Campaigns.at(tempCampId).loadedIndex[3]);
	if (/*SsmPlayIndexString(i, CampID) SsmPlayFile(i, Campaigns.at(tempCampId).promptsPath[4], -1, 0, -1)*/ PlayMediaFile(ch, CALLER_PROMPT) == -1)
	{
		LogErrorCodeAndMessage(ch);
		HangupCall(ch);
	}
	if (StrStrA(Campaigns.at(ChInfo[ch].CampaignID).campaign_name, rvCampaign))
	{
		isContest = true;
	}
	//Fetching an idle channel for IVR patching
	for (int count = 0; count <= 1; count++)
	{
		ChInfo[ch].IVRChannelNumber = GetAnIdleChannel(isContest);
		if (ChInfo[ch].IVRChannelNumber != -1) break;
	}
	if (ChInfo[ch].IVRChannelNumber == -1)
	{
		HangupCall(ch);
		return;
	}
	//Set cli and DNIS for call patchedup
	SsmSetIsupFlag(ChInfo[ch].IVRChannelNumber, ISUP_CallerParam, 0x1303, 0); //set IAM for calling party number

	if (SsmSetTxCallerId(ChInfo[ch].IVRChannelNumber, ChInfo[ch].pPhoNumBuf) == -1)
	{
		getErrorResult(L"DoUserWork-> SsmSetTxCallerId call patchup IVR");
		HangupCall(ch);
	}

	CString cliStr(ChInfo[ch].pPhoNumBuf), dnisStr(ChInfo[ch].CDRStatus.DNIS), campNameStr(Campaigns.at(ChInfo[ch].CampaignID).campaign_name);
	m_TrkChList.SetItemText(ChInfo[ch].IVRChannelNumber, 1, campNameStr);
	m_TrkChList.SetItemText(ChInfo[ch].IVRChannelNumber, 3, cliStr);
	m_TrkChList.SetItemText(ChInfo[ch].IVRChannelNumber, 4, dnisStr);
	//setting ISUP parameters
	UCHAR IsupParamFCI[2];

	IsupParamFCI[0] = 0x20;
	IsupParamFCI[1] = 0x01;
	SsmSetIsupParameter(ChInfo[ch].IVRChannelNumber, 0x01, 0x07, 2, IsupParamFCI); //forward call indicator

	SsmSetIsupFlag(ChInfo[ch].IVRChannelNumber, ISUP_PhoNumParam, 0x1001, 0); //set IAM for called party number

	if (SsmAutoDial(ChInfo[ch].IVRChannelNumber, ChInfo[ch].CDRStatus.DNIS) == -1)
	{
		LogErrorCodeAndMessage(ChInfo[ch].IVRChannelNumber);
		HangupIVRCall(ChInfo[ch].IVRChannelNumber);
		HangupCall(ch);
	}
	//ChInfo[ChInfo[ch].IVRChannelNumber].InUse = true;
	logger.log(LOGINFO, "CLI to IVR: %s, DNIS: %s", ChInfo[ch].pPhoNumBuf, ChInfo[ch].CDRStatus.DNIS);
}

void CSpiceOBDDlg::DoUserWork()
{
	int nResult, nDirection;
	CString tempPhoneNum;
	int welcomePromptsNum;
	std::string aniNamePrompts;
	BOOL namePromptFound;
	//	char CampID[7];
	int tempCampId;
	IsUpdate = false;
	WORD releaseCode;
	logger.log(LOGINFO, "DoUserWork Start");
	for (int i = 0; i < nTotalCh; i++)
	{
		if (ChInfo[i].isChannelBlocked) continue; //skip these channels if they are blocked

		if (SsmGetChType(i) != 11) continue;
		nResult = SsmGetAutoCallDirection(i, &nDirection);

		if (nResult == -1 || nDirection < 1) continue;
		tempCampId = ChInfo[i].CampaignID;
		if (ChInfo[i].isIVRChannel)
		{
			if (SsmGetChState(i) == S_CALL_PENDING) {
				HangupIVRCall(i);
			}
			continue;
		}

		switch (ChInfo[i].nStep)
		{
		case USER_IDLE:
			ChInfo[i].lineState = SsmGetChState(i);
			//ChInfo[i].DTCounter = 0;//Initialize iteration counters...
			ChInfo[i].levelNumber = 1;
			if (ChInfo[i].lineState == S_CALL_STANDBY && ChInfo[i].EnCalled == false)
			{
				//Copy phone Numbers to auto dial
				if (tempCampId != -1 && !Campaigns.at(tempCampId).phnumBuf.empty() && IsStartDialling && IsDailingTimeInRange && !isDeallocateProcedureCalled)
				{
					StrCpyA(ChInfo[i].CDRStatus.cli, Campaigns.at(tempCampId).cliList.at(rand() % Campaigns.at(tempCampId).cliList.size()).c_str());//picking random CLI
																																					//Test call for each campaign
					if (Campaigns.at(tempCampId).testCallflag && ++Campaigns.at(tempCampId).tmpCallCounter >= Campaigns.at(tempCampId).testCallCounter)
					{
						std::string DecryptedVal = aesEncryption.DecodeAndDecrypt(Campaigns.at(tempCampId).testCallNumber);
						/*if (StrCmpA(circleLrn, ""))
						{
						DecryptedVal.insert(0, circleLrn);
						}*/
						StrCpyA(ChInfo[i].pPhoNumBuf, DecryptedVal.c_str());
						StrCpyA(ChInfo[i].CDRStatus.encrypted_ani, Campaigns.at(tempCampId).testCallNumber);
						Campaigns.at(tempCampId).tmpCallCounter = 0;
					}
					else
					{
						/*if (StrCmpA(circleLrn, ""))
						{
						Campaigns.at(tempCampId).phnumBuf.front().ani.insert(0, circleLrn);
						}*/
						//logger.log(LOGINFO, "UpdateStatusAndPickNextRecords vector current size: %d for campaign Id: %d", Campaigns.at(tmpCmpId).phnumBuf.size(), tmpCmpId);
						char* tmpEncryptedAni = Campaigns.at(tempCampId).phnumBuf.front().encryptedAni;
						int priority = Campaigns.at(tempCampId).phnumBuf.front().priority;
						if (!UpdatePhNumbersStatus(Campaigns.at(tempCampId).campaign_id, tmpEncryptedAni))
						{
							logger.log(LOGINFO, "DoUserWork Row not updated...ph number: %s, channel number: %d", tmpEncryptedAni, i);
						}
						if (priority && IsPhNumCalledSuccess(tmpEncryptedAni))
						{
							Campaigns.at(tempCampId).phnumBuf.erase(Campaigns.at(tempCampId).phnumBuf.begin()); i--; continue;
						}
						std::string DecryptedVal = aesEncryption.DecodeAndDecrypt(tmpEncryptedAni);
						//StrCpyA(ChInfo[i].pPhoNumBuf, Campaigns.at(tempCampId).phnumBuf.front().ani.c_str());
						StrCpyA(ChInfo[i].pPhoNumBuf, DecryptedVal.c_str());
						StrCpyA(ChInfo[i].CDRStatus.encrypted_ani, tmpEncryptedAni);
						StrCpyA(ChInfo[i].promptsName, Campaigns.at(tempCampId).phnumBuf.front().promptsName);
						ChInfo[i].obdType = Campaigns.at(tempCampId).phnumBuf.front().obdType;
						//delete Campaigns.at(tempCampId).phnumBuf.front().ani;
						if (StrCmpA(ChInfo[i].promptsName, EMPTY_STRING))
						{
							StrCpyA(ChInfo[i].CDRStatus.songName, ChInfo[i].promptsName);
						}
						Campaigns.at(tempCampId).phnumBuf.erase(Campaigns.at(tempCampId).phnumBuf.begin());
					}
					CString tempStr2(ChInfo[i].pPhoNumBuf), tempStr1(ChInfo[i].CDRStatus.cli), campIdStr, campNameStr(Campaigns.at(tempCampId).campaign_name);
					//setting caller ID 
					if (SsmSetTxCallerId(i, ChInfo[i].CDRStatus.cli) == -1)
					{
						getErrorResult(_T(" SsmSetTxCallerId"));
					}
					wchar_t curCLI[20], curCampID[31], curPhBuf[31], curCampName[50];

					campIdStr.Format(_T("%d"), tempCampId);
					m_TrkChList.GetItemText(i, 1, curCampName, 50);
					if (curCampName != campNameStr)
					{
						m_TrkChList.SetItemText(i, 1, campNameStr);
					}
					m_TrkChList.GetItemText(i, 3, curCLI, 20);
					if (curCLI != tempStr1)
					{
						m_TrkChList.SetItemText(i, 3, tempStr1);
					}
					m_TrkChList.GetItemText(i, 4, curPhBuf, 31);
					if (curPhBuf != tempStr2)
					{
						m_TrkChList.SetItemText(i, 4, tempStr2);
					}
					m_TrkChList.GetItemText(i, 5, curCampID, 31);
					if (curCampID != campIdStr)
					{
						m_TrkChList.SetItemText(i, 5, campIdStr);
					}
					ChInfo[i].isAvailable = false;
					ChInfo[i].InUse = true;
					logger.log(LOGINFO, "DoUserWork Update data Ani : %s, Encrypted Ani: %s, Channel Num: %d", ChInfo[i].pPhoNumBuf, ChInfo[i].CDRStatus.encrypted_ani, i);
				}
				else
				{
					if (ChInfo[i].InUse)
					{
						ChInfo[i].InUse = false;
						StrCpyA(ChInfo[i].pPhoNumBuf, EMPTY_STRING);
						StrCpyA(ChInfo[i].promptsName, EMPTY_STRING);
						StrCpyA(ChInfo[i].CDRStatus.encrypted_ani, EMPTY_STRING);
						ChInfo[i].obdType = 0;
						m_TrkChList.SetItemText(i, 1, L"");
						m_TrkChList.SetItemText(i, 3, L"");
						m_TrkChList.SetItemText(i, 4, L"");
						m_TrkChList.SetItemText(i, 5, L"");
					}
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
				SsmStopPlay(i);
				if (ChInfo[i].IVRChannelNumber != -1)
				{
					SsmStopTalkWith(i, ChInfo[i].IVRChannelNumber);
					SsmHangup(ChInfo[i].IVRChannelNumber);
					//ChInfo[ChInfo[i].IVRChannelNumber].InUse = false;
					ChInfo[i].IVRChannelNumber = -1;
				}
				GetDTMFandDNISBuffer(i);
				//SsmClearRxDtmfBuf(i);
				SsmHangup(i);
				ChInfo[i].nStep = USER_IDLE;
			}
			break;
		case USER_WAIT_REMOTE_PICKUP:
			/*logger.log(LOGINFO, "Phone Number: %s, AutoDial State:%d Channel Number: %d, channel state: %d, Release reason: %hu , autodialFail Reason : %d, Pending reason: %d",
			ChInfo[i].pPhoNumBuf, SsmChkAutoDial(i), i, SsmGetChState(i), SsmGetReleaseReason(i), SsmGetAutoDialFailureReason(i), SsmGetPendingReason(i));*/
			/*if (SsmGetChState(i) == S_CALL_RINGING)
			{
			StrCpyA(ChInfo[i].CDRStatus.status, "FAIL");
			StrCpyA(ChInfo[i].CDRStatus.reason, "Missed");
			StrCpyA(ChInfo[i].CDRStatus.reason_code, "2");
			}*/
			switch (SsmChkAutoDial(i))
			{
			case DIAL_VOICE:
			case DIAL_VOICEF1:
			case DIAL_VOICEF2:
				//sprintf_s(CampID, "%d", Campaigns.at(tempCampId).loadedIndex[1]);
				//logger.log(LOGINFO, "Camp Id: %s", CampID);
				StrCpyA(ChInfo[i].CDRStatus.status, "SUCCESS");
				StrCpyA(ChInfo[i].CDRStatus.reason, "Answered");
				//StrCpyA(ChInfo[i].CDRStatus.reason_code, "7");
				ChInfo[i].CDRStatus.answer_time = time(0);
				logger.log(LOGINFO, "Call picked up channel: %d, phone number: %s", i, ChInfo[i].pPhoNumBuf);

				if (StrCmpA(circleLrn, "") && strlen(ChInfo[i].pPhoNumBuf) > 10) //remove circle LRN from phonne number 
				{
					strncpy_s(ChInfo[i].pPhoNumBuf, ChInfo[i].pPhoNumBuf + 4, sizeof(ChInfo[i].pPhoNumBuf));
				}
				//logger.log(LOGINFO, "file to be played: %s", Campaigns.at(tempCampId).promptsPath[1]);
				welcomePromptsNum = WELCOME_PROMPT; //default value is 100
				aniNamePrompts = ""; //default empty
				namePromptFound = false;
				ChInfo[i].mediaPlaySubState = POST_TOKEN_PROMPT;
				logger.log(LOGINFO, "First Prompt Name: %s and OBD_TYPE: %d", ChInfo[i].promptsName, ChInfo[i].obdType);
				if (ChInfo[i].obdType == Informative && StrCmpA(ChInfo[i].promptsName, EMPTY_STRING))//check if Informative trigger OBD
				{
					welcomePromptsNum = 0;
				}
				else if (StrStrA(Campaigns.at(tempCampId).campaign_name, namingTunes) &&
					(namePromptFound = GetAniTokenPromptsDetails(ChInfo[i].pPhoNumBuf, aniNamePrompts))) //check if name prompts found 
				{
					welcomePromptsNum = 0;
					StrCpyA(ChInfo[i].promptsName, nameTunesPrev);
					ChInfo[i].mediaPlaySubState = PREV_TOKEN_PROMPT;
				}
				if (/*SsmPlayIndexString(i, CampID)*/ PlayMediaFile(i, welcomePromptsNum) == -1)
				{
					LogErrorCodeAndMessage(i);
					HangupCall(i);
				}
				else
				{
					ChInfo[i].nStep = USER_TALKING;
					if (!ChInfo[i].obdType) { //check if obd_type is 0 in outdialer base table
						ChInfo[i].DialPlanStatus = Campaigns.at(tempCampId).obdDialPlan;
					}
					else
					{
						ChInfo[i].DialPlanStatus = Campaigns.at(tempCampId).obdDialPlan = (OBD_DIAL_PLAN)ChInfo[i].obdType;
					}

					if (ChInfo[i].DialPlanStatus != Informative)
					{
						ChInfo[i].ConsentState = USER_PLAY_PROMPT;
						ChInfo[i].nextConsentState = USER_PLAYING_PROMPT;
						ChInfo[i].levelNumber = 1;
						ChInfo[i].DtServiceState = USER_PLAY_PRODUCT;
						/**
						In case of name prompts, set prompts_name for TOKEN_PROMPT_PLAYING sub state
						**/
						if (namePromptFound)	StrCpyA(ChInfo[i].promptsName, aniNamePrompts.c_str());
						if (ChInfo[i].DialPlanStatus == AcquisitionalOBDWith1stAnd2ndConsent)
						{
							ChInfo[i].repeatCounter = 1; //Initialize iteration counters...
							ChInfo[i].ConsentState = USER_PLAYING_PROMPT;
							int pnFormat; long pnTime;
							if (SsmGetPlayingFileInfo(i, &pnFormat, &pnTime) == 0)
							{
								WORD wTimeOut = (WORD)(pnTime / 1000 + 5);
								SsmSetWaitDtmf(i, wTimeOut, 1, ' ', true); //set the DTMF termination character
							}
						}
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
			if (SsmGetChState(i) == S_CALL_PENDING || SsmGetChState(i) == S_CALL_STANDBY)
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
				case USER_PLAYING_PROMPT:
					ChInfo[i].mediaState = SsmCheckPlay(i);
					//logger.log(LOGINFO, "Media state: %d, ph Number: %s", ChInfo[i].mediaState, ChInfo[i].pPhoNumBuf);
					if (ChInfo[i].mediaState >= 0)
					{
						if (SsmChkWaitDtmf(i, ChInfo[i].CDRStatus.dtmf) >= 1)
						{
							if (StrCmpA(ChInfo[i].CDRStatus.dtmf, "1") == 0) //Right Input
							{
								SsmStopPlay(i);
								SsmClearRxDtmfBuf(i);
								if (PlayMediaFile(i, CALLER_PROMPT) == -1)
								{
									LogErrorCodeAndMessage(i);
									HangupCall(i);
								}
								int pnFormat; long pnTime;
								if (SsmGetPlayingFileInfo(i, &pnFormat, &pnTime) == 0)
								{
									WORD wTimeOut = (WORD)(pnTime / 1000 + 5);
									SsmSetWaitDtmf(i, wTimeOut, 6, '\0', false); //set the DTMF termination conditions
								}
								ChInfo[i].ConsentState = USER_WAIT_SECOND_CONSENT;
								//StrCpyA(ChInfo[i].CDRStatus.firstConsent, ChInfo[i].CDRStatus.dtmf); //copying correct input only.
							}
							else if (StrCmpA(ChInfo[i].CDRStatus.dtmf, "") == 0) //No Input
							{
								SsmStopPlay(i);
								if (++ChInfo[i].repeatCounter <= MaxPromptsRepeatsCount)
								{
									ChInfo[i].ConsentState = USER_PLAY_PROMPT;
									ChInfo[i].nextConsentState = USER_PLAYING_PROMPT;
									if (PlayMediaFile(i, NO_INPUT_PROMPT) == -1)
									{
										LogErrorCodeAndMessage(i);
										HangupCall(i);
									}
								}
								else
								{
									ChInfo[i].ConsentState = CALL_HANGUP;
									if (PlayMediaFile(i, NO_INPUT_PROMPT) == -1)
									{
										LogErrorCodeAndMessage(i);
										HangupCall(i);
									}
								}
							}
							else //Wrong input
							{
								SsmStopPlay(i);
								SsmClearRxDtmfBuf(i);
								if (++ChInfo[i].repeatCounter <= MaxPromptsRepeatsCount)
								{
									ChInfo[i].ConsentState = USER_PLAY_PROMPT;
									ChInfo[i].nextConsentState = USER_PLAYING_PROMPT;
									if (PlayMediaFile(i, INVALID_PROMPT) == -1)
									{
										LogErrorCodeAndMessage(i);
										HangupCall(i);
									}
								}
								else
								{
									ChInfo[i].ConsentState = CALL_HANGUP;
									if (PlayMediaFile(i, INVALID_PROMPT) == -1)
									{
										LogErrorCodeAndMessage(i);
										HangupCall(i);
									}
								}
							}
						}
					}
					break;
				case USER_PLAY_PROMPT:
					ChInfo[i].mediaState = SsmCheckPlay(i);
					if (ChInfo[i].mediaState >= 1)
					{
						SsmStopPlay(i);
						SsmClearRxDtmfBuf(i);
						if (PlayMediaFile(i, WELCOME_PROMPT) == -1)
						{
							LogErrorCodeAndMessage(i);
							HangupCall(i);
						}
						int pnFormat; long pnTime;
						if (SsmGetPlayingFileInfo(i, &pnFormat, &pnTime) == 0)
						{
							logger.log(LOGINFO, "File length: %ld ", pnTime);
							if (pnTime < 1000) //wav file is empty hangup call.
							{
								SsmStopPlay(i);
								ChInfo[i].ConsentState = CALL_HANGUP;
								if (PlayMediaFile(i, CALL_HANGUP_PROMPT) == -1)
								{
									LogErrorCodeAndMessage(i);
									HangupCall(i);
								}
							}
							else
							{
								ChInfo[i].ConsentState = ChInfo[i].nextConsentState;
								WORD wTimeOut = (WORD)(pnTime / 1000 + 5);
								SsmSetWaitDtmf(i, wTimeOut, 1, ' ', true); //set the DTMF termination character

							}
						}
					}
					if (SsmGetChState(i) == S_CALL_PENDING)
					{
						HangupCall(i);
					}
					break;
				case USER_WAIT_SECOND_CONSENT:
					ChInfo[i].mediaState = SsmCheckPlay(i);
					if (ChInfo[i].mediaState >= 1)
					{
						if (SsmChkWaitDtmf(i, ChInfo[i].CDRStatus.dtmf2) >= 1)
						{
							logger.log(LOGINFO, "pin-code captured: %s, ph Number: %s", ChInfo[i].CDRStatus.dtmf2, ChInfo[i].pPhoNumBuf);
							SsmStopPlay(i);
							ChInfo[i].ConsentState = CALL_HANGUP;
							if (PlayMediaFile(i, CALL_HANGUP_PROMPT) == -1)
							{
								LogErrorCodeAndMessage(i);
								HangupCall(i);
							}
						}
					}
					if (SsmGetChState(i) == S_CALL_PENDING)
					{
						HangupCall(i);
					}
					break;
				case CALL_HANGUP:
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
			case AcquisitionalOBDWithIVRServiceCrossPromo:
				switch (ChInfo[i].DtServiceState) {
				case USER_PLAYING_PRODUCT:
					ChInfo[i].mediaState = SsmCheckPlay(i);
					//logger.log(LOGINFO, "Media state: %d, ph Number: %s", ChInfo[i].mediaState, ChInfo[i].pPhoNumBuf);
					if (ChInfo[i].mediaState >= 0)
					{
						//StrCpyA(ChInfo[i].CDRStatus.dtmf, SsmGetDtmfStrA(i));
						if (SsmChkWaitDtmf(i, ChInfo[i].CDRStatus.dtmf) >= 1)
						{
							if (StrCmpA(ChInfo[i].CDRStatus.dtmf, "")) //Input detected
							{
								ChInfo[i].isApiToBeCalled = false; //Reset flag if first call is disconnected only.
																   //Get song details
																   /*char songQuery[1024],songName[50]*/
								char songCode[20], levelType[10], patchDNIS[31];
								int  jumpLevel;

								/*sprintf_s(songQuery, "select song_name, promo_code, level_type, getSecondDnis('%s') as DNIS, cg_level from tbl_songs_master where campaign_id = '%s' and dtmf = '%s' and repeat_level = %d",
								Campaigns.at(ChInfo[i].CampaignID).campaign_id, Campaigns.at(ChInfo[i].CampaignID).campaign_id, ChInfo[i].CDRStatus.dtmf, ChInfo[i].levelNumber);

								queryState = mysql_query(connSelect, songQuery);
								logger.log(LOGINFO, "Song query for input Detected: %s", songQuery);
								if (queryState != 0)
								{
								CString err(mysql_error(connSelect));
								AfxMessageBox(err);
								}

								MYSQL_RES *resPromo = mysql_store_result(connSelect);
								MYSQL_ROW rowPromo;

								StrCpyA(songName, "");*/
								StrCpyA(songCode, "");
								StrCpyA(levelType, "");
								StrCpyA(patchDNIS, "");
								jumpLevel = -1;
								/*if ((rowPromo = mysql_fetch_row(resPromo)) != NULL)
								{*/
								//StrCpyA(songName, rowPromo[0]);
								if (Campaigns.at(ChInfo[i].CampaignID).tblSongsMaster.count(ChInfo[i].levelNumber))
								{
									StrCpyA(levelType, Campaigns.at(ChInfo[i].CampaignID).tblSongsMaster[ChInfo[i].levelNumber].levelType);
									if (Campaigns.at(ChInfo[i].CampaignID).tblSongsMaster[ChInfo[i].levelNumber].dtmfWiseData.count(ChInfo[i].CDRStatus.dtmf)) //Right Input
									{
										jumpLevel = atoi(Campaigns.at(ChInfo[i].CampaignID).tblSongsMaster[ChInfo[i].levelNumber].cgLevel);

										std::string tmpDNIS = Campaigns.at(ChInfo[i].CampaignID).tblSongsMaster[ChInfo[i].levelNumber].patchDnis;
										std::string tmpLangCode = Campaigns.at(ChInfo[i].CampaignID).tblSongsMaster[ChInfo[i].levelNumber].dtmfWiseData[ChInfo[i].CDRStatus.dtmf].langCode;
										if (tmpLangCode.compare(DEFAULT_LANG_CODE)) //check for default lang code.
										{
											tmpDNIS.replace(4, tmpLangCode.length(), tmpLangCode);
										}
										std::string tmpDTMF = "X";
										_strupr_s(levelType);
										if (std::string(levelType).substr(0, 2).compare("DT") == 0)//Find if level type is DT
										{
											sprintf_s(songCode, "%s%07s", std::string(levelType).substr(3, std::string::npos).c_str(),
												Campaigns.at(ChInfo[i].CampaignID).tblSongsMaster[ChInfo[i].levelNumber].dtmfWiseData[ChInfo[i].CDRStatus.dtmf].dtmfPromoCode.c_str());
											StrCpyA(ChInfo[i].CDRStatus.songName,
												Campaigns.at(ChInfo[i].CampaignID).tblSongsMaster[ChInfo[i].levelNumber].dtmfWiseData[ChInfo[i].CDRStatus.dtmf].dtmfPromoCode.c_str()); //copying song code for MIS instead of song name.

											tmpDNIS.erase(tmpDNIS.find(tmpDTMF), std::string::npos);
											StrCpyA(patchDNIS, tmpDNIS.c_str());
											//StrCatA(patchDNIS, ChInfo[i].CDRStatus.dtmf);
											//Changes made to acomodate in case of * and # are part of dtmf in DT songs
											if (!StrCmpA(ChInfo[i].CDRStatus.dtmf, "*")) //check if *
											{
												StrCatA(patchDNIS, "1");
											}
											else if (!StrCmpA(ChInfo[i].CDRStatus.dtmf, "#")) // check if #
											{
												StrCatA(patchDNIS, "2");
											}
											else //for all 0-9
											{
												StrCatA(patchDNIS, ChInfo[i].CDRStatus.dtmf);
											}
										}
										else if (StrCmpIA(levelType, "SERVICE") == 0)
										{
											sprintf_s(songCode, "%04s",
												Campaigns.at(ChInfo[i].CampaignID).tblSongsMaster[ChInfo[i].levelNumber].dtmfWiseData[ChInfo[i].CDRStatus.dtmf].dtmfPromoCode.c_str());
											tmpDNIS.erase(tmpDNIS.find(tmpDTMF), std::string::npos);
											StrCpyA(patchDNIS, tmpDNIS.c_str());
											//StrCatA(patchDNIS, ChInfo[i].CDRStatus.dtmf);
											StrCatA(patchDNIS, Campaigns.at(tempCampId).first_consent_digit);
										}
										else
										{
											sprintf_s(songCode, "%04s",
												Campaigns.at(ChInfo[i].CampaignID).tblSongsMaster[ChInfo[i].levelNumber].dtmfWiseData[ChInfo[i].CDRStatus.dtmf].dtmfPromoCode.c_str());
											StrCpyA(patchDNIS, "");
										}
										StrCatA(patchDNIS, songCode);
										StrCpyA(ChInfo[i].CDRStatus.DNIS, patchDNIS);

										if (StrCmpIA(levelType, "skip") == 0) //check for skip first dtmf
										{
											DialToIVR(i, true);
										}
										else
										{
											DialToIVR(i, false);
										}
										ChInfo[i].levelNumber = jumpLevel;
									}
									else //Wrong Input
									{
										//Fetch invalid input repeat level value.
										/*char songQuery[1024];*/
										//int queryState, 
										int invalidKeyLevel;

										/*sprintf_s(songQuery, "select promo_code, level_type, invalid_key  from tbl_songs_master where campaign_id = '%s' and repeat_level = %d limit 1",
										Campaigns.at(ChInfo[i].CampaignID).campaign_id, ChInfo[i].levelNumber);

										queryState = mysql_query(connSelect, songQuery);
										logger.log(LOGINFO, "Song query for invalid Input: %s", songQuery);
										if (queryState != 0)
										{
										CString err(mysql_error(connSelect));
										AfxMessageBox(err);
										}
										MYSQL_RES *resPromo = mysql_store_result(connSelect);
										MYSQL_ROW rowPromo;*/
										invalidKeyLevel = 1;
										if (Campaigns.at(ChInfo[i].CampaignID).tblSongsMaster[ChInfo[i].levelNumber].dtmfWiseData.size())
										{
											StrCpyA(songCode, Campaigns.at(ChInfo[i].CampaignID).tblSongsMaster[ChInfo[i].levelNumber].dtmfWiseData.begin()->second.dtmfPromoCode.c_str());
											StrCpyA(levelType, Campaigns.at(ChInfo[i].CampaignID).tblSongsMaster[ChInfo[i].levelNumber].levelType);
											invalidKeyLevel = atoi(Campaigns.at(ChInfo[i].CampaignID).tblSongsMaster[ChInfo[i].levelNumber].invalidKeyLevel);
										}
										//mysql_free_result(resPromo);
										SsmClearRxDtmfBuf(i);

										if (invalidKeyLevel)
										{
											if (StrCmpIA(levelType, "skip") == 0) //check for skip first dtmf
											{
												StrCpyA(ChInfo[i].CDRStatus.DNIS, songCode);
												DialToIVR(i, true);
											}
											else
											{
												SsmStopPlay(i);
												ChInfo[i].DtServiceState = USER_PLAY_PRODUCT;
												ChInfo[i].levelNumber = invalidKeyLevel;
												int invalidKeyPrompt = INVALID_PROMPT;
												if (invalidKeyLevel >= WELCOME_PROMPT && invalidKeyLevel < CALLER_PROMPT)
												{
													invalidKeyPrompt = invalidKeyLevel;
												}
												//sprintf_s(CampID, "%d", Campaigns.at(tempCampId).loadedIndex[4]);
												if (/*SsmPlayIndexString(i, CampID) SsmPlayFile(i, Campaigns.at(tempCampId).promptsPath[6], -1, 0, -1)*/PlayMediaFile(i, invalidKeyPrompt) == -1)
												{
													LogErrorCodeAndMessage(i);
													HangupCall(i);
												}
											}
										}
										else
										{
											ContinuePlayingPrompts(i);
										}
										//SsmSetWaitDtmfExA(i, 15000, 1, "1", true);
									}
								}
							}
							else  //No Input
							{
								//Fetch no input repeat level value.
								//char songQuery[1024],
								char songCode[20], levelType[10];
								//int queryState, 
								int noKeyLevel;

								/*sprintf_s(songQuery, "select promo_code, level_type, no_key from tbl_songs_master where campaign_id = '%s' and repeat_level = %d limit 1",
								Campaigns.at(ChInfo[i].CampaignID).campaign_id, ChInfo[i].levelNumber);

								queryState = mysql_query(connSelect, songQuery);
								logger.log(LOGINFO, "Song query for No Input: %s", songQuery);
								if (queryState != 0)
								{
								CString err(mysql_error(connSelect));
								AfxMessageBox(err);
								}
								MYSQL_RES *resPromo = mysql_store_result(connSelect);
								MYSQL_ROW rowPromo;*/
								noKeyLevel = 2;
								if (Campaigns.at(ChInfo[i].CampaignID).tblSongsMaster[ChInfo[i].levelNumber].dtmfWiseData.size())
								{
									StrCpyA(songCode, Campaigns.at(ChInfo[i].CampaignID).tblSongsMaster[ChInfo[i].levelNumber].dtmfWiseData.begin()->second.dtmfPromoCode.c_str());
									StrCpyA(levelType, Campaigns.at(ChInfo[i].CampaignID).tblSongsMaster[ChInfo[i].levelNumber].levelType);
									noKeyLevel = atoi(Campaigns.at(ChInfo[i].CampaignID).tblSongsMaster[ChInfo[i].levelNumber].noKeyLevel);
								}
								//mysql_free_result(resPromo);
								if (StrCmpIA(levelType, "skip") == 0) //check for skip first dtmf
								{
									StrCpyA(ChInfo[i].CDRStatus.DNIS, songCode);
									DialToIVR(i, true);
								}
								else
								{
									SsmStopPlay(i);
									ChInfo[i].DtServiceState = USER_PLAY_PRODUCT;
									ChInfo[i].levelNumber = noKeyLevel;

									int noKeyPrompt = NO_INPUT_PROMPT;
									if (noKeyLevel >= WELCOME_PROMPT && noKeyLevel < CALLER_PROMPT)
									{
										noKeyPrompt = noKeyLevel;
									}
									//sprintf_s(CampID, "%d", Campaigns.at(tempCampId).loadedIndex[2]);
									if (/*SsmPlayIndexString(i, CampID) SsmPlayFile(i, Campaigns.at(tempCampId).promptsPath[7], -1, 0, -1)*/PlayMediaFile(i, noKeyPrompt) == -1)
									{
										LogErrorCodeAndMessage(i);
										HangupCall(i);
									}
								}
								//int pnFormat; long pnTime;
								//if (SsmGetPlayingFileInfo(i, &pnFormat, &pnTime) == 0)
								//{
								//	WORD wTimeOut = pnTime/1000 + 5;
								//	SsmSetWaitDtmf(i, wTimeOut, 1, ' ', true); //set the DTMF termination character
								//}
								//SsmSetWaitDtmfExA(i, 18000, 1, "1", true);
							}
						}
					}
					break;
				case USER_CALL_WAIT_PATCHUP:
					switch (SsmChkAutoDial(ChInfo[i].IVRChannelNumber))
					{
					case DIAL_VOICE:
					case DIAL_VOICEF1:
					case DIAL_VOICEF2:
						SsmStopPlay(i);
						//SsmStopSendTone(i);
						if (SsmTalkWith(i, ChInfo[i].IVRChannelNumber) == -1)
						{
							char  errMsg[256];
							SsmGetLastErrMsg(errMsg);
							logger.log(LOGERR, "Call Patchup failed between Ch: %d and ivr Ch: %d, Reason: %s", i, ChInfo[i].IVRChannelNumber, errMsg);
							SsmStopTalkWith(i, ChInfo[i].IVRChannelNumber);
							HangupCall(i);
							HangupIVRCall(ChInfo[i].IVRChannelNumber);
							ChInfo[i].IVRChannelNumber = -1;
						}
						ChInfo[i].DtServiceState = USER_CALL_PATCHUP;
						logger.log(LOGINFO, "Call PatchedUp between Ch: %d and ivr Ch: %d", i, ChInfo[i].IVRChannelNumber);

						break;
					case DIAL_FAILURE:
					case DIAL_NO_DIALTONE:
					case DIAL_ECHO_NOVOICE:
					case DIAL_INVALID_PHONUM:
					case DIAL_BUSYTONE:
					case DIAL_NOVOICE:
					case DIAL_NOANSWER:
						//ChInfo[i].CDRStatus.end_time = time(0);
						releaseCode = SsmGetReleaseReason(ChInfo[i].IVRChannelNumber);
						StrCpyA(ChInfo[ChInfo[i].IVRChannelNumber].CDRStatus.status, "FAIL");
						sprintf_s(ChInfo[ChInfo[i].IVRChannelNumber].CDRStatus.reason_code, "%hu", releaseCode);
						StrCpyA(ChInfo[ChInfo[i].IVRChannelNumber].CDRStatus.reason, GetReleaseErrorReason(releaseCode));
						HangupCall(i);
						HangupIVRCall(ChInfo[i].IVRChannelNumber);
						logger.log(LOGINFO, "patch up failed with reason: %s", ChInfo[ChInfo[i].IVRChannelNumber].CDRStatus.reason);
						ChInfo[i].IVRChannelNumber = -1;
						break;
					case DIAL_STANDBY:
						nResult = SsmGetChStateKeepTime(ChInfo[i].IVRChannelNumber);

						if (nResult > 2000)
						{
							//ChInfo[i].CDRStatus.end_time = time(0);
							StrCpyA(ChInfo[ChInfo[i].IVRChannelNumber].CDRStatus.reason, "DIAL_STANDBY");
							StrCpyA(ChInfo[ChInfo[i].IVRChannelNumber].CDRStatus.status, "FAIL");
							sprintf_s(ChInfo[ChInfo[i].IVRChannelNumber].CDRStatus.reason_code, "%hu", SsmGetReleaseReason(i));
							HangupIVRCall(ChInfo[i].IVRChannelNumber);
							ChInfo[i].IVRChannelNumber = -1;
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
					if (SsmGetChState(i) == S_CALL_PENDING || SsmGetChState(ChInfo[i].IVRChannelNumber) == S_CALL_PENDING)
					{
						//SsmStopSendTone(i);
						HangupCall(i);
						HangupIVRCall(ChInfo[i].IVRChannelNumber);
						ChInfo[i].IVRChannelNumber = -1;
					}
					break;
				case USER_CALL_PATCHUP:
					if ((SsmGetChState(ChInfo[i].IVRChannelNumber) == S_CALL_PENDING || SsmGetChState(ChInfo[i].IVRChannelNumber) == S_CALL_STANDBY) && ChInfo[i].levelNumber != -1)
					{
						GetDTMFandDNISBuffer(i);
						SsmStopTalkWith(i, ChInfo[i].IVRChannelNumber);
						HangupIVRCall(ChInfo[i].IVRChannelNumber);
						m_TrkChList.SetItemText(ChInfo[i].IVRChannelNumber, 1, L"");
						m_TrkChList.SetItemText(ChInfo[i].IVRChannelNumber, 3, L"");
						m_TrkChList.SetItemText(ChInfo[i].IVRChannelNumber, 4, L"");
						ChInfo[i].DtServiceState = USER_PLAYING_PRODUCT;
						ChInfo[i].IVRChannelNumber = -1;
						//sprintf_s(CampID, "%d", Campaigns.at(tempCampId).loadedIndex[2]);
						if (/*SsmPlayIndexString(i, CampID) SsmPlayFile(i, Campaigns.at(tempCampId).promptsPath[ChInfo[i].levelNumber], -1, 0, -1)*/ PlayMediaFile(i, ChInfo[i].levelNumber) == -1)
						{
							LogErrorCodeAndMessage(i);
							HangupCall(i);
						}
						if (ChInfo[i].levelNumber >= WELCOME_PROMPT && ChInfo[i].levelNumber < CALLER_PROMPT)
						{
							ChInfo[i].DtServiceState = USER_PLAY_PRODUCT;
						}
						else
						{
							int pnFormat; long pnTime;
							if (SsmGetPlayingFileInfo(i, &pnFormat, &pnTime) == 0)
							{
								WORD wTimeOut = (WORD)(pnTime / 1000 + 5);
								SsmSetWaitDtmf(i, wTimeOut, 1, ' ', true); //set the DTMF termination character
							}
						}
						logger.log(LOGINFO, "First call patchup Disconnected..");
					}
					else if (SsmGetChState(i) == S_CALL_PENDING || SsmGetChState(ChInfo[i].IVRChannelNumber) == S_CALL_PENDING || SsmGetChState(ChInfo[i].IVRChannelNumber) == S_CALL_STANDBY)
					{
						GetDTMFandDNISBuffer(i);
						SsmStopTalkWith(i, ChInfo[i].IVRChannelNumber);
						HangupCall(i);
						HangupIVRCall(ChInfo[i].IVRChannelNumber);

						ChInfo[i].IVRChannelNumber = -1;
						logger.log(LOGINFO, "call patchup Disconnected..");
					}

					break;
				case USER_STOP_PLAYING:
					ChInfo[i].mediaState = SsmCheckPlay(i);
					if (ChInfo[i].mediaState >= 1 || SsmGetChState(i) == S_CALL_PENDING)
					{
						HangupCall(i);
					}
					break;
				case USER_PLAY_PRODUCT:
					ChInfo[i].mediaState = SsmCheckPlay(i);
					if (ChInfo[i].mediaState >= 1)
					{
						switch (ChInfo[i].mediaPlaySubState)
						{
						case PREV_TOKEN_PROMPT:
							if (PlayMediaFile(i, 0) == -1) //prompts name set already
							{
								LogErrorCodeAndMessage(i);
								HangupCall(i);
							}
							ChInfo[i].mediaPlaySubState = TOKEN_PROMPT_PLAYING;
							break;
						case TOKEN_PROMPT_PLAYING:
							StrCpyA(ChInfo[i].promptsName, nameTunesPost);
							if (PlayMediaFile(i, 0) == -1)
							{
								LogErrorCodeAndMessage(i);
								HangupCall(i);
							}
							ChInfo[i].mediaPlaySubState = POST_TOKEN_PROMPT;
							break;
						case POST_TOKEN_PROMPT:
							//Fetch no input repeat level value.
							//char songQuery[1024],
							char levelType[10];
							//int queryState, 
							int noKeyLevel;
							noKeyLevel = 2;
							if (Campaigns.at(ChInfo[i].CampaignID).tblSongsMaster.size())
							{
								StrCpyA(levelType, Campaigns.at(ChInfo[i].CampaignID).tblSongsMaster[ChInfo[i].levelNumber].levelType);
								noKeyLevel = atoi(Campaigns.at(ChInfo[i].CampaignID).tblSongsMaster[ChInfo[i].levelNumber].noKeyLevel);
							}
							//mysql_free_result(resPromo);

							if (ChInfo[i].levelNumber >= WELCOME_PROMPT && ChInfo[i].levelNumber < CALLER_PROMPT)
							{
								ChInfo[i].levelNumber = noKeyLevel;
							}
							SsmStopPlay(i);
							SsmClearRxDtmfBuf(i);
							//sprintf_s(CampID, "%d", Campaigns.at(tempCampId).loadedIndex[2]);
							if (/*SsmPlayIndexString(i, CampID) SsmPlayFile(i, Campaigns.at(tempCampId).promptsPath[ChInfo[i].levelNumber], -1, 0, -1)*/ PlayMediaFile(i, ChInfo[i].levelNumber) == -1)
							{
								LogErrorCodeAndMessage(i);
								HangupCall(i);
							}
							int pnFormat; long pnTime;
							if (SsmGetPlayingFileInfo(i, &pnFormat, &pnTime) == 0)
							{
								logger.log(LOGINFO, "File length: %ld ", pnTime);
								if (pnTime < 1000 || ChInfo[i].levelNumber == CALL_HANGUP_PROMPT) //wav file is empty hangup call.
								{
									SsmStopPlay(i);
									ChInfo[i].DtServiceState = USER_STOP_PLAYING;
									//sprintf_s(CampID, "%d", Campaigns.at(tempCampId).loadedIndex[2]);
									if (/*SsmPlayIndexString(i, CampID) SsmPlayFile(i, Campaigns.at(tempCampId).promptsPath[5], -1, 0, -1)*/ PlayMediaFile(i, CALL_HANGUP_PROMPT) == -1)
									{
										LogErrorCodeAndMessage(i);
										HangupCall(i);
									}
								}
								else
								{
									ChInfo[i].DtServiceState = USER_PLAYING_PRODUCT;
									WORD wTimeOut;
									if (StrCmpIA(levelType, "skip") == 0) //check for skip first dtmf
									{
										wTimeOut = (WORD)(pnTime / 1000);
									}
									else
									{
										wTimeOut = (WORD)(pnTime / 1000 + 5);
									}
									SsmSetWaitDtmf(i, wTimeOut, 1, ' ', true); //set the DTMF termination character
								}
							}
							break;
						default:
							break;
						}
					}
					if (SsmGetChState(i) == S_CALL_PENDING)
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
				//if (ChInfo[i].mediaState == 0)
				//{
				//	StrCpyA(ChInfo[i].CDRStatus.reason_code, "0");
				//	//logger.log(LOGINFO, "media state 0");
				//}
				if (ChInfo[i].mediaState >= 1)
				{
					StrCpyA(ChInfo[i].CDRStatus.reason_code, "1");
					//logger.log(LOGINFO, "media state > 1");
					HangupCall(i);
				}
				break;

			case AcquisitionalOBDWith1stConsent:
				switch (ChInfo[i].ConsentState) {
				case USER_PLAYING_PROMPT:
					ChInfo[i].mediaState = SsmCheckPlay(i);
					//logger.log(LOGINFO, "Media state: %d, ph Number: %s", ChInfo[i].mediaState, ChInfo[i].pPhoNumBuf);
					if (ChInfo[i].mediaState >= 0)
					{
						if (SsmChkWaitDtmf(i, ChInfo[i].CDRStatus.dtmf) >= 1)
						{
							if (StrCmpA(ChInfo[i].CDRStatus.dtmf, "1") == 0 || StrCmpA(ChInfo[i].CDRStatus.dtmf, "2") == 0 || StrCmpA(ChInfo[i].CDRStatus.dtmf, "3") == 0) //Right Input
							{
								SsmStopPlay(i);
								SsmClearRxDtmfBuf(i);
								ChInfo[i].ConsentState = CALL_HANGUP;
								if (PlayMediaFile(i, CALL_HANGUP_PROMPT) == -1)
								{
									LogErrorCodeAndMessage(i);
									HangupCall(i);
								}
								StrCpyA(ChInfo[i].CDRStatus.firstConsent, ChInfo[i].CDRStatus.dtmf); //copying correct input only.
							}
							else if (StrCmpA(ChInfo[i].CDRStatus.dtmf, "") == 0) //No Input
							{
								SsmStopPlay(i);
								ChInfo[i].ConsentState = USER_PLAY_PROMPT;
								ChInfo[i].nextConsentState = USER_REPEAT_PROMPT;
								if (PlayMediaFile(i, NO_INPUT_PROMPT) == -1)
								{
									LogErrorCodeAndMessage(i);
									HangupCall(i);
								}
							}
							else //Wrong input
							{
								SsmStopPlay(i);
								SsmClearRxDtmfBuf(i);
								ChInfo[i].ConsentState = USER_PLAY_PROMPT;
								ChInfo[i].nextConsentState = USER_REPEAT_PROMPT;

								if (PlayMediaFile(i, INVALID_PROMPT) == -1)
								{
									LogErrorCodeAndMessage(i);
									HangupCall(i);
								}
							}
						}
					}
					break;
				case USER_REPEAT_PROMPT:
					ChInfo[i].mediaState = SsmCheckPlay(i);
					if (ChInfo[i].mediaState >= 0)
					{
						if (SsmChkWaitDtmf(i, ChInfo[i].CDRStatus.dtmf) >= 1)
						{
							if (StrCmpA(ChInfo[i].CDRStatus.dtmf, "1") == 0 || StrCmpA(ChInfo[i].CDRStatus.dtmf, "2") == 0 || StrCmpA(ChInfo[i].CDRStatus.dtmf, "3") == 0) //Right Input
							{
								SsmStopPlay(i);
								SsmClearRxDtmfBuf(i);
								ChInfo[i].ConsentState = CALL_HANGUP;
								if (PlayMediaFile(i, CALL_HANGUP_PROMPT) == -1)
								{
									LogErrorCodeAndMessage(i);
									HangupCall(i);
								}
								StrCpyA(ChInfo[i].CDRStatus.firstConsent, ChInfo[i].CDRStatus.dtmf); //copying correct input only.
							}
							else if (StrCmpA(ChInfo[i].CDRStatus.dtmf, "") == 0) //No input
							{
								SsmStopPlay(i);
								ChInfo[i].ConsentState = CALL_HANGUP;
								if (PlayMediaFile(i, CALL_HANGUP_PROMPT) == -1)
								{
									LogErrorCodeAndMessage(i);
									HangupCall(i);
								}
							}
							else //Wrong input
							{
								SsmStopPlay(i);
								SsmClearRxDtmfBuf(i);
								ChInfo[i].ConsentState = CALL_HANGUP;
								if (PlayMediaFile(i, CALL_HANGUP_PROMPT) == -1)
								{
									LogErrorCodeAndMessage(i);
									HangupCall(i);
								}
							}
						}
					}
					break;
				case USER_PLAY_PROMPT:
					ChInfo[i].mediaState = SsmCheckPlay(i);
					if (ChInfo[i].mediaState >= 1)
					{
						SsmStopPlay(i);
						SsmClearRxDtmfBuf(i);
						if (PlayMediaFile(i, 0) == -1)
						{
							LogErrorCodeAndMessage(i);
							HangupCall(i);
						}
						int pnFormat; long pnTime;
						if (SsmGetPlayingFileInfo(i, &pnFormat, &pnTime) == 0)
						{
							logger.log(LOGINFO, "File length: %ld ", pnTime);
							if (pnTime < 1000) //wav file is empty hangup call.
							{
								SsmStopPlay(i);
								ChInfo[i].ConsentState = CALL_HANGUP;
								if (PlayMediaFile(i, CALL_HANGUP_PROMPT) == -1)
								{
									LogErrorCodeAndMessage(i);
									HangupCall(i);
								}
							}
							else
							{
								ChInfo[i].ConsentState = ChInfo[i].nextConsentState;
								WORD wTimeOut = (WORD)(pnTime / 1000 + 5);
								SsmSetWaitDtmf(i, wTimeOut, 1, ' ', true); //set the DTMF termination character
							}
						}
					}
					if (SsmGetChState(i) == S_CALL_PENDING)
					{
						HangupCall(i);
					}
					break;
				case CALL_HANGUP:
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
			if (ChInfo[i].lineState == S_CALL_PENDING || ChInfo[i].lineState == S_CALL_STANDBY) //remote/user hangup
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
	logger.log(LOGINFO, "DoUserWork End");
}

BOOL CSpiceOBDDlg::GetAniTokenPromptsDetails(const char* msisdn, std::string & retTknPrompt)
{
	char tknPromptsQuery[1024];
	int queryState;

	sprintf_s(tknPromptsQuery, "select prompts_name from tbl_prompts_details where ani = '%s'", msisdn);

	queryState = mysql_query(connSelect, tknPromptsQuery);
	logger.log(LOGINFO, "Prompts name query: %s", tknPromptsQuery);
	if (queryState != 0)
	{
		CString err(mysql_error(connSelect));
		AfxMessageBox(err);
	}

	MYSQL_RES *resPromo = mysql_store_result(connSelect);
	MYSQL_ROW rowPromo;
	if ((rowPromo = mysql_fetch_row(resPromo)) != NULL)
	{
		retTknPrompt = rowPromo[0];
	}
	mysql_free_result(resPromo);
	//logger.log(LOGINFO, "song name: %s, song code : %s", songName, songCode);
	if (retTknPrompt.empty()) //not found
	{
		retTknPrompt = "N0";
		return false;
	}
	logger.log(LOGINFO, "Prompts name query return: %s", retTknPrompt.c_str());
	return true;
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
	//AfxMessageBox(L"Card Initialized !!");
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
	//char timeValue[25];

	GetCurrentDirectoryA(200, CurPath);
	StrCatA(CurPath, "\\CDRLogs");
	if (!PathIsDirectoryA(CurPath))
	{
		CreateDirectoryA(CurPath, NULL);
	}
	tm timeVal = logger.getTime(std::string());
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
	//char timeValue[25];

	GetCurrentDirectoryA(200, CurPath);
	StrCatA(CurPath, "\\ConsentLogs");
	if (!PathIsDirectoryA(CurPath))
	{
		CreateDirectoryA(CurPath, NULL);
	}
	tm timeVal = logger.getTime(std::string());
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


void CSpiceOBDDlg::WriteToINIFile(const char* key, const char* value)
{
	char curDirectory[256];
	GetCurrentDirectoryA(256, curDirectory);
	StrCatA(curDirectory, "\\DBSettings.INI");
	WritePrivateProfileStringA("Database", key, value, curDirectory);
}

BOOL CSpiceOBDDlg::InitializeChannels()
{
	try {
		//ReadNumbersFromFiles();
		Sleep(5 * 1000); //wait for 5 seconds initially.
						 //get total ports and cg ports and campaign wise distribution
		char queryStr[256];
		sprintf_s(queryStr, "select total_ports, cgport, circle_lrn from tbl_port_manager where circle = '%s'", circle);
		logger.log(LOGINFO, "Select ports query:  %s", queryStr);
		int query_state = mysql_query(connPort, queryStr);

		if (query_state != 0)
		{
			CString err(mysql_error(connPort));
			AfxMessageBox(err);
		}

		MYSQL_RES * resPort = mysql_store_result(connPort);
		MYSQL_ROW rowPort;
		while ((rowPort = mysql_fetch_row(resPort)) != NULL)
		{
			int totalch = atoi(rowPort[0]);
			std::string cgports = rowPort[1];
			StrCpyA(circleLrn, rowPort[2]);
			//Extracting cg channels range from cgport column
			nIVRMinCh = atoi(cgports.substr(0, cgports.find("-")).c_str());
			nIVRMaxCh = atoi(cgports.substr(cgports.find("-") + 1, cgports.length() - 1).c_str());
			if (nIVRMaxCh == 0) nIVRMaxCh = -1; //there is no cg port allocated.
			nTotalCh = totalch + (nIVRMaxCh - nIVRMinCh) + 1;
			int totalChannelsAvailable = SsmGetMaxCh(); //maximum channels available. 

			if (nTotalCh > totalChannelsAvailable)
			{
				nTotalCh = totalChannelsAvailable;
				CString cStr;
				cStr.Format(_T("Total number of ports available on the card are: %d, please reset the port numbers in DB Accordingly !"), totalChannelsAvailable);
				AfxMessageBox(cStr);
			}
			//nIVRMinCh = 0;//totalch;
			//nIVRMaxCh = CGMaxCHNum - 1;//nTotalCh;
		}
		mysql_free_result(resPort);
		logger.log(LOGINFO, "total Ports: %d, nIVRMinCh: %d, nIVRMaxCh: %d, blocked range : %s", nTotalCh, nIVRMinCh, nIVRMaxCh, blockedRangeStr);

		//get channels range from file and fill them in vector 
		std::string blockedRange = blockedRangeStr;
		while (!blockedRange.empty())
		{
			size_t DollarPos = blockedRange.find_first_of('$', 0);
			std::string tmpStr = blockedRange.substr(0, DollarPos);
			blockedChannelsRange.push_back({ atoi(tmpStr.substr(0, tmpStr.find("-")).c_str()), atoi(tmpStr.substr(tmpStr.find("-") + 1, tmpStr.length() - 1).c_str()) });
			blockedRange.erase(0, DollarPos + 1);
		}
		//Contest channel range distribution
		if (StrCmpA(contestChRange, "") == 0)
		{
			contestMinCh = contestMaxCh = 0;
		}
		else
		{
			const char* delim = "-#";
			char *context;
			char *chValueStr = strtok_s(contestChRange, delim, &context);
			contestMinCh = atoi(chValueStr);
			if (chValueStr)
			{
				chValueStr = strtok_s(NULL, delim, &context);
				contestMaxCh = atoi(chValueStr);
			}
		}
		logger.log(LOGINFO, "contestMinCh: %d, contestMaxCh: %d", contestMinCh, contestMaxCh);
		if (contestMaxCh)
		{
			if (contestMaxCh == nIVRMaxCh)
			{
				tempIVRMinCh = nIVRMinChNew = nIVRMinCh;
				nIVRMaxChNew = contestMinCh - 1;
			}
			else
			{
				tempIVRMinCh = nIVRMinChNew = contestMaxCh + 1;
				nIVRMaxChNew = nIVRMaxCh;
			}
			tmpContestMinCh = contestMinCh;
		}
		else
		{
			tempIVRMinCh = nIVRMinChNew = nIVRMinCh;
			nIVRMaxChNew = nIVRMaxCh;
		}
		logger.log(LOGINFO, "nIVRMinChNew: %d, tempIVRMinCh: %d, nIVRMaxChNew: %d", nIVRMinChNew, tempIVRMinCh, nIVRMaxChNew);
		//Extract the list of wait timeout values from string and put them in drop down.
		while (!waitTimeListStr.empty())
		{
			size_t DollarPos = waitTimeListStr.find_first_of('$', 0);
			std::string tmpStr = waitTimeListStr.substr(0, DollarPos);
			waitTimeList.push_back(atoi(tmpStr.c_str()));
			waitTimeListStr.erase(0, DollarPos + 1);
		}
		for (std::vector<int>::iterator itr = waitTimeList.begin(); itr != waitTimeList.end(); itr++)
		{
			CString tmpStr;
			tmpStr.Format(L"%d", *itr);
			mWaitAnswerComboCtrl.AddString(tmpStr);
		}
		mWaitDialAnswerTime = atoi(curWaitTimeOutStr);

		CString defaultwaittimeStr(curWaitTimeOutStr);
		mWaitAnswerComboCtrl.SetCurSel(mWaitAnswerComboCtrl.FindString(-1, defaultwaittimeStr));
		mSetWaitAnswerTimeOutBtn.EnableWindow(false);//Disable set button in the beginning

		SsmSetWaitAutoDialAnswerTime(mWaitDialAnswerTime);

		if (StrCmpA(triggerOBDChRangeStr, ""))
		{
			const char* delim = "-#";
			char *context;
			char *chValueStr = strtok_s(triggerOBDChRangeStr, delim, &context);
			triggerOBDRange[0] = atoi(chValueStr);
			if (chValueStr)
			{
				chValueStr = strtok_s(NULL, delim, &context);
				triggerOBDRange[1] = atoi(chValueStr);
			}
		}

		if (GetDBData() == TRUE)
		{
			//logger.log(LOGINFO, "map size: %d, vector1 size: %d", Campaigns.size(), Campaigns.size() > 0 ? Campaigns.at(1).phnumBuf.size() : 0);
			systemIpAddr = Utils::GetIPAdd();
			IsStartDialling = false;
			IsDailingTimeInRange = true;
			openCDRLogFile();
			openConsentLogFile();

			//Initialization of channels on trunk-board
			ChInfo = new CH_INFO[nTotalCh];

			for (int i = 0; i < nTotalCh; i++)
			{
				ChInfo[i].EnCalled = false;
				ChInfo[i].isChannelBlocked = IsChannelBlocked(i);
				ChInfo[i].IVRChannelNumber = -1;
				ChInfo[i].isIVRChannel = false;
				ChInfo[i].isAvailable = true;
				ChInfo[i].isApiToBeCalled = false;
				//ChInfo[i].rowTobeUpdated = false;
				int chType = SsmGetChType(i);
				if (chType == 11) //ISUP channel(China SS7 signaling ISUP)
				{
					if (SsmGetChState(i) == S_CALL_STANDBY) //check idle
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
					StrCpyA(ChInfo[i].pPhoNumBuf, EMPTY_STRING);
					StrCpyA(ChInfo[i].promptsName, EMPTY_STRING);
					StrCpyA(ChInfo[i].CDRStatus.encrypted_ani, EMPTY_STRING);
					StrCpyA(ChInfo[i].CDRStatus.dtmf, EMPTY_STRING);
					StrCpyA(ChInfo[i].CDRStatus.dtmf2, EMPTY_STRING);
					StrCpyA(ChInfo[i].CDRStatus.dtmfBuf, EMPTY_STRING);
					StrCpyA(ChInfo[i].CDRStatus.DNISBuf, EMPTY_STRING);
					StrCpyA(ChInfo[i].CDRStatus.firstConsent, EMPTY_STRING);
					StrCpyA(ChInfo[i].CDRStatus.secondConsent, EMPTY_STRING);
					StrCpyA(ChInfo[i].CDRStatus.songName, EMPTY_STRING);
					StrCpyA(ChInfo[i].CDRStatus.DNIS, EMPTY_STRING);
					StrCpyA(ChInfo[i].CDRStatus.reason, EMPTY_STRING);
					StrCpyA(ChInfo[i].CDRStatus.status, EMPTY_STRING);
					StrCpyA(ChInfo[i].CDRStatus.reason_code, EMPTY_STRING);
					ChInfo[i].InUse = false;
					ChInfo[i].InUseCount = 0;
					if (i >= nIVRMinCh && i <= nIVRMaxCh)
					{
						ChInfo[i].isIVRChannel = true;
						//ChInfo[i].InUse = false;
						continue;
					}
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
							logger.log(LOGINFO, "InitializeChannels Update data Ani : %s, Encrypted Ani: %s, Channel Num: %d", ChInfo[i].pPhoNumBuf, ChInfo[i].CDRStatus.encrypted_ani, i);
							if (!UpdatePhNumbersStatus(i))
							{
							logger.log(LOGINFO, "InitializeChannels Row not updated... channel number: %d", i);
							}
							}*/
							//setting caller ID 
							/*if (SsmSetTxCallerId(i, Campaigns.at(j).CLI) == -1)
							{
							getErrorResult(L" SsmSetTxCallerId");
							return false;
							}*/
						}
					}
					//logger.log(LOGINFO, "ch no: %d, campaign Id: %d", i, ChInfo[i].CampaignID);
				}
			}
			CWinThread* bThread = AfxBeginThread(SetChannelsStateCount, new ChCount{ nTotalCh, nIVRMinCh, nIVRMaxCh });
			if (IsSMSApiEnabled)
			{
				CWinThread* threadP = AfxBeginThread(ThreadProcApiCall, (LPVOID)&logger);
			}
			//Loading wav file on different positions for all campaigns
			//			char alias[10];
			//int index = 0;
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

void CSpiceOBDDlg::ClearChannelsGrid()
{
	for (size_t i = 0; i < nTotalCh; i++)
	{
		ChInfo[i].InUse = false;
		StrCpyA(ChInfo[i].pPhoNumBuf, EMPTY_STRING);
		StrCpyA(ChInfo[i].promptsName, EMPTY_STRING);
		StrCpyA(ChInfo[i].CDRStatus.encrypted_ani, EMPTY_STRING);
		m_TrkChList.SetItemText(i, 1, L"");
		m_TrkChList.SetItemText(i, 3, L"");
		m_TrkChList.SetItemText(i, 4, L"");
		m_TrkChList.SetItemText(i, 5, L"");
	}
}

void CSpiceOBDDlg::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	try
	{
		logger.log(LOGINFO, "On Timer Start");
		if (nIDEvent == LOOP_CTRL)
		{
			//Number should be dialled only between 8AM IST to 8:50 PM IST 
			//char dateVal[25];
			tm dateTime = logger.getTime(std::string());

			if (dateTime.tm_hour > stopTimeHour || dateTime.tm_hour < startTimeHour ||
				(dateTime.tm_hour >= stopTimeHour && dateTime.tm_min > stopTimeMin) ||
				(dateTime.tm_hour <= startTimeHour && dateTime.tm_min < startTimeMin) || dateTime.tm_hour >= 21)
			{
				IsDailingTimeInRange = false;
				BOOL isAllChannelsCleared = true;
				for (size_t campKey = 1; campKey <= Campaigns.size(); campKey++)
				{
					if (!isCampaignChannelsCleared(campKey))
					{
						isAllChannelsCleared = false;
						break;
					}
				}
				if (IsTimerOn && (isAllChannelsCleared || dateTime.tm_hour >= 21))
				{
					ClearChannelsGrid();
					OnBnClickedDiallingStop();
					KillTimer(nIDEvent);
					IsTimerOn = false;
					map<int, CampaignData>().swap(Campaigns); //Clear all data
				}
				//logger.log(LOGINFO, "Dailling false hour: %d, Minute : %d", dateTime.tm_hour, dateTime.tm_min);
			}
			else
			{
				IsDailingTimeInRange = true;
				//logger.log(LOGINFO, "Dailling true hour: %d, Minute : %d", dateTime.tm_hour, dateTime.tm_min);
			}
			DoUserWork();
			UpDateATrunkChListCtrl();
		}
		else if (nIDEvent == INITIALIZE_CTRL)
		{
			KillTimer(nIDEvent);
			logger.log(LOGINFO, "Controls Initialization start.");
			BeginWaitCursor(); //Set wait cursor
			if (!InitCtiBoard())  return;
			InitializeDBConnection();
			tm dateTime = logger.getTime(std::string());
			if (dateTime.tm_hour > stopTimeHour || dateTime.tm_hour < startTimeHour ||
				(dateTime.tm_hour >= stopTimeHour && dateTime.tm_min > stopTimeMin) ||
				(dateTime.tm_hour <= startTimeHour && dateTime.tm_min < startTimeMin) || dateTime.tm_hour >= 21)
			{
				return;
			}
			if (!InitializeChannels()) return;
			InitUserDialingList();
			BringWindowToTop();
			EndWaitCursor(); //Reset wait cursor.
			logger.log(LOGINFO, "Controls Initialized.");
			IsTimerOn = false;
			OnBnClickedDiallingStart();
		}
		logger.log(LOGINFO, "On Timer End");
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
	//logger.log(LOGFATAL, "Minimum Log level:%d", m_SetMinLogLevel);
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
	IsStartDialling = true;
	SetDiallingStartStopBtn(false);
	if (!IsTimerOn)
	{
		SetTimer(LOOP_CTRL, 200, NULL);
		/*map<int, CampaignData>().swap(Campaigns);
		logger.log(LOGINFO, "campaigns size: %d", Campaigns.size());*/
		IsTimerOn = true;
		/*CloseDBConn();
		InitializeDBConnection();*/
	}
}


void CSpiceOBDDlg::OnBnClickedDiallingStop()
{
	IsStartDialling = false;
	SetDiallingStartStopBtn(true);
}


void CSpiceOBDDlg::SetDiallingStartStopBtn(BOOL enableStart)
{
	if (enableStart) //enable start button and disable stop button
	{
		CWnd* btn = GetDlgItem(IDC_DIALLING_START);
		btn->EnableWindow(true);
		btn = GetDlgItem(IDC_DIALLING_STOP);
		btn->EnableWindow(false);
	}
	else
	{
		CWnd* btn = GetDlgItem(IDC_DIALLING_STOP);
		btn->EnableWindow(true);
		btn = GetDlgItem(IDC_DIALLING_START);
		btn->EnableWindow(false);
	}
}

void CSpiceOBDDlg::OnBnClickedSetWaitAnswerTime()
{
	char tmpWaitTimeListStr[20];
	mWaitDialAnswerTime = waitTimeList.at(mWaitAnswerComboCtrl.GetCurSel());
	SsmSetWaitAutoDialAnswerTime(mWaitDialAnswerTime);
	sprintf_s(tmpWaitTimeListStr, "%d", mWaitDialAnswerTime);
	WriteToINIFile("waitdialanswertimeout", tmpWaitTimeListStr);
	mSetWaitAnswerTimeOutBtn.EnableWindow(false);
}


void CSpiceOBDDlg::OnCbnSelchangeWaitAnswerList()
{
	mSetWaitAnswerTimeOutBtn.EnableWindow(true);
}

HBRUSH CSpiceOBDDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_RETRY_ALERT)
	{
		pDC->SetTextColor(RGB(255, 0, 0));
	}

	return hbr;
}


BOOL CSpiceOBDDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;                // Do not process further
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}
