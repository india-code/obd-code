
// SpiceOBDDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "mysql.h"
#include "shpa3api.h"
#include "AESEncryption.h"
#include "Logger.h"

enum APP_USER_STATE {
	USER_IDLE,
	USER_DIAL_CALL,
	USER_GET_PHONE_NUM,
	USER_WAIT_DIAL_TONE,
	USER_WAIT_REMOTE_PICKUP,
	USER_TALKING,
	USER_WAIT_HANGUP
};

typedef struct {
	// trunck channel vars
	bool  EnCalled;
	int   lineState;
	int InUse;
	char DtmfBuf[251];
	// user channel  vars
	APP_USER_STATE	nStep;
	int				nToTrkCh;
	char			pPhoNumBuf[31];
	int				nTimeOut;
}CH_INFO;

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
	static int chIndex;
	static int OffSet, row_count, getAndUpdateRowCount;
	bool IsUpdate;
	int GetAnIdleChannel();
	BOOL InitCtiBoard();
	void DoUserWork();
	void SetChannelInitialStatus();
	void InitUserDialingList();
	void InitilizeDBConnection();
	void GetDBData();
	BOOL UpdateDBData();
	void CloseDBConn();
	bool SetCLIOnChannels();
	void UpDateATrunkChListCtrl();
	void GetNextUserData();
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
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_TrkChList;
};