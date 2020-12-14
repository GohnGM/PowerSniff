////////////////////////////////////////////////////
// Mainfrm.cpp  - definitions for the CMainFrame class

#include "stdafx.h"
#include <time.h>
#include <thread>
#include "mainfrm.h"
#include "resource.h"
#include "MyStatusBar.h"
#include "../other/PluginLua.h"
#include "../other/PluginTCC.h"
#include "../other/ConfigWrap.h"
#include "../other/compile_time.h"

CMainFrame::CMainFrame()
{
	SetView(m_listview);
	m_bRefreshListview = FALSE;
	m_pDockTextBottomLeftProtocolData = NULL;
	m_pDockTextBottomRightHexShower = NULL;
	m_pDockTextBottomRightPluginOutput = NULL;

	for (int i = 0; i < 32; i++)
		m_rawsocket[i] = NULL;
	m_bStartCapture = FALSE;
	m_nSelectPlugin = -1;
	m_pPlugin = NULL;
	m_nSelectListctrlItem = -1;
	m_nDataPoolThreadID = -1;
	m_buffer = (TCHAR *)malloc((1 << 20) * sizeof(TCHAR));

	LoadRegistrySettings(REGIST_KEY);
}

CMainFrame::~CMainFrame()
{
	free(m_buffer);

	if (m_pDataPoolThread) {
		m_pDataPoolThread->join();
		delete m_pDataPoolThread;
		m_pDataPoolThread = NULL;
	}

	// ��������
	for (unsigned int i = 0; i < pcapdata_datapool.size(); i++) {
		free(pcapdata_datapool[i].data1);
	}
	pcapdata_datapool.clear();

	for (auto it = pcapdata_ui.begin(); it != pcapdata_ui.end(); it++) {
		free((*it).data1);
	}
	pcapdata_ui.clear();
}

void CMainFrame::OnDestroy()
{
	KillTimer(TIMER_REFRESH_STATUSBAR);
	KillTimer(TIMER_REFRESH_LISTCTRL);
	for (int i = 0; i < 32; i++) {
		if (m_rawsocket[i]) {
			m_rawsocket[i]->UnInit();
			delete m_rawsocket[i];
			m_rawsocket[i] = NULL;
		}
	}

	if (m_pPlugin) {
		m_pPlugin->unload();
		delete m_pPlugin;
		m_pPlugin = NULL;
	}

	::PostThreadMessage(m_nDataPoolThreadID, WM_QUIT, 0, 0);
	CDockFrame::OnDestroy();
}

// �ڹ��캯���е���
BOOL CMainFrame::LoadRegistrySettings(LPCTSTR szKeyName)
{
	CDockFrame::LoadRegistrySettings(szKeyName);

	CRegistryConfig *pconfig = CRegistryConfig::getInstance();
	pconfig->load();
	return TRUE;
}

// ��ܻ��Զ�����
BOOL CMainFrame::SaveRegistrySettings()
{
	CDockFrame::SaveRegistrySettings();

	CRegistryConfig *pconfig = CRegistryConfig::getInstance();
	pconfig->save();
	return TRUE;
}

void CMainFrame::__SetReBarColors(COLORREF clrBkGnd1, COLORREF clrBkGnd2, COLORREF clrBand1, COLORREF clrBand2)
{
	if (IsReBarSupported()) {
		ReBarTheme rt;
		ZeroMemory(&rt, sizeof(ReBarTheme));
		rt.UseThemes = true;
		rt.clrBkgnd1 = clrBkGnd1;
		rt.clrBkgnd2 = clrBkGnd2;
		rt.clrBand1 = clrBand1;
		rt.clrBand2 = clrBand2;
		rt.FlatStyle = true;
		rt.BandsLeft = true;
		rt.LockMenuBand = false;
		rt.RoundBorders = false;
		rt.ShortBands = false;
		rt.UseLines = false;
		{
			rt.clrBand1 = 0;
			rt.clrBand2 = 0;
		}

		SetReBarTheme(rt);
	}
}

void CMainFrame::__SetCustomToolbarText(LPCTSTR text)
{
	CStatusBar& Status = GetStatusBar();
	Status.SetPartText(0, text);
}

