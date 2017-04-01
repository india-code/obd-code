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
	char retry_status[11];
	char status[11];
} CDR_STATUS;


typedef struct {
	// trunck channel vars
	bool  EnCalled;
	int   lineState;
	int InUse;
	char DtmfBuf[251];
	int DtmfState;
	CDR_STATUS CDRStatus;
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
	vector<char*> phnumBuf;
	char CLI[31];
	int channelsAllocated;
	int allocatedChannels;
	char promptsPath[255];
	char campaign_id[255];
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
	static int chIndex1, chIndex2;
	static int OffSet, row_count, getAndUpdateRowCount;
	bool IsUpdate;
	char *systemIpAddr;
	ofstream outfile;
	char PhoneNumbers1[6600][31], PhoneNumbers2[8100][31];
	map<int, CampaignData> Campaigns;

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