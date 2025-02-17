// SpiceOBDDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "mysql.h"
#include "shpa3api.h"
#include "AESEncryption.h"
#include "Logger.h"
#include "Utils.h"
#include <fstream>
#include <map>
#include <vector>
#include <Shlwapi.h>
#include "ApiHttpURL.h"
#include "afxwin.h"

#define DEFAULT_LANG_CODE "10"
#define EMPTY_STRING ""

using namespace std;

enum APP_USER_STATE {
	USER_IDLE,
	USER_DIAL_CALL,
	USER_GET_PHONE_NUM,
	USER_WAIT_DIAL_TONE,
	USER_WAIT_REMOTE_PICKUP,
	USER_TALKING,
	USER_WAIT_HANGUP
};

enum TIMER_ID {
	INITIALIZE_CTRL = 1000,
	LOOP_CTRL = 2000
};

enum //for naming prompts ex. 100.wav for WELCOME_PROMPT
{
	WELCOME_PROMPT = 100,
	CALLER_PROMPT = 200,
	INVALID_PROMPT = 300,
	NO_INPUT_PROMPT = 400,
	CALL_HANGUP_PROMPT = 500
};
enum OBD_DIAL_PLAN //Different OBD type to differentiate between campaign types
{
	Informative = 1,
	AcquisitionalOBDWith1stAnd2ndConsent, 
	//AcquisitionalOBDWithout1stConsent,
	AcquisitionalOBDWith1stConsent, 
	AcquisitionalOBDWithIVRServiceCrossPromo 
};

//different states during dt and service 
enum DT_SERVICE_STATE
{
	USER_PLAY_PRODUCT,
	USER_PLAYING_PRODUCT,
	USER_CALL_WAIT_PATCHUP,
	USER_CALL_PATCHUP,
	USER_STOP_PLAYING
};

enum MEDIA_PLAY_STATE
{
	USER_PLAY_PROMPT = 1,
	USER_PLAYING_PROMPT,
	USER_REPEAT_PROMPT,
	USER_WAIT_SECOND_CONSENT,
	CALL_HANGUP
};

enum MEDIA_PLAY_SUBSTATE
{
	PREV_TOKEN_PROMPT,
	TOKEN_PROMPT_PLAYING,
	POST_TOKEN_PROMPT
};

typedef struct
{
	char call_id[20];
	int channel;
	char groupid[30];
	char campaign_id[100];
	//char circle[20];
	char call_date[20];
	char ani[20];
	char cli[20];
	time_t call_time;
	time_t answer_time;
	time_t end_time;
	//int ring_duration;
	int answer_duration;
	int total_duration;
	int callPatch_duration;
	char reason_code[20];
	char reason[100];
	char context[30];
	char encrypted_ani[100];
	char dtmf[10];
	char dtmf2[30];
	char dtmfBuf[100];
	char DNISBuf[100];
	char firstConsent[50];
	char secondConsent[50];
	char songName[100];
	char DNIS[31];
	char retry_status[11];
	char status[11];
} CDR_STATUS;


typedef struct {
	// trunck channel vars
	bool  EnCalled;
	BOOL rowTobeUpdated;
	BOOL isChannelBlocked;
	BOOL isApiToBeCalled;
	bool isIVRChannel;
	int   lineState;
	BOOL isAvailable;
	int InUse;
	int InUseCount;
	int IVRChannelNumber; //channel number patchedup
						  //char DtmfBuf[251];
	DT_SERVICE_STATE DtServiceState;
	MEDIA_PLAY_SUBSTATE mediaPlaySubState;
	int ConsentState;
	int nextConsentState;
	//int DTCounter;
	int repeatCounter;
	int levelNumber;
	CDR_STATUS CDRStatus;
	OBD_DIAL_PLAN DialPlanStatus;
	int obdType;
	// user channel  vars
	APP_USER_STATE	nStep;
	int mediaState;
	int	nToTrkCh;
	char pPhoNumBuf[31];
	char promptsName[31];
	int	nTimeOut;
	int CampaignID;
}CH_INFO;

typedef struct
{
	char promptsName[50];
	char encryptedAni[31];
	int priority;
	int obdType;
}pnNumWithEncryptedAni;

typedef struct
{
	std::string dtmfPromoCode;
	std::string langCode;
}DTMFWiseData;

typedef struct
{
	map<std::string, DTMFWiseData> dtmfWiseData;
	char patchDnis[31];
	char levelType[15];
	char cgLevel[4];
	char noKeyLevel[4];
	char invalidKeyLevel[4];
}SongsRepeatLevelInfo;

typedef struct
{
	map<int, SongsRepeatLevelInfo> tblSongsMaster;
	vector<pnNumWithEncryptedAni> phnumBuf;
	vector<std::string> cliList;
	//char CLI[31];
	char testCallNumber[31];
	int testCallCounter;
	int tmpCallCounter;
	BOOL testCallflag;
	char first_consent_digit[5];
	int channelsAllocated;
	BOOL isCampaignCompleted;
	char promptsDirectory[100];
	char campaign_id[100];
	char campaign_name[50];
	char cgShortCode[20];
	BOOL enableSMSFlag;
	char promptsPath[10][100];
	OBD_DIAL_PLAN obdDialPlan;
	int minCh, maxCh;
	int curRetryCount;
}CampaignData;