void CMainFrame::SetCurrentItemSummary(TCHAR *textcontent)
{
	if (!textcontent)
		return;

	if (m_pPlugin) {
		if (PLUGIN_FUNCTION_TYPE_HANDLE_DATA == m_pPlugin->m_function_type) {
			// ��̨�߳�
			wcscpy(m_pPlugin->m_plugin_summary, textcontent);
		} else if (PLUGIN_FUNCTION_TYPE_HANDLE_CLICK == m_pPlugin->m_function_type || PLUGIN_FUNCTION_TYPE_HANDLE_DOUBLE == m_pPlugin->m_function_type) {
			// ǰ̨�߳�
			if (m_nSelectListctrlItem >= 0) {
				auto it = pcapdata_ui.begin();
				advance(it, m_nSelectListctrlItem);
				wcscpy(it->plugin_summary, textcontent);

				// ˢ����ʾ��__OnGetdispinfoList()����ִ��
				m_listview.Invalidate(TRUE);
			}
		}
	}
}

void CMainFrame::__InitPlugin(LPCTSTR path)
{
	if (NULL == path || 0 == wcscmp(path, L""))
		return;

	if (m_pPlugin) {
		delete m_pPlugin;
		m_pPlugin = NULL;
	}

	// �ж���lua����c����
	int strlen = wcslen(path);
	if (_T('c') == path[strlen - 1] || _T('C') == path[strlen - 1]) {
		m_pPlugin = new CPluginTCC();
	} else {
		m_pPlugin = new CPluginLua();
	}

	m_pPlugin->reload(path);
}

int CMainFrame::OnCreate(CREATESTRUCT& rcs)
{
	// OnCreate controls the way the frame is created.
	// Overriding CFrame::Oncreate is optional.
	// Uncomment the lines below to change frame options.
	UseIndicatorStatus(FALSE);	// Don't show keyboard indicators in the StatusBar
	UseMenuStatus(FALSE);	// Don't show menu descriptions in the StatusBar
					// SetUseReBar(FALSE);				// Don't use a ReBar
					// SetUseThemes(FALSE);				// Don't use themes
					// SetUseToolBar(FALSE);			// Don't use a ToolBar
	CDockFrame::OnCreate(rcs);	// call the base class function

	if (IsReBarSupported()) {
		//Set our theme colors
		__SetReBarColors(RGB(225, 230, 255), RGB(240, 242, 250), RGB(248, 248, 248), RGB(180, 200, 230));

		StatusBarTheme sbt = { FALSE, RGB(225, 230, 255), RGB(240, 242, 250) };
		SetStatusBarTheme(sbt);

		ToolBarTheme tbt = { FALSE, RGB(180, 250, 255), RGB(140, 190, 255), RGB(150, 220, 255), RGB(80, 100, 255), RGB(127, 127, 255) };
		SetToolBarTheme(tbt);

		MenuTheme mt = { FALSE, RGB(180, 250, 255), RGB(140, 190, 255), RGB(240, 250, 255), RGB(120, 170, 220), RGB(127, 127, 255) };
		SetMenuTheme(mt);

		RecalcLayout();

		// Set the band styles and positions
		CRegistryConfig *pconfig = CRegistryConfig::getInstance();
		for (int i = 0; i < GetReBar().GetBandCount(); ++i) {
			if (i < (int)pconfig->m_vBandStyles.size()) {
				// Move the band to the correct position
				int iFrom = GetReBar().IDToIndex(pconfig->m_vBandIDs[i]);
				GetReBar().MoveBand(iFrom, i);

				// Set the band's style
				REBARBANDINFO rbbi;
				ZeroMemory(&rbbi, sizeof(REBARBANDINFO));
				rbbi.fMask = RBBIM_STYLE;
				rbbi.fStyle = pconfig->m_vBandStyles[i];
				GetReBar().SetBandInfo(i, rbbi);
			}

			if (i < (int)pconfig->m_vBandSizes.size()) {
				// Set the band's size
				REBARBANDINFO rbbi;
				ZeroMemory(&rbbi, sizeof(REBARBANDINFO));
				rbbi.fMask = RBBIM_SIZE;
				rbbi.cx = pconfig->m_vBandSizes[i];
				GetReBar().SetBandInfo(i, rbbi);
			}
		}

		// Set the MenuBar's position and gripper
		int nBand = GetReBar().GetBand(GetMenuBar());
		GetReBar().ShowGripper(nBand, true);
		nBand = GetReBar().GetBand(GetToolBar());
		GetReBar().ShowGripper(nBand, true);
	} else {
		MessageBox(L"Some Theme features are not supported on this Operating System", L"Warning", MB_ICONWARNING);
	}

	TCHAR title[128] = L"";
	std::wstring compile_time = __CAtoW(get_compile_strtime().c_str());
	wsprintf(title, L"PowerSniff-%s (compile at %s)", POWERSNIFF_VERSION, compile_time.c_str());
	SetTitle((LPCTSTR)title);

	return 0;
}

