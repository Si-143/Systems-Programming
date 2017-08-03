/*	File			ServerDialog.cpp
Purpose			SIMPLE TCP SERVER (for demonstration)
Author			Richard Anthony	(ar26@gre.ac.uk)
Date			December 2003
Updated			February 2009

Outline operation:
The server, once enabled, waits for TCP connection requests from clients
Multiple client connections are supported
When a client connects, the server sends a message to ALL connected clients
The message states the total number of connected clients

This is skeletal code with no frills.
It is intended to provide a minimalistic TCP demonstration
(and thus hopefully is quite easy to follow).
Students following "Systems Programming" can use this code
as a starting point for their coursework application
*/

#include "stdafx.h"
#include "ServerDialog.h"
#include <sys/types.h>
#include <sys/timeb.h>

IMPLEMENT_DYNAMIC(CServerDialog, CDialog)
CServerDialog::CServerDialog(CWnd* pParent /*=NULL*/) // Class Constructor
: CDialog(CServerDialog::IDD, pParent)
{

	// Create the Send Socket
	if (!CreateSocket())
	{	// Exit the application if the socket could not be created
		this->OnCancel();
	}

	// Set socket option for BROADCAST
	if (!SetBroadcastOption())
	{	// Exit the application if setting the Broadcast mode was not successful
		this->OnCancel();
	}

	// Set up the socket address structure
	SetUpSendAddressStruct();




	m_bTimerSet = false;

	for (int i = 0; i < MAX_CONNECTIONS; i++) // Initialise connections array
	{
		m_ConnectionArray[i].bInUse = false;
	}
	m_iMessageNumber = 0;
}

CServerDialog::~CServerDialog() // Class Destructor
{
	closesocket(m_SendSOCKET);
}

bool CServerDialog::CreateSocket()//////// UDP
{
	m_SendSOCKET = socket(AF_INET, SOCK_DGRAM, PF_UNSPEC);
	if (INVALID_SOCKET == m_SendSOCKET)
	{
		MessageBox("socket() Failed!", "Simple Send Dialog");
		return false;
	}
	return true;
}

bool CServerDialog::SetBroadcastOption()
{	// This turns on the Broadcast mode
	char cOpt[2];
	cOpt[0] = 1; // true
	cOpt[1] = 0; // null terminate the option array
	int iError = setsockopt(m_SendSOCKET, SOL_SOCKET, SO_BROADCAST,
		cOpt, sizeof(cOpt));
	if (SOCKET_ERROR == iError)
	{
		MessageBox("setsockopt() Failed!", "Simple Send Dialog");
		return false;
	}
	return true;
}

void CServerDialog::SetUpSendAddressStruct()// need this code for the address 
{
	m_SendSockAddr.sin_addr = SetUpSendAddress();
	m_SendSockAddr.sin_family = AF_INET;
	m_SendSockAddr.sin_port = htons(8001); // Decimal 8000
}

IN_ADDR CServerDialog::SetUpSendAddress()
{
	IN_ADDR addr;

	// Use Broadcast address for simplicity in this demonstration
	addr.S_un.S_un_b.s_b1 = (unsigned  char)255;
	addr.S_un.S_un_b.s_b2 = (unsigned  char)255;
	addr.S_un.S_un_b.s_b3 = (unsigned  char)255;
	addr.S_un.S_un_b.s_b4 = (unsigned  char)255;
	return addr;
}


void CServerDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	// Map controls onto variables in the code
	DDX_Control(pDX, IDC_ADDRESS, m_LocalAddress);
	DDX_Control(pDX, IDC_CONNECTED_COUNT, m_ConnectedCount);
	DDX_Control(pDX, IDC_ENABLE_SERVER, m_EnableServerButton);
	DDX_Control(pDX, ID_CLOSE, m_ShutdownServerButton);
	DDX_Control(pDX, IDCANCEL, m_CancelButton);
	DDX_Control(pDX, IDC_EDIT_PORT, m_Port);
	//DDX_Control(pDX, IDC_UserName, m_UserName);
	DDX_Control(pDX, IDC_UserName2, m_UserName);
	DDX_Control(pDX, IDC_EDIT_RECEIVED_MESSAGE, m_ReceivedMessage);
	m_pSendText = (CEdit*)GetDlgItem(IDC_ADDRESS);// broadcast the IP 
	m_pSendPort = (CEdit*)GetDlgItem(IDC_EDIT_PORT);// broadcast the Port 

	// Initialisation of User Interface controls
	m_EnableServerButton.EnableWindow(true);
	m_ShutdownServerButton.EnableWindow(false);
	m_CancelButton.EnableWindow(true);
	m_Port.EnableWindow(true);
	m_Port.SetWindowTextA("8001");
	m_ReceivedMessage.EnableWindow(false);;
	SetUpLocalAddressStruct();	// Also displays address
	CountActiveConnections();	// Updates control


}

