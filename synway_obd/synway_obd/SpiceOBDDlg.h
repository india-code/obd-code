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

using namespace std;

enum APP_USER_STATE {
	USER_IDLE,
	USER_DIAL_CALL,
	USER_GET_PHONE_NUM,
	USER_WAIT_DIAL_TONE,
	USER_WAIT_REMOTE_PICKUP,
	USER_TALKING,
	USER_CALL_WAIT_PATCHUP,
	USER_CALL_PATCHUP,
	USER_WAIT_HANGUP
};

enum MEDIA_STATE {
	MEDIA_IDLE,
	MEDIA_PLAYING
};

enum OBD_DIAL_PLAN
{
	Informative = 1,
	AcquisitionalOBDWith1stAnd2ndConsent, /*Promprs Index are as 1: welcome , 2: again , 3: confirm, 4: no-thanks ,5: thanks */
	//AcquisitionalOBDWithout1stConsent,
	AcquisitionalOBDWith1stConsent, /*1: welcome, 2: No input again, 3: Thanks 4: wrong*/
	AcquisitionalOBDWithIVRServiceCrossPromo, /*Promprs Index are as 1: 1st prompt , 2: 2nd prompts , 3: 3rd prompt, 4: idea jingle ,5: thanks , 6: wrong input, 7 : no input*/
	AcquisitionalOBDWith1stIVRConsentDT10, /*Promprs Index are as 1: 1st prompt , 2: thanks, 3: idea jingle, 4: wrong input, 5: no input*/
	AcquisitionalOBDWith1stIVRConsentDT20,/*Promprs Index are as 1: 1st prompt , 2: 2nd prompt , 3: idea jingle, 4: thanks, 5: wrong input, 6: no input  */
	AcquisitionalOBDWith1stIVRConsentDT30,/*Promprs Index are as 1: 1st prompt , 2: 2nd prompts , 3: 3rd prompt, 4: idea jingle ,5: thanks , 6: wrong input, 7 : no input*/
};

extern void getErrorResult(LPCTSTR  ApiName);

typedef struct
{
	char call_id[20];
	int channel;
	char groupid[30];
	char campaign_id[50];
	//char circle[20];
	char call_date[20];
	char ani[20];
	char cli[20];
	time_t call_time;
	time_t answer_time;
	time_t end_time;
	int ring_duration;
	int answer_duration;
	int total_duration;
	int callPatch_duration;
	char reason_code[20];
	char reason[100];
	char context[30];
	char encrypted_ani[100];
	char dtmf[10];
	char dtmf2[10];
	char dtmfBuf[50];
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
	bool isIVRChannel;
	int   lineState;
	BOOL isAvailable;
	int InUse;
	int IVRChannelNumber; //channel number patchedup
	//char DtmfBuf[251];
	int DtmfState;
	int ConsentState;
	int DTCounter;
	int levelNumber;
	CDR_STATUS CDRStatus;
	OBD_DIAL_PLAN DialPlanStatus;
	// user channel  vars
	APP_USER_STATE	nStep;
	int mediaState;
	int	nToTrkCh;
	char pPhoNumBuf[31];
	int	nTimeOut;
	int CampaignID;
}CH_INFO;

typedef struct
{
	char* ani;
	char encryptedAni[31];
}pnNumWithEncryptedAni;

typedef struct
{
	vector<pnNumWithEncryptedAni> phnumBuf;
	vector<std::string> cliList;
	//char CLI[31];
	char first_consent_digit[5];
	int channelsAllocated;
	BOOL isCampaignCompleted;
	char promptsDirectory[100];
	char campaign_id[50];
	char campaign_name[50];
	char promptsPath[10][100];
	OBD_DIAL_PLAN obdDialPlan;
	int minCh, maxCh;
}CampaignData;

// CSpiceOBDDlg dialog
class CSpiceOBDDlg : public CDialogEx
{
private:
	MYSQL* conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	//int i;
	void SetDiallingStartStopBtn(BOOL enableStart);

public:
	WORD	nTotalCh;
	int nIVRMinCh, nIVRMaxCh, tempIVRMinCh, CGMaxCHNum;
	char circle[20], zone[20];
	CH_INFO* ChInfo;
	CLogger logger;
	int totalPhoneNumbers;
	static int OffSet, row_count, getAndUpdateRowCount;
	AESEncryption aesEncryption;
	bool IsUpdate;
	BOOL IsStartDialling;
	BOOL IsDailingTimeInRange;
	BOOL IsTimerOn;
	char *systemIpAddr;
	ofstream outfile;
	ofstream ConsentFile;
	map<int, CampaignData> Campaigns;

	BOOL UpdatePhNumbersStatus(int ch);
	BOOL UpdateReasonInDialerBase(int ch);
	void ReadNumbersFromFiles();
	BOOL isCampaignChannelsCleared(int campaignKey);
	int GetAnIdleChannel();
	BOOL InitCtiBoard();
	void DoUserWork();
	void SetChannelInitialStatus();
	void InitUserDialingList();
	void InitilizeDBConnection();
	BOOL GetDBData();
	BOOL UpdateDBData();
	void CloseDBConn();
	BOOL InitilizeChannels();
	void LogErrorCodeAndMessage(int ch);
	void UpDateATrunkChListCtrl();
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
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_TrkChList;
	int m_SetMinLogLevel;
	afx_msg void OnBnClickedDiallingStart();
	afx_msg void OnBnClickedDiallingStop();
};