void CMainFrame::SetupToolBar()
{
	// Define the resource IDs for the toolbar
	AddToolBarButton(IDT_TOOLBAR_OPEN);
	AddToolBarButton(IDT_TOOLBAR_SAVE);
	AddToolBarButton(0);				// Separator
	AddToolBarButton(IDT_TOOLBAR_START);
	AddToolBarButton(IDT_TOOLBAR_STOP);
	AddToolBarButton(IDT_TOOLBAR_CLEAR);
	AddToolBarButton(0);				// Separator
	AddToolBarButton(IDT_TOOLBAR_OPTIONS);
	AddToolBarButton(0);				// Separator
	AddToolBarButton(IDT_TOOLBAR_ABOUT);

	// Set the three image lists for the first toolbar
	SetToolBarImages(RGB(255, 0, 255), IDB_TOOLBAR_NORM, IDB_TOOLBAR_HOT, IDB_TOOLBAR_DIS);
}

void CMainFrame::__PluginScanner()
{
	// ɨ��plugin�µ��ļ���c���Ի���lua
	TCHAR findpath[256] = L"";
	GetModuleFileName(NULL, findpath, MAX_PATH);
	TCHAR *p = wcsrchr(findpath, L'\\');
	*p = L'\0';
	wcscat(findpath, L"\\plugin\\");

	// �ֱ��.lua��.cɨ��
	TCHAR *types[] = {L"*.lua", L"*.c"};
	for (int i = 0; i < sizeof(types) / sizeof(TCHAR *); i++) {
		TCHAR tmppath[MAX_PATH] = L"";
		wcscpy(tmppath, findpath);
		wcscat(tmppath, types[i]);

		WIN32_FIND_DATA FindFileData;
		HANDLE hFind = ::FindFirstFile(tmppath, &FindFileData);
		if (INVALID_HANDLE_VALUE == hFind)
			break;

		while (TRUE) {
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			} else {
				TCHAR filename1[256] = L"";
				wcscpy(filename1, findpath);
				wcscat(filename1, FindFileData.cFileName);

				TCHAR firstline[512] = L"";
				if (0 == wcsicmp(firstline, L"powersniff")) {
					// �жϵ�һ������������У�-- powersniff plugin name: xxx
					// Ŀǰ��ʱ��������ж�
				}
				SendMessage(WM_USER_PLUGIN_ADD, (WPARAM)filename1, (LPARAM)FindFileData.cFileName);
			}

			if (!FindNextFile(hFind, &FindFileData))
				break;
		}
		::FindClose(hFind);
	}
}