BEGIN_MESSAGE_MAP(CServerDialog, CDialog)
	ON_WM_TIMER()	// Needed if using a timer

	// User Interface Event handlers
	ON_BN_CLICKED(IDC_ENABLE_SERVER, OnBnClickedEnableServer)
	ON_BN_CLICKED(ID_CLOSE, OnBnClickedClose)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDC_Broadcast, &CServerDialog::OnBnClickedBroadcast)
END_MESSAGE_MAP()

int CServerDialog::GetPort()
{
	CString csStr;
	int iPort;
	m_Port.GetWindowText(csStr);
	iPort = (int)atol(csStr.GetString());
	return iPort;
}

void CServerDialog::SetUpLocalAddressStruct()
{
	m_LocalSockAddr.sin_addr = GetLocalHostAddressDetails();
	m_LocalSockAddr.sin_family = AF_INET;

	int iPort = GetPort();
	m_LocalSockAddr.sin_port = htons(iPort);

	// Display the local address value
	CString csStr;
	csStr.Format("%d.%d.%d.%d",
		m_LocalSockAddr.sin_addr.S_un.S_un_b.s_b1,
		m_LocalSockAddr.sin_addr.S_un.S_un_b.s_b2,
		m_LocalSockAddr.sin_addr.S_un.S_un_b.s_b3,
		m_LocalSockAddr.sin_addr.S_un.S_un_b.s_b4);
	m_LocalAddress.SetWindowText(csStr.GetString());
}

void CServerDialog::OnBnClickedEnableServer()
{
	m_iSocket = socket(AF_INET, SOCK_STREAM, PF_UNSPEC); // 1. Create the (listen) socket
	if (INVALID_SOCKET == m_iSocket)
	{
		MessageBox("Socket create failed", "Server");
		exit(1);
	}

	u_long lNonBlocking = 1;
	int iError = ioctlsocket(m_iSocket, FIONBIO, &lNonBlocking); // 2. Set the socket NonBlocking
	if (SOCKET_ERROR == iError)
	{
		MessageBox("Could not set Non-Blocking behaviour for listen socket", "Server");
		exit(1);
	}

	SetUpLocalAddressStruct(); // 3. Set up the address structure

	int iResult = bind(m_iSocket, (const SOCKADDR FAR*)&m_LocalSockAddr, sizeof(m_LocalSockAddr));	// 4. Bind
	if (SOCKET_ERROR == iResult)
	{
		MessageBox("Bind failed !\nPort is probably in use", "Server");
		exit(1);
	}

	iResult = listen(m_iSocket, SOMAXCONN); // 5. Listen - for TCP connection requests from clients
	if (SOCKET_ERROR == iResult)
	{
		MessageBox("Listen failed");
		exit(1);
	}

	m_EnableServerButton.EnableWindow(false); // 6. Update controls on success
	m_ShutdownServerButton.EnableWindow(true);
	m_CancelButton.EnableWindow(false);
	m_Port.EnableWindow(false);
	Accept();
}

int CServerDialog::GetUnusedConnection()
{
	for (int i = 0; i < MAX_CONNECTIONS; i++)
	{
		if (m_ConnectionArray[i].bInUse == false)
		{
			return i;
		}
	}
	return -1;	// Signal that all connections are in use
}