// CSpiceOBDDlg dialog
class CSpiceOBDDlg : public CDialogEx
{
private:
	MYSQL* conn, *connBase, *connSelect, *connInsert, *connUpdate, *connPort, *connCallProc;
	MYSQL_RES *res;
	MYSQL_ROW row;
	void SetDiallingStartStopBtn(BOOL enableStart);
public:
	static const int MaxPromptsRepeatsCount;
	WORD	nTotalCh;
	int nIVRMinCh, nIVRMaxCh, tempIVRMinCh, nIVRMinChNew, nIVRMaxChNew;//, CGMaxCHNum;
	int contestMinCh, contestMaxCh, tmpContestMinCh;
	int triggerOBDRange[2];
	char circle[20], circleLrn[5]; //, zone[20];
								   //Data stored in DBSettings.INI file
	char InitDBSettings[260];
	char host[255];
	char DBName[255];
	char username[255];
	char password[255];
	int port, nextCallGapDuration;
	char rvCampaign[50];
	char nameTunesPrev[50];
	char namingTunes[50];
	char nameTunesPost[50];
	char obdStartTimeStr[10], obdStopTimeStr[10];
	int startTimeMin, startTimeHour;
	int stopTimeMin, stopTimeHour;
	CH_INFO* ChInfo;
	CLogger logger;
	//BOOL isReloadConfiguration;
	static int OffSet, row_count;
	static int loopCountForCampaignUpdate;
	int mWaitDialAnswerTime;
	AESEncryption aesEncryption;
	bool IsUpdate;
	BOOL IsSMSApiEnabled;
	BOOL IsStartDialling;
	BOOL IsDailingTimeInRange;
	BOOL IsTimerOn;
	char *systemIpAddr;
	ofstream outfile;
	ofstream ConsentFile;
	map<int, CampaignData> Campaigns;

	struct ChCount
	{
		int nTotalCh;
		int nIVRMinCh;
		int nIVRMaxCh;
	};
	static UINT SetChannelsStateCount(LPVOID  chCount);
	static UINT ThreadProcApiCall(LPVOID);

	struct DeallocateProcParam
	{
		CSpiceOBDDlg * spiceDlg;
		char* campaign_id;
	};

	static UINT CallProcedure(LPVOID deallocateProcParam);
	static BOOL isDeallocateProcedureCalled;

	typedef struct
	{
		int minChVal;
		int maxChVal;
	}BlockedChannelRange;
	char blockedRangeStr[100], triggerOBDChRangeStr[100], triggerOBDName[100];
	vector<BlockedChannelRange> blockedChannelsRange;
	BOOL IsChannelBlocked(int ch); //Find the channel is in blocked range
	BOOL IsPhNumCalledSuccess(char* encrypted_ani);
	BOOL UpdatePhNumbersStatus(const char* campaignId, const char* encryptedAni);
	BOOL UpdateReasonInDialerBase(int ch);
	void ReadNumbersFromFiles();
	void WriteToINIFile(const char* key, const char* value);
	void GetDTMFandDNISBuffer(int ch);
	void PrepareAndHitURLApi(int ch);
	int PlayMediaFile(int ch, int promptsNumber);
	void ContinuePlayingPrompts(int ch);
	BOOL isCampaignChannelsCleared(int campaignKey);
	void ClearChannelsGrid();
	int GetAnIdleChannel(BOOL isContest);
	BOOL InitCtiBoard();
	void DoUserWork();
	void DialToIVR(int ch, BOOL isContest);
	void SetChannelInitialStatus();
	void InitUserDialingList();
	void InitializeDBConnection();
	void RefreshDBConnection(MYSQL* dbConn, const char* dbQuery);
	BOOL GetDBData();
	void GetSongsMasterData(char* campaignId, size_t campaignKey);
	BOOL UpdateDBData();
	void CloseDBConn();
	BOOL InitializeChannels();
	void LogErrorCodeAndMessage(int ch);
	void getErrorResult(LPCTSTR  ApiName);
	void LogOBDAlerts(const char* alertMessage, const char* messageType, const char* alertLevel); //some deafult value are set already
	void UpDateATrunkChListCtrl();
	BOOL GetAniTokenPromptsDetails(const char* msisdn, std::string & retTknPrompt);
	void GetNextUserData();
	void openCDRLogFile();
	void openConsentLogFile();
	void UpdateStatusAndPickNextRecords();
	char* GetReleaseErrorReason(WORD code);
	void HangupCall(int ch);
	void HangupIVRCall(int ch);
	CSpiceOBDDlg(CWnd* pParent = NULL);	// standard constructor

										// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SYNWAY_OBD_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


														// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBnClickedLogLevel();
	afx_msg void OnBnClickedDiallingStart();
	afx_msg void OnBnClickedDiallingStop();
	afx_msg void OnBnClickedSetWaitAnswerTime();
	afx_msg void OnCbnSelchangeWaitAnswerList();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_TrkChList;
	int m_SetMinLogLevel;

	static CStatic dailingValCtrl;
	static CStatic connctedValCtrl;
	static CStatic cgValCtrl;
	static CStatic totalChannelsAvlCtrl;
	static CStatic nChDownCtrl;
	static CStatic mChDownRangeVal;
	std::string waitTimeListStr;
	char curWaitTimeOutStr[20], contestChRange[20];
	std::vector<int> waitTimeList;
	CComboBox mWaitAnswerComboCtrl;
	CButton mSetWaitAnswerTimeOutBtn;
	CStatic mRetryAlertMsg;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};