LRESULT CMainFrame::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CRegistryConfig *pconfig = CRegistryConfig::getInstance();
	switch (uMsg) {
	case WM_USER_PLUGIN_ADD:
	{
		// ɨ�赽�˲���Ĵ���
		PlugInDescriptor p;
		wcscpy(p.filefullpath, (TCHAR *)wParam);
		wcscpy(p.descriptor, (TCHAR *)lParam);
		p.msg_id = m_listPlugin.size() + MENU_PLUGIN_START_ID;
		m_listPlugin.push_back(p);

		m_menu.GetSubMenu(2).GetSubMenu(5).AppendMenu(MFT_STRING, p.msg_id, p.descriptor);
		if (0 == wcscmp(pconfig->m_strSelectedPlugin, p.descriptor)) {
			// �������, ��4 ����Ϊ�и��˵���Lua Compile Test������TCC Compile Test���͸��Ϸ����Լ�������0��ʼ
			m_nSelectPlugin = m_menu.GetSubMenu(2).GetSubMenu(5).GetMenuItemCount() - 5;
			m_menu.GetSubMenu(2).GetSubMenu(5).CheckMenuRadioItem(MENU_PLUGIN_START_ID, MENU_PLUGIN_START_ID + 100
				, MENU_PLUGIN_START_ID + m_nSelectPlugin, 0);

			// ���û�ѡ���Ĳ������ʼ��
			__InitPlugin(p.descriptor);
		}
		break;
	}
	case WM_USER_DATA:
	{
		// �����pcapdata_datapool������һ֡�󣬰ѽ�����ݶ���pcapdata_list��Ȼ����ϢWM_USER_DATA��UI
		std::unique_ptr<PcapDataDescriptor> myParams(reinterpret_cast<PcapDataDescriptor*>(wParam));
		ProcessDataOnUI(*myParams);
		break;
	}
	case WM_USER_RUN_LUA:		// �����ǰ��lua��������õ�ǰ������������new�µ������ִ�����ͷ�
	case WM_USER_RUN_TCC:		// �����ǰ��tcc��������õ�ǰ������������new�µ������ִ�����ͷ�
	{
		const char *script = (const char *)wParam;
		//int script_len = (int)lParam;
		if (m_pPlugin) {
			m_pPlugin->dostring(script);
		}
		break;
	}
	case WM_DROPFILES:
	{
		TCHAR filePath[256];
		::DragQueryFile((HDROP)wParam, 0, filePath, 256);		// ��ȡ�Ϸŵ�һ���ļ��������ļ���
		OnDropFiles(filePath);
		::DragFinish((HDROP)wParam);
		break;
	}
	// CViewList::OnNotifyReflect()�����ֶ�SendMessage��������Ϣ
	case WM_NOTIFY:
	{
		NMHDR *pNMHDR = reinterpret_cast<NMHDR*>(lParam);

		if (LVN_ITEMCHANGED == pNMHDR->code) {
			OnListItemChanged();
		}
		if (NM_CLICK == pNMHDR->code && m_listview.GetSelectedCount() > 0) {
			// ��Ҫ���������Ϣ����ΪLVN_ITEMCHANGED���洦���ˣ�����
			// __OnListItemClick();
		}
		if (NM_DBLCLK == pNMHDR->code && m_listview.GetSelectedCount() > 0) {
			OnListItemDoubleClick();
		}
		break;
	}
	case WM_TIMER:
		OnTimer(wParam);
		break;
	}

	// pass unhandled messages on for default processing
	return WndProcDefault(uMsg, wParam, lParam);
}

#define MSGFLT_ADD		1
#define MSGFLT_REMOVE		2
#define WM_COPYGLOBALDATA	0x0049
BOOL __ChangeWndMessageFilterOk(UINT nMessage, BOOL bAllow)
{
	typedef BOOL(WINAPI * ChangeWindowMessageFilterOkFn)(UINT, DWORD);

	HMODULE hModUser32 = NULL;
	hModUser32 = LoadLibrary(L"user32.dll");
	if (hModUser32 == NULL) {
		return FALSE;
	}

	ChangeWindowMessageFilterOkFn pfnChangeWindowMessageFilter 
		= (ChangeWindowMessageFilterOkFn)GetProcAddress(hModUser32, "ChangeWindowMessageFilter");
	if (pfnChangeWindowMessageFilter == NULL) {
		FreeLibrary(hModUser32);
		return FALSE;
	}

	FreeLibrary(hModUser32);

	return pfnChangeWindowMessageFilter(nMessage, bAllow ? MSGFLT_ADD : MSGFLT_REMOVE);
}