void CServerDialog::Accept()
{
	int iIndex = GetUnusedConnection();
	if (iIndex == -1)
	{
		return;
	}

	int iRemoteAddrLen = sizeof(SOCKADDR_IN);
	bool bResult = false;
	m_ConnectionArray[iIndex].iConnectSocket = accept(m_iSocket,
		(SOCKADDR FAR*)&m_ConnectionArray[iIndex].ConnectSockAddr, &iRemoteAddrLen);
	if (INVALID_SOCKET == m_ConnectionArray[iIndex].iConnectSocket)
	{
		// If no connection is queued for the socket the operation would block.
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			MessageBox("Accept failed", "Server");
		}
	}
	else
	{
		// Set the newly-created connection socket to Non-Blocking
		u_long lNonBlocking = 1;
		int iError = ioctlsocket(m_ConnectionArray[iIndex].iConnectSocket, FIONBIO, &lNonBlocking);
		if (SOCKET_ERROR == iError)
		{
			MessageBox("Could not set Non-Blocking behaviour for connection socket", "Server");
			OnCancel();	// Exit the application
		}

		m_ConnectionArray[iIndex].bInUse = true;
		SendMessage();
	}
	InitiateTimer();	// Needed because socket is Non-Blocking 
	// The timer facilitates periodically checking 
	// for connection requests
}

int CServerDialog::CountActiveConnections()
{
	int iCount = 0;
	for (int i = 0; i < MAX_CONNECTIONS; i++)
	{
		if (m_ConnectionArray[i].bInUse == true)
		{
			iCount++;;
		}
	}
	CString csStr;
	csStr.Format("%d", iCount);
	m_ConnectedCount.SetWindowText(csStr.GetString());	// Update No. connections control
	return iCount;
}

void CServerDialog::InitiateTimer()
{
	m_nTimer = SetTimer(1, 100, 0);
	m_bTimerSet = true;
}

void CServerDialog::StopTimer()
{
	if (true == m_bTimerSet)
	{
		KillTimer(m_nTimer);
		m_bTimerSet = false;
	}
}

void CServerDialog::OnTimer(UINT nIDEvent)
{	// Every time the timer 'times out' this code runs
	StopTimer();
	DoReceive();// Receive needs to be called periodically - to check for 'disconnect' messages
	Accept();	// Accept thus gets called periodically
	// Needs to be periodic because a client may request to connect at any time

}

void CServerDialog::SendMessage() // Send a simple message to all connected clients
{
	Message_PDU Message;
	Message.iMessageNumber = ++m_iMessageNumber;
	CString Text = Message.Username;
	int i;
	for (i = 0; i < Text.GetLength(); i++)// the same method to the array below. 
	{
		Message.Username[i] = Text[i];
	}
	Message.Username[i] = 0;
	m_UserName.AddString(Message.Username);


	strcpy_s(Message.szText, "There are now ");
	int iConnections;
	iConnections = CountActiveConnections();
	char szTemp[15];
	_itoa_s(iConnections, szTemp, 10);
	strcat_s(Message.szText, szTemp);
	strcat_s(Message.szText, " client(s) connected to the server");
	DoSend(Message);



	m_UserName.SetWindowTextA(Text);

}

void CServerDialog::DoSend(Message_PDU Message) // The actual send
{
	int iBytesSent;
	int iIndex;
	for (iIndex = 0; iIndex < MAX_CONNECTIONS; iIndex++)
	{
		if (true == m_ConnectionArray[iIndex].bInUse)
		{
			iBytesSent = send(m_ConnectionArray[iIndex].iConnectSocket,
				(char *)&Message, sizeof(Message_PDU), 0);
			if (SOCKET_ERROR == iBytesSent)
			{
				MessageBox("Failed to send", "Server");
				CloseConnection(iIndex);	// Simplistic - assumes connection has been lost
			}
		}
	}
}

void CServerDialog::CloseConnection(int iConnection)
{
	if (m_ConnectionArray[iConnection].bInUse == true)
	{
		shutdown(m_ConnectionArray[iConnection].iConnectSocket, 2);	// Close the connection
		closesocket(m_ConnectionArray[iConnection].iConnectSocket);	// Close the socket
		m_ConnectionArray[iConnection].bInUse = false;
		SendMessage();
	}
}

void CServerDialog::CloseAll()
{
	for (int i = 0; i < MAX_CONNECTIONS; i++)	// Close All active connections
	{
		CloseConnection(i);
	}
	closesocket(m_iSocket); // Close the listen socket
}

