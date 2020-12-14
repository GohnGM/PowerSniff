///////////////////////////////////////////////////////
// Mainfrm.h
//  Declaration of the CMainFrame class

#ifndef MAINFRM_H
#define MAINFRM_H

#include <vector>
#include <list>
#include "PowerSniffPanes.h"
#include "MyStatusBar.h"
#include "ConfigDialog.h"
#include "AboutDialog.h"
#include "CompileDialogLua.h"
#include "CompileDialogTCC.h"
#include "../other/common.h"
#include "../other/RawSock.h"
#include "../other/PcapRW.h"
#include "../other/LuaSource.h"
#include "../other/PluginBase.h"
#include "../other/mutex.h"

class CMainFrame : public CDockFrame
{
	friend class CAboutDialog;
public:
	CMainFrame(void);
	virtual ~CMainFrame();
	virtual void OnDestroy();
	virtual BOOL LoadRegistrySettings(LPCTSTR szKeyName);
	virtual BOOL SaveRegistrySettings();

	void __SetReBarColors(COLORREF clrBkGnd1, COLORREF clrBkGnd2, COLORREF clrBand1, COLORREF clrBand2);
	void __SetCustomToolbarText(LPCTSTR text);		// ����״̬�����ַ���
	void SetCurrentItemSummary(TCHAR *textcontent);		// ����listview��Plugin Summaryһ��
	void __InitPlugin(LPCTSTR filename);			// filename������path
	virtual int  OnCreate(CREATESTRUCT& rcs);
	void SetupToolBar();
	void __PluginScanner();
	virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnInitialUpdate();

public:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	BOOL OnDropFiles(LPCTSTR path);
	BOOL OnFileOpen();
	BOOL OnFileSave();
	BOOL OnFileExit();
	BOOL OnCaptureStart();
	BOOL OnCaptureStop();
	BOOL OnCaptureClear();
	BOOL OnOptions();
	BOOL OnAutoScroll();
	BOOL OnHelp();
	BOOL OnLuaCompileTest();
	BOOL OnTCCCompileTest();
	BOOL OnPluginReinit();

	void OnListItemChanged(void);
	void OnListItemDoubleClick(void);
	void __OnListItemClick(void);
	void OnChangeCursor(int start, int end);	// �����������λ�ñ仯ʱ����ʾ����������
	void __OnGetdispinfoList(NMHDR *pNMHDRt);	// virtual list�Ի�

private:
	void __DataPoolThread();
	void ProcessDataPool(BOOL bDrop);		// ���ݵĵ�һ�鴦����datapool��Ӧ���߳��У��ò������
	void ProcessDataOnUI(const struct PcapDataDescriptor &data1);		// ���ݵĵڶ��鴦��UI
	void OnTimer(UINT_PTR nIDEvent);

public:
	// �˵���listview��3��docktext
	CMenu						m_menu;
	CViewList					m_listview;
	BOOL						m_bRefreshListview;	// ��Ҫһ�����ݾ�ˢ�£��ö�ʱ������һ��
	CDockText					*m_pDockTextBottomLeftProtocolData;
	CDockText					*m_pDockTextBottomRightHexShower;
	CDockText					*m_pDockTextBottomRightPluginOutput;

private:
	CRawSocket					*m_rawsocket[32];	// ���32��������ÿ������һ������
	BOOL						m_bStartCapture;	// �Ƿ�ʼ��ץ��

	std::vector<struct PlugInDescriptor>		m_listPlugin;		// ����lua��c������б�
	int						m_nSelectPlugin;	// �û��ڲ˵�ѡ�еĲ����ţ���0��ʼ
public:
	CPluginBase					*m_pPlugin;		// ��ǰʹ�����ĸ����

private:
	int						m_nSelectListctrlItem;	// listviewѡ����
	TCHAR						*m_buffer;		// ��ʱ����

private:
	CMyMutex					m_mutex_datapool;
	std::vector<struct PcapDataDescriptor>		pcapdata_datapool;	// ��������֮�󣬶�������أ�Ȼ�����Ϸ��أ�
										// ��̨�߳��ò�������������İ���Ȼ�󶪵�pcapdata_list
										// ͨ������£����������İ�����̫��
	std::thread					*m_pDataPoolThread;	// ����������ݵ��߳�
	int						m_nDataPoolThreadID;	// ����������ݵ��߳�ID
	//CMyMutex					m_mutex_ui;		// ȫ����ui�̴߳������Բ���Ҫ��
	std::list<struct PcapDataDescriptor>		pcapdata_ui;		// ����������MAX_PACKET_NUMBER

private:
	CConfigDialog					m_ConfigDialog;
	CAboutDialog					m_AboutDialog;
	CCompileDialogLua				m_LuaCompileDialog;
	CCompileDialogTCC				m_TCCCompileDialog;
};

#define TIMER_REFRESH_STATUSBAR		1
#define TIMER_REFRESH_LISTCTRL		2

#endif