void CMainFrame::OnInitialUpdate()
{
	CRegistryConfig *pconfig = CRegistryConfig::getInstance();
	DragAcceptFiles(TRUE);
	m_menu.Attach(GetMenuBar().GetMenu());
	m_listview.SetMainFrm(this);

#if 1
	DWORD dwStyle = DS_NO_UNDOCK | DS_NO_CAPTION;
	SetDockStyle(dwStyle);

	// Add the bottom pane first. It is a child of the main pane.
	m_pDockTextBottomRightHexShower = static_cast<CDockText*>(AddDockedChild(new CDockText(L"Hex Shower")
		, dwStyle | DS_DOCKED_BOTTOM
		, pconfig->m_nWndHeightOrigData));
	m_pDockTextBottomRightPluginOutput = static_cast<CDockText*>(m_pDockTextBottomRightHexShower->AddDockedChild(new CDockText(L"Plugin Output")
		, dwStyle | DS_CLIENTEDGE | DS_DOCKED_CONTAINER
		, pconfig->m_nWndHeightOrigData));
	m_pDockTextBottomRightPluginOutput->SetReadOnly(FALSE);
	m_pDockTextBottomRightHexShower->SetReadOnly(TRUE);

	// Add the bottom right pane. It is a child of the bottom pane 
	m_pDockTextBottomLeftProtocolData = static_cast<CDockText*>(m_pDockTextBottomRightHexShower->AddDockedChild(new CDockText(L"Protocol Data")
		, dwStyle | DS_DOCKED_LEFT
		, pconfig->m_nWndWidthPluginData));
	m_pDockTextBottomLeftProtocolData->SetReadOnly(TRUE);
	m_pDockTextBottomLeftProtocolData->SetHideSingleTab(TRUE);
	m_pDockTextBottomLeftProtocolData->SetMainFrm(this);
#endif

	// menu, toolbar���ְ�ť����
	GetToolBar().DisableButton(IDT_TOOLBAR_SAVE);
	GetToolBar().DisableButton(IDT_TOOLBAR_STOP);
	GetToolBar().DisableButton(IDT_TOOLBAR_CLEAR);
	m_menu.GetSubMenu(1).CheckMenuRadioItem(IDM_CAPTURE_START, IDM_CAPTURE_STOP, IDM_CAPTURE_STOP, 0);
	if (pconfig->m_bAutoScroll) {
		m_menu.GetSubMenu(2).CheckMenuItem(2, MF_BYPOSITION | MF_CHECKED);
	} else {
		m_menu.GetSubMenu(2).CheckMenuItem(2, MF_BYPOSITION | MF_UNCHECKED);
	}
	::DragAcceptFiles(GetHwnd(), TRUE);
	__ChangeWndMessageFilterOk(WM_DROPFILES, TRUE);
	__ChangeWndMessageFilterOk(WM_COPYGLOBALDATA, TRUE);
	SetTimer(TIMER_REFRESH_STATUSBAR, 100, NULL);
	SetTimer(TIMER_REFRESH_LISTCTRL, 200, NULL);

#if 0
	std::thread scanner(&CMainFrame::__PluginScanner, this);
	scanner.detach();
#else
	// ɨ��������Ҫ���߳�������
	__PluginScanner();
#endif

	// ����������ݵ��߳�
	m_pDataPoolThread = new std::thread(&CMainFrame::__DataPoolThread, this);
	std::stringstream ss;
	ss << m_pDataPoolThread->get_id();
	m_nDataPoolThreadID = std::stoi(ss.str());

	// ÿ������������һ��raw socket�߳�
	CRawSocket::EnumAdapters();
	if (CRawSocket::adapter_number > 32)
		CRawSocket::adapter_number = 32;
	for (int i = 0; i < CRawSocket::adapter_number; i++) {
		m_rawsocket[i] = new CRawSocket();
		m_rawsocket[i]->Init(i, this, [this, i, pconfig](const char *buffer, int len)->int {
			// �������û�п�ʼ
			if (!m_bStartCapture)
				return -1;

			// ����İ�������ڣ������ڲ����߳����棬������������
			if (NULL == buffer)
				return -2;

			// 16�ֽ�pcapͷ��14��macͷ, ��20�ֽ�ipͷ��������tcp/udp/icmpͷ��
			if (len <= 50)
				return -3;

			// ��ǰ�������Ƿ��
			if (0 == (pconfig->m_nUsedAdapterMask & (1 << m_rawsocket[i]->m_nAdapterID)))
				return -4;

			// ��ǰ�������Ƿ�ʹ��
			if (FALSE == m_rawsocket[i]->m_bEnabled)
				return -5;

			PcapDataDescriptor des;
			des.len = len;
			des.data1 = (unsigned char *)malloc(len);	// �ڴ���䣡����
			memcpy(des.data1, buffer, len);
			memset(des.plugin_summary, 0x0, sizeof(des.plugin_summary));

			// �ŵ���������棬֪ͨ��̨�̴߳���
			m_mutex_datapool.Lock();
			pcapdata_datapool.push_back(des);
			m_mutex_datapool.Unlock();
			::PostThreadMessage(m_nDataPoolThreadID, WM_USER_POOL, 0, 0);
			return 0;
		});
	}
}
