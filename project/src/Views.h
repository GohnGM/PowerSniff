/////////////////////////////////////////////////
// Views.h
//  Declaration of the CViewSimple, CViewText,
//   CViewTree, and CViewList classes

#ifndef VIEWS_H
#define VIEWS_H
#include <vector>
#include "../other/common.h"
class CMainFrame;

class CViewText : public CRichEdit
{
public:
	CViewText(void);
	virtual ~CViewText(void);
	virtual void SetFontDefaults();

protected:
	virtual void PreCreate(CREATESTRUCT &cs);
	virtual void OnInitialUpdate();

public:
	void TextClear();
	void TextAppend(COLORREF color, int textsize, TCHAR *text);
	void TextSetColor(int start, int end, COLORREF color, int textsize, BOOL italic);
	virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void SetMainFrm(CMainFrame *p);

private:
	CFont		m_Font;
	CMainFrame	*m_mainframe;
};

class CViewList : public CListView
{
public:
	CViewList();
	virtual ~CViewList();
	virtual void PreCreate(CREATESTRUCT &cs);
	virtual void OnInitialUpdate();
	void __SetColumns();

	// ���º�������������
	BOOL SetSubItem(int nItem, int nSubItem, LPCTSTR szText);
	int AddLine(const struct PcapDataDescriptor &data);
	void ScrollToBottom();
	void SetMainFrm(CMainFrame *p);
	virtual LRESULT OnNotifyReflect(WPARAM wParam, LPARAM lParam);

private:
	CMainFrame		*m_mainframe;

	// CListCtrl�ؼ�ʧȥ����ʱ�Ա��ָ���(��LVIS_DROPHILITED): 
	// http://blog.csdn.net/guoxiaobo2010/article/details/21730955
	// ע��listview������LVS_SHOWSELALWAYS����
	int			m_nSelItem;
};


#endif // VIEWS_H
