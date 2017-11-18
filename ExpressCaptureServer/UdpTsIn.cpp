#include "stdafx.h"
#include <stdint.h>
#include <memory.h>
#include <queue>
#include <deque>
#include <list>
#include "Dvb.h"

using namespace std;
static DWORD glpUdpThreadId;
static BOOL gThreadUdpRunning;
// Mutexes
static CRITICAL_SECTION mutex_udp;

SOCKET m_sockID;
struct sockaddr_in m_fepAddr;
int m_sock_len;
BOOL m_tox;
static ULONG m_inet_mult;
static ULONG m_inet_rmt;
static ULONG m_inet_lcl;
#define TP_SIZE 188	
UCHAR m_udp_buffer[TP_SIZE * 7 ];
static BOOL g_Ts_active;
#define u_int32 UINT32  // Unix uses u_int32

#pragma warning(disable : 4996)

int	join_source_group(int sd, u_int32 grpaddr, u_int32 srcaddr, u_int32 iaddr)
{
	struct ip_mreq_source imr;
	int res;

	imr.imr_multiaddr.s_addr = grpaddr;
	imr.imr_sourceaddr.s_addr = srcaddr;
	imr.imr_interface.s_addr = iaddr;

	if (srcaddr == 0)
		res = setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&imr, sizeof(imr));
	else
		res = setsockopt(sd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, (char *)&imr, sizeof(imr));

	return res;
}

int leave_source_group(int sd, u_int32 grpaddr, u_int32 srcaddr, u_int32 iaddr)
{
	struct ip_mreq_source imr;
	int res;

	imr.imr_multiaddr.s_addr = grpaddr;
	imr.imr_sourceaddr.s_addr = srcaddr;
	imr.imr_interface.s_addr = iaddr;
	if (srcaddr == 0)
		res = setsockopt(sd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&imr, sizeof(imr));
	else
		res = setsockopt(sd, IPPROTO_IP, IP_DROP_SOURCE_MEMBERSHIP, (char *)&imr, sizeof(imr));
	return res;
}

int create_new_socket(void) {

	m_sockID = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

	/*if (theApp.m_multicast == FALSE) {
		m_fepAddr.sin_family = AF_INET;
		m_fepAddr.sin_port = htons(theApp.m_ip_port);
		m_fepAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		m_sock_len = sizeof(struct sockaddr_in);
	}*/

	/*if (theApp.m_multicast == TRUE)*/ {
		m_fepAddr.sin_family = AF_INET;
		m_fepAddr.sin_port = htons(get_TSIn_port());// htons(10000);
		m_sock_len = sizeof(struct sockaddr_in);
		//char c_lcl[40], c_rmt[40], c_mult[40];
		//WideCharToMultiByte(CP_ACP, 0, theApp.m_ip_local, -1, c_lcl, 40, 0, 0);
		//WideCharToMultiByte(CP_ACP, 0, theApp.m_ip_remote, -1, c_rmt, 40, 0, 0);
		//WideCharToMultiByte(CP_ACP, 0, theApp.m_ip_multicast, -1, c_mult, 40, 0, 0);

		m_inet_mult = htonl(get_TSIn_ip_addr()); //inet_addr("230.0.0.10");// get_TSIn_ip_addr();// inet_addr("230.0.0.10");
		//m_inet_mult = inet_addr("230.0.0.10");
		m_inet_rmt = inet_addr("0.0.0.0");
		m_inet_lcl = get_TSIn_LocalNic();// inet_addr("0.0.0.0");//JPC
		//m_inet_lcl = inet_addr("192.168.1.89"); //ANY Fixme for definy a Local NIC
		m_fepAddr.sin_addr.s_addr =m_inet_lcl;

		BOOL reuseadd = true;
		int res=setsockopt(m_sockID, SOL_SOCKET, SO_REUSEADDR,(char *) &reuseadd, sizeof(BOOL));
		if (res != 0) AfxMessageBox("Reuse failed");

		{
			
			int Bufsize;
			int Len=sizeof(Bufsize);
			getsockopt(m_sockID, SOL_SOCKET, SO_RCVBUF,(char*) &Bufsize, &Len);
			TRACE("Recv buff=%d\n", Bufsize);
		}

		if (join_source_group(m_sockID, m_inet_mult, m_inet_rmt, m_inet_lcl) != 0)
		{
			char DebugMessage[255];
			sprintf(DebugMessage, "Socket Errror %d", WSAGetLastError());
			AfxMessageBox(DebugMessage);
		}
	}

	if (bind(m_sockID, (struct sockaddr *)&m_fepAddr, m_sock_len) < 0)
	{
		char DebugMessage[255];
		sprintf(DebugMessage, "Bind Errror %d", WSAGetLastError());
		AfxMessageBox(DebugMessage);
		closesocket(m_sockID);
		return -1;
	}
	return 0;
}

