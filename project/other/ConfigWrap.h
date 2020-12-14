#pragma once
#include <vector>

#define REGIST_KEY		L"PowerSniff"

class CRegistryConfig
{
private:
	CRegistryConfig();
	virtual ~CRegistryConfig();

public:
	static CRegistryConfig *getInstance();
	BOOL load();
	BOOL save();			// �˳�����������ʱ����

public:
	// Theme Settings��vector����Ϊ2����ʾ��������״̬��
	std::vector<UINT>	m_vBandIDs;
	std::vector<UINT>	m_vBandStyles;
	std::vector<UINT>	m_vBandSizes;

	// Program Settings����������
	BOOL			m_bDebug;			// �Ƿ�������Ե�xtrace�����������ʹ�ã�
	BOOL			m_bAutoScroll;			// �Ƿ��Զ�����listview����ײ�
	TCHAR			m_strSelectedPlugin[256];	// ��ǰѡ���˲�����ļ�������ȫ·��
								// �����ֻ����plugin�ļ����£�

	int			m_nFilterProtocol;		// ����Э�飺TCP, UDP, ICMP
	int			m_nFilterIPType;		// ����IP����
	int			m_nFilterIP;			// ����IP
	int			m_nFilterPortType;		// ���˶˿�����
	int			m_nFilterPort;			// ���˶˿�
	int			m_nUsedAdapterMask;		// ��������
	BOOL			m_bValidDataCheck;		// ��Ч���ݳ��ȴ���0

	int			m_nWndHeightOrigData;		// ���µ����ݴ��ڵĸ߶�
	int			m_nWndWidthPluginData;		// ���µĽ��������������ڵĿ��
};
