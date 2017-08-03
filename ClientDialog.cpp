/*	File			ClientDialog.cpp
Purpose			SIMPLE TCP CLIENT (for demonstration)
Author			Richard Anthony	(ar26@gre.ac.uk)
Date			December 2003
Updated			February 2009

Outline operation:
The client sends a TCP connection request to the server
Once connected, the client displays messages sent by the server

This is skeletal code with no frills.
It is intended to provide a minimalistic TCP demonstration
(and thus hopefully is quite easy to follow).
Students following "Systems Programming" can use this code
as a starting point for their coursework application
*/

#include "stdafx.h"
#include "ClientDialog.h"
#include <sys/types.h>
#include <sys/timeb.h>


IMPLEMENT_DYNAMIC(CClientDialog, CDialog)
CClientDialog::CClientDialog(CWnd* pParent /*=NULL*/) // Class Constructor
: CDialog(CClientDialog::IDD, pParent)
, m_Timer(_T(""))
{
	m_bTimerSet = false;

	//UDP
	// Create the Send Socket
	if (!CreateSocket())
	{	// Exit the application if the socket could not be created
		this->OnCancel();
	}

	// Set up the socket address structure with the local address
	SetUpLocalAddressStruct();

	// Bind to the local address
	if (!BindToLocalAddress())
	{	// Exit the application if the bind was not successful
		this->OnCancel();
	}

}


bool CClientDialog::CreateSocket()
{
	m_ReceiveSOCKET = socket(AF_INET, SOCK_DGRAM, PF_UNSPEC);
	if (INVALID_SOCKET == m_ReceiveSOCKET)
	{
		MessageBox("Could not create socket", "Simple Receive Dialog");
		return false;
	}

	// Make the socket operate in 'Non Blocking' mode
	unsigned long lNonBlocking = 1;
	int iError = ioctlsocket(m_ReceiveSOCKET, FIONBIO, &lNonBlocking);
	if (SOCKET_ERROR == iError)
	{
		MessageBox("Could not set socket mode to NonBlocking", "Simple Receive Dialog");
		return false;
	}

	return true;
}



CClientDialog::~CClientDialog() // Class Destructor
{
	closesocket(m_ReceiveSOCKET);
}

bool CClientDialog::BindToLocalAddress()
{
	// This is where the process associates itself with a port to receive on
	int iError = bind(m_ReceiveSOCKET, (const SOCKADDR FAR*)&m_LocalSockAddr,
		sizeof(m_LocalSockAddr));
	if (SOCKET_ERROR == iError)
	{
		MessageBox("bind() Failed!", "Simple Receive Dialog");
		return false;
	}
	return true;
}

BEGIN_MESSAGE_MAP(CClientDialog, CDialog)
	ON_WM_TIMER()	// Needed if using a timer

	// User Interface Event handlers
	ON_BN_CLICKED(IDC_CONNECT, OnBnClickedConnect)
	ON_BN_CLICKED(IDC_Disconnect, OnBnClickedDisconnect)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CClientDialog::OnBnClickedButtonSend)
	ON_BN_CLICKED(IDC_Refresh, &CClientDialog::OnBnClickedRefresh)
END_MESSAGE_MAP()



void CClientDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	m_pReceiveText = (CEdit*)GetDlgItem(IDC_List);

	// Map controls onto variables in the code
	DDX_Control(pDX, IDC_MESSAGE_TEXT, m_MessageText);
	DDX_Control(pDX, IDC_MESSAGE_NUMBER, m_MessageNumber);
	DDX_Control(pDX, IDC_ADD_1, m_ConnectAddress_1);
	DDX_Control(pDX, IDC_ADD_2, m_ConnectAddress_2);
	DDX_Control(pDX, IDC_ADD_3, m_ConnectAddress_3);
	DDX_Control(pDX, IDC_ADD_4, m_ConnectAddress_4);
	DDX_Control(pDX, IDC_EDIT_PORT, m_Port);
	DDX_Control(pDX, IDC_CONNECT, m_ConnectButton);
	DDX_Control(pDX, IDC_Disconnect, m_DisconnectButton);
	DDX_Control(pDX, IDCANCEL, m_CancelButton);
	DDX_Control(pDX, IDC_EDIT_SEND, m_Send);
	DDX_Control(pDX, IDC_User, m_SendUser);


	// Initialisation of User Interface controls
	m_ConnectButton.EnableWindow(true);
	m_CancelButton.EnableWindow(true);
	m_DisconnectButton.EnableWindow(false);
	m_Port.SetWindowTextA("8001");
	m_Port.EnableWindow(true);
	m_Send.EnableWindow(false);
	

}


