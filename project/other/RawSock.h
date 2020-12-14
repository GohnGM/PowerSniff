#ifndef RAW_SOCK_H
#define RAW_SOCK_H

#include <cstring>
#include <cmath>
#include <conio.h>
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <functional>
#include <thread>

// pcap�ļ���ʽ��https://www.cnblogs.com/2017Crown/p/7162303.html
#define PCAP_HEAD_SIZE		16		// �ֶ����죺pcap�ļ���ʽ��ÿ֡��ͷ
#define IPV4_ETH_HEAD_SIZE	14		// �ֶ����죺macͷ14�ֽ�

// ===============================================================
//Macͷ�����ܳ���14�ֽ�
typedef struct _eth_hdr
{
	unsigned char	dstmac[6];	//Ŀ��mac��ַ
	unsigned char	srcmac[6];	//Դmac��ַ
	unsigned short	eth_type;	//��̫������
}eth_hdr;

//IPͷ�����ܳ���20�ֽ�
typedef struct _ip_hdr
{
	unsigned char	version;	//�汾
	unsigned char	tos;		//��������
	unsigned short	tot_len;	//�ܳ���
	unsigned short	id;		//��־
	unsigned short	frag_off;	//��Ƭƫ��
	unsigned char	ttl;		//����ʱ��
	unsigned char	protocol;	//Э��
	unsigned short	chk_sum;	//�����
	struct in_addr	srcaddr;	//ԴIP��ַ
	struct in_addr	dstaddr;	//Ŀ��IP��ַ
}ip_hdr;

//TCPͷ�����ܳ���20�ֽ�
typedef struct _tcp_hdr
{
	unsigned short	src_port;	//Դ�˿ں�
	unsigned short	dst_port;	//Ŀ�Ķ˿ں�
	unsigned int	seq_no;		//���к�
	unsigned int	ack_no;		//ȷ�Ϻ�

	unsigned char	reserved_1:4;	//����6λ�е�4λ�ײ�����
	unsigned char	thl:4;		//tcpͷ������
	unsigned char	flag:6;		//6λ��־
	unsigned char	reseverd_2:2;	//����6λ�е�2λ

	unsigned short	wnd_size;	//16λ���ڴ�С
	unsigned short	chk_sum;	//16λTCP�����
	unsigned short	urgt_p;		//16Ϊ����ָ��
}tcp_hdr;

//UDPͷ�����ܳ���8�ֽ�
typedef struct _udp_hdr
{
	unsigned short	src_port;	//Զ�˿ں�
	unsigned short	dst_port;	//Ŀ�Ķ˿ں�
	unsigned short	uhl;		//udpͷ������
	unsigned short	chk_sum;	//16λudp�����
}udp_hdr;

//ICMPͷ�����ܳ���4�ֽ�
typedef struct _icmp_hdr
{
	unsigned char	icmp_type;	//����
	unsigned char	code;		//����
	unsigned short	chk_sum;	//16λ�����
}icmp_hdr; 

// ===============================================================
#define PROT_ICMP		1	// Internet Control Message [RFC792] 
#define PROT_TCP		6	// Transmission Control     [RFC793]
#define PROT_UDP		17	// User Datagram            [RFC768][JBP]

// ===============================================================
#define MAX_PACKET_NUMBER	(10000 * 1000)		// �������������˾�ѭ������
#define MAX_PACKET_SIZE		0x40000
#define MAX_ADAPTER_NUMBER	128

// ===============================================================

#define SIO_RCVALL		(IOC_IN | IOC_VENDOR | 1)

#define ULONG_BYTE4(u)		((u & 0xFF000000) >> 24)
#define ULONG_BYTE3(u)		((u & 0xFF0000) >> 16)
#define ULONG_BYTE2(u)		((u & 0xFF00) >> 8)
#define ULONG_BYTE1(u)		(u & 0xFF)

#define BYTE_L(u)		(u & 0xF)
#define BYTE_H(u)		(u >> 4)

class CMainFrame;

// �м������������м���CRawSocket����
class CRawSocket
{
public:
	// ��̬�ӿ�
	static int EnumAdapters();
	static TCHAR adapter_ip[MAX_ADAPTER_NUMBER][32];		// ����IP��ַ
	static TCHAR adapter_des[MAX_ADAPTER_NUMBER][1024];		// ���������ַ���
	static int adapter_number;					// ��������

public:
	// ÿ�����������ʵ��
	CRawSocket()
	{
		m_nAdapterID = -1;
		m_bEnabled = FALSE;

		s = INVALID_SOCKET;
		sa = { 0 };
		buffer = (BYTE *)malloc(MAX_PACKET_SIZE);
		ZeroMemory(buffer, MAX_PACKET_SIZE);

		m_pThread = NULL;
		m_pFunc = nullptr;
	};
	~CRawSocket()
	{
		::closesocket(s);
		s = INVALID_SOCKET;
		free(buffer);
		buffer = NULL;
	};

	// Init��UnInitֻ�ڳ�ʼ����ж�ص�ʱ�����һ�Σ�Start��Stop������ʱ����
	void Init(int adapter_id, CMainFrame *pFrame, std::function<int(const char *buffer, int len)> pFunc);
	void UnInit();
	void Start(void);
	void Stop(void);
	void* GetSniffData(int *len);				// �ȴ�һ֡���ݣ�������

public:
	int			m_nAdapterID;			// ����ID
	BOOL			m_bEnabled;			// �Ƿ�ʹ��ץ��

private:
	void __CaptureThread();

private:
	SOCKET			s;				// socket���
	SOCKADDR_IN		sa;				// ��ַ
	BYTE			*buffer;			// һ֡���ݣ�����Ƶ�������ڴ棩

	std::thread		*m_pThread;
	std::function<int(const char *buffer, int len)>		m_pFunc;
};
#endif
