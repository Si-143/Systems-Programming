/*	File			ClientDialog.h
Purpose			SIMPLE TCP CLIENT (for demonstration)
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
#define RECEIVE_BUFFER_SIZE 100

#include "TCP_Demonstator_General.h"
#include "afxwin.h"
#include "resource.h"

class CClientDialog : public CDialog
{
	DECLARE_DYNAMIC(CClientDialog)

public:
	CClientDialog(CWnd* pParent = NULL);   // constructor
	virtual ~CClientDialog();
	IN_ADDR GetConnectAddress();
	IN_ADDR GetLocalHostAddressDetails();
	int GetPort();
	void InitiateTimer();
	void StopTimer();
	void OnTimer(UINT nIDEvent);
	void DoReceive();
	void Close();
	void CloseAndQuit();
	IN_ADDR GetAddress();
	void SendMessage();
	void DoSend(Message_PDU Message);
	void SetUpLocalAddressStruct();
	bool CClientDialog::CreateSocket();
	bool BindToLocalAddress();
	enum { IDD = IDD_TCP_CLIENT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	// Communications
	SOCKET m_iSocket;
	SOCKADDR_IN m_ConnectSockAddr;
	SOCKET m_ReceiveSOCKET;
	SOCKADDR_IN m_LocalSockAddr;
	// Flags
	bool m_bTimerSet;

	// Timer
	UINT_PTR m_nTimer;

	// Controls
	CEdit m_ConnectAddress_1;
	CEdit m_ConnectAddress_2;
	CEdit m_ConnectAddress_3;
	CEdit m_ConnectAddress_4;
	CEdit m_SocketNumber;
	CEdit m_MessageText;
	CEdit m_MessageNumber;
	CButton m_ConnectButton;
	CButton m_DisconnectButton;
	CButton m_CancelButton;
	CEdit * m_pReceiveText;
	char m_szRecvBuf[RECEIVE_BUFFER_SIZE + 1];

public:
	// Event handlers
	afx_msg void OnBnClickedConnect();
	afx_msg void OnBnClickedDisconnect();
	CEdit m_Port;
	CEdit m_Send;
	CEdit m_SendUser;
	afx_msg void OnBnClickedButtonSend();
	afx_msg void OnBnClickedRefresh();
	CString m_Timer;
	CButton m_Rock;
	CButton m_Paper;
	CButton m_Scissors;
	CEdit m_Gamescreen;
};