void CClientDialog::SetUpLocalAddressStruct()// use this code
{
	// This is where the IP address of the local machine 
	// is placed in the socket address structure
	m_LocalSockAddr.sin_addr = GetLocalHostAddressDetails();
	m_LocalSockAddr.sin_family = AF_INET;
	// This is where the port to use is decided (in this case port 8000)
	m_LocalSockAddr.sin_port = htons(8001); // Decimal 8000
}
IN_ADDR CClientDialog::GetLocalHostAddressDetails()// use this code
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
		MessageBox("Failed to get local address!", "Simple Receive Dialog");
		this->OnCancel();
	}
	return addr;
}



IN_ADDR CClientDialog::GetConnectAddress()
{
	IN_ADDR addr;

	// Use Broadcast address for simplicity in this demonstration
	addr.S_un.S_un_b.s_b1 = (unsigned  char)255;
	addr.S_un.S_un_b.s_b2 = (unsigned  char)255;
	addr.S_un.S_un_b.s_b3 = (unsigned  char)255;
	addr.S_un.S_un_b.s_b4 = (unsigned  char)255;


	CString csStr;
	m_ConnectAddress_1.GetWindowText(csStr);
	addr.S_un.S_un_b.s_b1 = (unsigned  char)atol(csStr.GetString());
	m_ConnectAddress_2.GetWindowText(csStr);
	addr.S_un.S_un_b.s_b2 = (unsigned  char)atol(csStr.GetString());
	m_ConnectAddress_3.GetWindowText(csStr);
	addr.S_un.S_un_b.s_b3 = (unsigned  char)atol(csStr.GetString());
	m_ConnectAddress_4.GetWindowText(csStr);
	addr.S_un.S_un_b.s_b4 = (unsigned  char)atol(csStr.GetString());
	return addr;
}






int CClientDialog::GetPort()
{
	CString csStr;
	int iPort;
	m_Port.GetWindowText(csStr);
	iPort = (int)atol(csStr.GetString());
	return iPort;
}

void CClientDialog::OnBnClickedConnect() // Connect Button handler
{
	m_iSocket = socket(AF_INET, SOCK_STREAM, PF_UNSPEC); // Create the (listen) socket
	if (INVALID_SOCKET == m_iSocket)
	{
		MessageBox("Socket create failed", "Client");
		CloseAndQuit();
	}

	u_long lBlocking = 0;
	int iError = ioctlsocket(m_iSocket, FIONBIO, &lBlocking); // Set blocking socket behaviour
	if (SOCKET_ERROR == iError)
	{
		MessageBox("Could not set Blocking behaviour for socket", "Client");
		CloseAndQuit();
	}

	m_ConnectSockAddr.sin_addr = GetConnectAddress();	// Setup Connect Address Structure
	m_ConnectSockAddr.sin_family = AF_INET;

	int iPort = GetPort();
	m_ConnectSockAddr.sin_port = htons(iPort);

	// Connect
	iError = connect(m_iSocket, (const SOCKADDR FAR*)&m_ConnectSockAddr, sizeof(m_ConnectSockAddr));
	if (SOCKET_ERROR == iError)
	{
		MessageBox("Connect failed !\n\nIs server address correct ?\nIs server running ?", "Client");
		closesocket(m_iSocket);		// Close the socket
		return;
	}

	// Set socket non-blocking (now that connection established) for receiving
	u_long lNonBlocking = 1;
	iError = ioctlsocket(m_iSocket, FIONBIO, &lNonBlocking);
	if (SOCKET_ERROR == iError)
	{
		MessageBox("Could not set Non Blocking behaviour for socket", "Client");
		closesocket(m_iSocket);		// Close the socket
		return;
	}

	DoReceive();	// 7. Enable receiving 
	m_ConnectButton.EnableWindow(false); // Update controls
	m_CancelButton.EnableWindow(false);
	m_Port.EnableWindow(false);
	m_DisconnectButton.EnableWindow(true);
	m_Send.EnableWindow(true);

	closesocket(m_ReceiveSOCKET);
}