int get_udp_buffer(UCHAR *b, INT len)
{
	static INT bytes_left=0;// Bytes left in UDP frames
	static INT offset=0;// Current offset into UDP frame

	
	if (bytes_left >= len)
	{
		// More bytes available than requested
		memcpy(b, &m_udp_buffer[offset], len);
		offset += len;
		bytes_left -= len;
	}
	else
	{
		// Not enough available copy what we can
		memcpy(b, &m_udp_buffer[offset], bytes_left);
		INT to_do = len - bytes_left;// Number still requires
		INT start = bytes_left;// Where to place them
							   // get a new buffer
		bytes_left = recvfrom(m_sockID, (char*)m_udp_buffer, TP_SIZE * 7, 0, (struct sockaddr *)&m_fepAddr, &m_sock_len);
		offset = 0;// Start at the begining of the buffer
		if (bytes_left >= 0)
		{
			if (bytes_left > 0)
			{
				// Copy the remainder of the data
				memcpy(&b[start], m_udp_buffer, to_do);
				offset = to_do;
				bytes_left -= to_do;
			}
			else
				return 0;

		}
		else
		{
			// Something has gone horribly wrong
			bytes_left = 0;
			Sleep(100);
			return 0;
		}
	}
	return len;
}

void TsIn_run()
{
	if (create_new_socket()<0) {
		WSACleanup();
		return ;
	}
	g_Ts_active = TRUE;
}

void TsIn_pause()
{
	g_Ts_active = FALSE;
	closesocket(m_sockID);
}

UINT udp_thread(LPVOID pParam) {
	
	
	
	UCHAR *b;
	

	while (gThreadUdpRunning) {
		if (g_Ts_active == FALSE) {
			Sleep(10); continue;
		}
		else
			Sleep(0);
		b = alloc_tx_buff();
		while (get_udp_buffer(b, TP_SIZE) == 0)
		{
			
			Sleep(0);
		}
		if (b[0] != 0x47)
		{
			TRACE("NOT SYNC\n");
		}
		else
		{
			if (((b[1] == 0x1F) && (b[2] == 0xFF))) //remove null packets
			{
				//post_tx_buff(b); 
			}
			else
			{
				post_tx_buff(b);
			}
		}
	}
	leave_source_group(m_sockID, m_inet_mult, m_inet_rmt, m_inet_lcl);

	closesocket(m_sockID);
	WSACleanup();
	return 0;
}

int udp_init(void) {

	int n = 0;
	gThreadUdpRunning = TRUE;
	g_Ts_active = FALSE;
	WSADATA wsaData = { 0 };
	// Create and listen to the UDP socket
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	

	CreateThread(NULL, //Choose default security
		0, //Default stack size
		(LPTHREAD_START_ROUTINE)&udp_thread, //Routine to execute
		(LPVOID)&n, //Thread parameter
		0, //Immediately run the thread
		&glpUdpThreadId //Thread Id
	);
	

	return 0;
}
void udp_deinit(void) {
	gThreadUdpRunning = FALSE;
	
}