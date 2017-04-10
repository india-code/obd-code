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
	AcquisitionalOBDWithout1stConsent,
	AcquisitionalOBDWith1stConsent /*1: welcome, 2: No input, 3: Thanks, 4: wrong*/
};
typedef struct
{
	char call_id[20];
	int channel;
	char groupid[30];
	char campaign_id[50];
	char circle[20];
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
	char dtmf[50];
	char dtmf2[50];
	char retry_status[11];
	char status[11];
} CDR_STATUS;


typedef struct {
	// trunck channel vars
	bool  EnCalled;
	BOOL rowTobeUpdated;
	int   lineState;
	int InUse;
	char DtmfBuf[251];
	int DtmfState;
	int ConsentState;
	CDR_STATUS CDRStatus;
	OBD_DIAL_PLAN DialPlanStatus;
	// user channel  vars
	APP_USER_STATE	nStep;
	int mediaState;
	int				nToTrkCh;
	char			pPhoNumBuf[31];
	int				nTimeOut;
	int CampaignID;
}CH_INFO;

typedef struct
{
	char* ani;
	char* encryptedAni;
}pnNumWithEncryptedAni;

typedef struct
{
	vector<pnNumWithEncryptedAni> phnumBuf;
	char CLI[31];
	int channelsAllocated;
	BOOL hasReachedThreshold;
	char promptsPath[255];
	char campaign_id[255];
	int loadedIndex[10];
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
	int i;
	// Construction
public:
	WORD	nTotalCh;
	CH_INFO* ChInfo;
	CLogger logger;
	int totalPhoneNumbers;
	static int OffSet, row_count, getAndUpdateRowCount;
	AESEncryption aesEncryption;
	bool IsUpdate;
	char *systemIpAddr;
	ofstream outfile;
	map<int, CampaignData> Campaigns;

	BOOL UpdatePhNumbersStatus(int ch);
	void ReadNumbersFromFiles();
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
	void UpdateStatusAndPickNextRecords();
	char* GetReleaseErrorReason(WORD code);
	void HangupCall(int ch);
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
};