void CClientDialog::InitiateTimer()
{
	m_nTimer = SetTimer(1, 100, 0);
	m_bTimerSet = true;
}

void CClientDialog::StopTimer()
{
	if (true == m_bTimerSet)
	{
		KillTimer(m_nTimer);
		m_bTimerSet = false;
	}
}

void CClientDialog::OnTimer(UINT nIDEvent)
{
	DoReceive();
}

void CClientDialog::DoReceive()
{
	StopTimer();
	int iBytesRecd;
	Message_PDU Message;

	iBytesRecd = recv(m_iSocket, (char *)&Message, sizeof(Message_PDU), 0);
	if (SOCKET_ERROR == iBytesRecd)
	{
		int iError = WSAGetLastError();
		// If no packet is queued for the socket the operation would block.
		if (iError != WSAEWOULDBLOCK)
		{
			MessageBox("Receive failed", "Client");
			CloseAndQuit();	// Simplistic - assumes connection has been lost
		}
	}
	else
	{	// Received a message
		if (Message.iMessageNumber < 0)
		{
			CloseAndQuit();	// Simplistic - assumes server has disconnected
		}///////////////////// line break
		int size = m_MessageText.GetWindowTextLength();
		m_MessageText.SetSel(size, size);
		CString linebreak = "\r\n";
		CString text = Message.szText;
		CString name = Message.Username;
		m_MessageText.ReplaceSel(name + ": " + text + linebreak);
		char szMessageNumber[15];
		_itoa_s(Message.iMessageNumber, szMessageNumber, 10);
		m_MessageNumber.SetWindowText(szMessageNumber);

	}
	InitiateTimer();
}

void CClientDialog::OnBnClickedDisconnect() // Disconnect Button handler
{
	SendMessage();
	CloseAndQuit();
}

void CClientDialog::Close()
{
	shutdown(m_iSocket, 2);	// Close the connection
	closesocket(m_iSocket);	// Close the socket
}

void CClientDialog::CloseAndQuit()
{
	Close();
	StopTimer();
	OnCancel();	// Exit the application
}

IN_ADDR CClientDialog::GetAddress()
{
	char szStr[200];
	DWORD lLen = sizeof(szStr);
	GetComputerNameA(szStr, &lLen);
	hostent * m_pHostent;
	m_pHostent = gethostbyname(szStr);

	IN_ADDR addr;
	char **ppChar = m_pHostent->h_addr_list;
	char * pChar;
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
		addr.S_un.S_addr = -1;	// signal error
	}
	return addr;
}

void CClientDialog::SendMessage() // Send a simple message to all connected clients
{
	Message_PDU Message;
	strcpy_s(Message.szText, "_Disconnect");
	DoSend(Message);
}

void CClientDialog::DoSend(Message_PDU Message) // The actual send
{
	int iBytesSent;
	iBytesSent = send(m_iSocket, (char *)&Message, sizeof(Message_PDU), 0);
	if (SOCKET_ERROR == iBytesSent)
	{
		MessageBox("Failed to send", "Client");
	}
}
void CClientDialog::OnBnClickedButtonSend()
{
	Message_PDU Message;
	CString text, name;
	m_Send.GetWindowText(text);
	m_SendUser.GetWindowText(name);
	int i;
	for (i = 0; i < text.GetLength(); i++)// the same method to the array below. 
	{
		Message.szText[i] = text[i];
	}
	Message.szText[i] = 0;// check if it reach the end of the message
	for (i = 0; i < name.GetLength(); i++)// check if i is greater then the name character length 
	{
		Message.Username[i] = name[i];
	}
	Message.Username[i] = 0;
	DoSend(Message);
}


void CClientDialog::OnBnClickedRefresh()
{
	// This is where the process inspects its buffer to see if any messages have arrived
	int iBytesRecd = recvfrom(m_ReceiveSOCKET, (char FAR*)m_szRecvBuf,
		RECEIVE_BUFFER_SIZE, 0, NULL, NULL);
	if (SOCKET_ERROR == iBytesRecd)
	{
		MessageBox("There was no message in the buffer", "Simple Receive Dialog");
	}
	else
	{
		// iBytesRecd is the number of bytes received, i.e. the message length
		// Null terminate the message string at the end of the message,
		// before displaying it on the screen
		m_szRecvBuf[iBytesRecd] = 0; // Ensure null termination
		m_pReceiveText->SetWindowText(m_szRecvBuf);
	}
}
