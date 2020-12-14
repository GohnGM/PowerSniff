#pragma once
#include "mutex.h"

// ��������ṹ�壬��Ϊ���ж�summary output�������PLUGIN_FUNCTION_TYPE_HANDLE_DATA��Ҳ���Ǻ�̨�̣߳�ֻ���õ����ݽṹ��
// �����ui�̣߳�PLUGIN_FUNCTION_TYPE_HANDLE_CLICK����PLUGIN_FUNCTION_TYPE_HANDLE_DOUBLE��������Ҫ��ʾ��ui
enum PLUGIN_FUNCTION_TYPE {
	PLUGIN_FUNCTION_TYPE_INIT = 0,
	PLUGIN_FUNCTION_TYPE_HANDLE_DATA = 1,
	PLUGIN_FUNCTION_TYPE_HANDLE_CLICK = 2,
	PLUGIN_FUNCTION_TYPE_HANDLE_DOUBLE = 3,
};

class CPluginBase
{
public:
	CPluginBase()
	{
		wcscpy(m_plugin_filename, L"");
		m_delete = FALSE;
		wcscpy(m_plugin_summary, L"");
		m_function_type = PLUGIN_FUNCTION_TYPE_INIT;
	}
	virtual ~CPluginBase()
	{
		unload();
	}

	virtual void reload(LPCTSTR filename) = 0;
	virtual void unload() {};
	virtual void dostring(const char *script_content) {};

	virtual int handle_data(LPCTSTR protocol, void *data, int len_total, int len_data) = 0;
	virtual int handle_click(LPCTSTR protocol, void *data, int len_total, int len_data) = 0;
	virtual int handle_double(LPCTSTR protocol, void *data, int len_total, int len_data) = 0;

	void lock()
	{
		m_mutex.Lock();
	}
	void unlock()
	{
		m_mutex.Unlock();
	}

	TCHAR *getPluginPath()
	{
		return m_plugin_filename;
	}

protected:
	CMyMutex		m_mutex;

public:
	TCHAR			m_plugin_filename[MAX_PATH];
	BOOL			m_delete;			// ��ʱ���û��õ��İ������������Ҫֱ��ɾ��
	TCHAR			m_plugin_summary[256];

public:
	PLUGIN_FUNCTION_TYPE	m_function_type;		// ��ǰ��ִ���ĸ�������handle_data, handle_click, ����handle_double
};
