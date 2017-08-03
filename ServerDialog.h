/*	File			ServerDialog.h
Purpose			SIMPLE TCP SERVER (for demonstration)
Author			Richard Anthony	(ar26@gre.ac.uk)
Date			December 2003
Updated			February 2009

This is skeletal code with no frills.
It is intended to provide a minimalistic TCP demonstration
(and thus hopefully is quite easy to follow).
Students following "Systems Programming" can use this code
as a starting point for their coursework application
*/

#pragma once
#include "afxwin.h"
#include "resource.h"
#include "TCP_Demonstator_General.h"

#define MAX_CONNECTIONS 50
#define SEND_BUFFER_SIZE 100

struct Connection {
	SOCKET iConnectSocket;
	SOCKADDR_IN ConnectSockAddr;
	bool bInUse;
};

class CServerDialog : public CDialog
{
	DECLARE_DYNAMIC(CServerDialog)

public:
	CServerDialog(CWnd* pParent = NULL);   // constructor
	virtual ~CServerDialog();
	void InitialiseControls();
	int GetPort();
	void SetUpLocalAddressStruct();
	IN_ADDR GetLocalHostAddressDetails();
	void InitiateTimer();					// REPEAT for non-blocking recv's
	void StopTimer();
	void OnTimer(UINT nIDEvent);
	int CountActiveConnections();
	int GetUnusedConnection();
	void CloseAll();
	void CloseConnection(int iConnection);
	void Accept();
	void SendMessage();
	void DoSend(Message_PDU Message);
	void DoReceive();
	void UserNameList();
	void UserNameList2();
	bool CreateSocket();
	bool SetBroadcastOption();
	void SetUpSendAddressStruct();
	IN_ADDR SetUpSendAddress();


	enum { IDD = IDD_TCP_SERVER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	// Controls
	CEdit m_LocalAddress;
	CEdit m_ConnectedCount;
	CButton m_EnableServerButton;
	CButton m_ShutdownServerButton;
	CButton m_CancelButton;
	CEdit * m_pSendText;
	CEdit * m_pSendPort;

	// Communications
	int m_iMessageNumber;
	SOCKET m_iSocket;
	SOCKADDR_IN m_LocalSockAddr;
	Connection m_ConnectionArray[MAX_CONNECTIONS];
	char m_szSendBuf[SEND_BUFFER_SIZE + 1];
	char m_szSendBufPort[SEND_BUFFER_SIZE + 1];
	int m_iSendLen;
	int  m_iSendLenPort;
	SOCKET m_SendSOCKET;
	SOCKADDR_IN m_SendSockAddr;


	// Flags
	bool m_bTimerSet;

	// Timer
	UINT_PTR m_nTimer;

public:
	// Event handlers
	afx_msg void OnBnClickedEnableServer();
	afx_msg void OnBnClickedClose();
	afx_msg void OnBnClickedCancel();
	CEdit m_Port;
	CEdit m_ReceivedMessage;
	CListBox m_UserName;
	afx_msg void OnBnClickedBroadcast();
};



