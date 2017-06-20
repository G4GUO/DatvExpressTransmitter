#include "stdafx.h"
#include <stdint.h>

static int g_logging;
static FILE *g_fp;
void udp_send(char *b, int len);
void udp_init(void);

void tp_log( uint8_t *tp ){
	if (g_logging == 1)
	{
		//fwrite(tp, 1, 188, g_fp);
		udp_send((char*)tp, 188);
	}
}
void start_log(void){
	char directory[250];
	if(GetCurrentDirectory(250,directory)){
		strncat_s(directory,"\\datvexpress.ts",17);
		if(fopen_s(&g_fp,directory,"wb") == 0 )
			g_logging = 1;
		else
			g_logging = 0;
	}
	udp_init();
}
void stop_log(void){
	if(g_logging) fclose(g_fp);
	g_logging = 0;
}

#pragma warning(disable : 4996)

#define u_int8_t unsigned char
#define u_int16_t unsigned int
static int m_sock;
static struct     sockaddr_in m_client;
//static char *UdpOutput;

void udp_send(char *b, int len)
{
#define BUFF_MAX_SIZE (7*188)
	static char Buffer[BUFF_MAX_SIZE];
	static int Size = 0;
	while (len>0)
	{
		if (Size + len >= BUFF_MAX_SIZE)
		{
			memcpy((char*)Buffer + Size, b, BUFF_MAX_SIZE - Size);
			b += (BUFF_MAX_SIZE - Size);
			len -= (BUFF_MAX_SIZE - Size);
			if (sendto(m_sock,(const char*) Buffer, BUFF_MAX_SIZE, 0, (struct sockaddr *) &m_client, sizeof(m_client))<0)
			{
				printf("UDP send failed\n");
			}
			Size = 0;
		}
		else
		{
			memcpy(Buffer + Size, b, len);
			b += len;
			Size += len;
			len = 0;

		}
	}
	/*
	if(sendto(m_sock, b, len, 0,(struct sockaddr *) &m_client, sizeof(m_client))<0){
	printf("UDP send failed\n");
	}*/
}



void udp_set_ip(char *ip)
{
	char text[40];
	char *add[2];
	u_int16_t sock;

	strcpy(text, ip);
	add[0] = strtok(text, ":");
	add[1] = strtok(NULL, ":");
	if (strlen(add[1]) == 0)
		sock = 1314;
	else
		sock = atoi(add[1]);
	// Construct the client sockaddr_in structure
	memset(&m_client, 0, sizeof(m_client));      // Clear struct
	m_client.sin_family = AF_INET;          // Internet/IP
	m_client.sin_addr.s_addr = inet_addr(add[0]);  // IP address
	m_client.sin_port = htons(sock);      // server socket
}
void udp_init(void)
{
	WSADATA wsaData = { 0 };
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	// Create a socket for transmitting UDP TS packets
	if ((m_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		printf("Failed to create socket\n");
		return;
	}
	udp_set_ip("234.5.5.5:10201");
}