void CServerDialog::OnBnClickedClose() // Shutdown Button handler
{
	CloseAll();
	OnCancel();	// Exit the application
}

void CServerDialog::OnBnClickedCancel() // Cancel Button handler
{
	CloseAll();
	OnCancel();	// Exit the application
}

IN_ADDR CServerDialog::GetLocalHostAddressDetails()
{
	// This is the most complicated aspect of addressing.
	// You probably don't need to understand this method,
	// just use it as is.
	char szStr[80];
	DWORD lLen = sizeof(szStr);
	GetComputerNameA(szStr, &lLen);
	hostent* pHost;
	pHost = gethostbyname(szStr);

	IN_ADDR addr;
	char*  pChar;
	char** ppChar = pHost->h_addr_list;

	if (ppChar != NULL && *ppChar != NULL)
	{
		pChar = *ppChar;
		addr.S_un.S_un_b.s_b1 = (unsigned  char)*pChar++;
		addr.S_un.S_un_b.s_b2 = (unsigned  char)*pChar++;
		addr.S_un.S_un_b.s_b3 = (unsigned  char)*pChar++;
		addr.S_un.S_un_b.s_b4 = (unsigned  char)*pChar;
	}
	else
	{
		MessageBox("Failed to get local address!", "Server");
		this->OnCancel();
	}
	return addr;
}

void CServerDialog::UserNameList(){

	Message_PDU Message;
	CString User = Message.Username;
	int i;
	for (i = 0; i < User.GetLength(); i++)// the same method to the array below. 
	{
		Message.Username[i] = User[i];
	}
	Message.Username[i] = 0;
	m_UserName.AddString(User);

}

void CServerDialog::UserNameList2(){
	Message_PDU Message;
	m_UserName.SetWindowTextA(Message.Username);
}

void CServerDialog::DoReceive()
{
	int iBytesRecd;
	Message_PDU Message;
	int iIndex;
	for (iIndex = 0; iIndex < MAX_CONNECTIONS; iIndex++)
	{
		if (true == m_ConnectionArray[iIndex].bInUse)
		{
			iBytesRecd = recv(m_ConnectionArray[iIndex].iConnectSocket, (char *)&Message, sizeof(Message_PDU), 0);
			if (SOCKET_ERROR == iBytesRecd)
			{
				int iError = WSAGetLastError();
				// If no packet is queued for the socket the operation would block.
				if (iError != WSAEWOULDBLOCK)
				{
					MessageBox("Receive failed", "Server");
				}
			}
			else
			{	// Received a message
				if (0 == strcmp(Message.szText, "_Disconnect"))
				{	// The message text was "Disconnect"
					CloseConnection(iIndex);
				}
				else
				{

					m_ReceivedMessage.SetWindowTextA(Message.szText);
					Message.iMessageNumber = 1;// chat
					DoSend(Message);//chat


				}


			}
		}
	}
}



void CServerDialog::OnBnClickedBroadcast()
{
	// send messages. 
	// Get the contents of the send text control
	// Truncate if send text is too big to fit into the send buffer
	m_pSendText->GetWindowText(m_szSendBuf, SEND_BUFFER_SIZE);
	m_pSendPort->GetWindowText(m_szSendBufPort, SEND_BUFFER_SIZE);
	// Remember the length of data placed in the send buffer
	m_iSendLen = (int)strlen(m_szSendBuf);
	m_iSendLenPort = (int)strlen(m_szSendBufPort);

	// Send the datagram

	int iBytesSent = sendto(m_SendSOCKET, (char FAR *)m_szSendBuf, m_iSendLen, 0,
		(const struct sockaddr FAR *)&m_SendSockAddr, sizeof(m_SendSockAddr));

	int iBytesSentPort = sendto(m_SendSOCKET, (char FAR *)m_szSendBufPort, m_iSendLenPort, 0,
		(const struct sockaddr FAR *)&m_SendSockAddr, sizeof(m_SendSockAddr));
	if (INVALID_SOCKET == iBytesSent && INVALID_SOCKET == iBytesSentPort)
	{
		MessageBox("sendto() Failed!", "Simple Send Dialog");
	}


}
