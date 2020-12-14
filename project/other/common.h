// �ο����£�
// https://www.boost.org/doc/libs/1_60_0/libs/smart_ptr/make_shared_array.html
#ifndef COMMON2018_H
#define COMMON2018_H
#include "../other/RawSock.h"

// ���������
struct PlugInDescriptor
{
	TCHAR				filefullpath[256];	// �ļ�ȫ·��
	TCHAR				descriptor[64];		// �ļ���������·����
	int				msg_id;			// ������ɲ˵�ʱ����Ӧ����Ϣid, ��MENU_PLUGIN_START_ID��ʼ
};

// �����"Plugin Output"���ڵ�һ���ı�������Ƕ�����vector����
struct PluginOutputItem
{
	int		r;
	int		g;
	int		b;
	int		fontsize;
	CString		text;
};

// rawsocket ��winpcapץ����һ֡����
struct PcapDataDescriptor
{
	int				len;			// ���ݳ��ȣ�����pcap֡ͷ��
	unsigned char			*data1;			// ���ݣ�������pcap֡ͷ��ץ����ʱ��malloc��ע����Ҫfree�ĵط���
	TCHAR				plugin_summary[256];	// �ű�ʹ��plugin_summary()��������һ֡���ݵõ��Ľ��

	TCHAR				protocol[8];
	int				src_addr;
	TCHAR				src_addr_str[16];
	int				dst_addr;
	TCHAR				dst_addr_str[16];

	int				src_port;
	int				dst_port;

	int				total_size;		// ���ȣ�����pcap֡ͷ
	int				data_size;		// ���ȣ������ݳ���

	int				capture_time_sec;
	int				capture_time_usec;
	TCHAR				capture_time_str[32];

	std::vector<PluginOutputItem>	plugin_output;		// ������
};

enum PROTOCOL_TYPE
{
	PROTOCOL_TYPE_ALL		= 0,
	PROTOCOL_TYPE_TCP		= 1,
	PROTOCOL_TYPE_UDP		= 2,
	PROTOCOL_TYPE_ICMP		= 3
};

#define WM_USER_PLUGIN_ADD		(WM_USER + 20001)
#define WM_USER_POOL			(WM_USER + 20002)	// ǰ��������������ݵ��߳�
#define WM_USER_DATA			(WM_USER + 20003)	// ����UI�߳�
#define WM_USER_RUN_LUA			(WM_USER + 20004)
#define WM_USER_RUN_TCC			(WM_USER + 20005)

#define MENU_PLUGIN_START_ID		0x6200			// �Զ���˵�����Ϣid

#define COLOR_MAC			RGB(0xaa, 0xaa, 0xaa)
#define COLOR_IP			RGB(0x00, 0x00, 0xff)
#define COLOR_TCP_UDP			RGB(0xff, 0x00, 0x00)
#define COLOR_OPTION			RGB(0x00, 0x80, 0x00)
#define TCP_UDP_ITALIC			FALSE

inline TCHAR *__get_protocol(void *__data, int len)
{
	unsigned char *data = (unsigned char *)__data;

	switch (data[23]) {					// 23��������Э�����͵�λ��
	case 6:
		return _T("tcp");
		break;
	case 17:
		return _T("udp");
		break;
	case 1:
		return _T("icmp");
		break;
	}

	return NULL;
}

inline int __get_data_len(void *__data, int len)
{
	unsigned char *data = (unsigned char *)__data;
	tcp_hdr *tcp_hdr1 = NULL;

	switch (data[23]) {					// 23��������Э�����͵�λ��
	case 6:
		tcp_hdr1 = (tcp_hdr *)(data + sizeof(eth_hdr) + sizeof(ip_hdr));
		return len - 14 - 20 - tcp_hdr1->thl * 4;
		break;
	case 17:
		return len - 14 - 20 - 8;
		break;
	case 1:
		return len - 14 - 20 - 4;
		break;
	}
	return 0;
}

// ������ת�ַ���
std::wstring LastErrorAsString(DWORD errorMessageID);

// �༭������ı���������ݼ�
#define SCINTILLA_MAX_BUFFER_SIZE		(256 * 1024)
#define SCINTILLA_SHOTKEY_OPEN			0x5000
#define SCINTILLA_SHOTKEY_SAVE			0x5001
#define SCINTILLA_SHOTKEY_COMPILE		0x5002
#define SCINTILLA_SHOTKEY_RUN			0x5003

#endif //MAINFRM_H

