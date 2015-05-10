#pragma once
#include "TxThread.h"
#include "../containers/cairo/cairo_container.h"
#include "../containers/cairo/cairo_font.h"


#define WM_UPDATE_CONTROL	(WM_USER + 2001)
#define WM_EDIT_ACTIONKEY	(WM_USER + 2003)
#define WM_EDIT_CAPTURE		(WM_USER + 2004)

class CSingleLineEditCtrl : public CTxThread
{
private:
	cairo_container*	m_container;
	HWND				m_parent;
	std::wstring		m_text;
	cairo_font*			m_hFont;
	litehtml::web_color	m_textColor;
	int					m_lineHeight;
	int					m_caretPos;
	int					m_leftPos;
	BOOL				m_caretIsCreated;
	int					m_selStart;
	int					m_selEnd;
	BOOL				m_inCapture;
	int					m_startCapture;
	int					m_width;
	int					m_height;
	int					m_caretX;
	BOOL				m_showCaret;
	RECT				m_rcText;

public:
	CSingleLineEditCtrl(HWND parent, cairo_container* container);
	virtual ~CSingleLineEditCtrl(void);

	BOOL	OnKeyDown(WPARAM wParam, LPARAM lParam);
	BOOL	OnKeyUp(WPARAM wParam, LPARAM lParam);
	BOOL	OnChar(WPARAM wParam, LPARAM lParam);
	void	OnLButtonDown(int x, int y);
	void	OnLButtonUp(int x, int y);
	void	OnLButtonDblClick(int x, int y);
	void	OnMouseMove(int x, int y);
	void	setRect(LPRECT rcText);
	void	setText(LPCWSTR text);
	LPCWSTR getText()	{ return m_text.c_str(); }
	void	setFont(cairo_font* font, litehtml::web_color& color);
	void	draw(cairo_t* cr);
	void	setSelection(int start, int end);
	void	replaceSel(LPCWSTR text);
	void	hideCaret();
	void	showCaret();
	void	set_parent(HWND parent);
	BOOL	in_capture()
	{
		return m_inCapture;
	}

	virtual DWORD ThreadProc();
private:
	void	UpdateCarret();
	void	UpdateControl();
	void	delSelection();
	void	createCaret();
	void	destroyCaret();
	void	setCaretPos(int pos);
	void	fillSelRect(cairo_t* cr, LPRECT rcFill);
	int		getCaretPosXY(int x, int y);

	void	drawText(cairo_t* cr, LPCWSTR text, int cbText, LPRECT rcText, litehtml::web_color textColor);
	void	getTextExtentPoint(LPCWSTR text, int cbText, LPSIZE sz);
	void	set_color(cairo_t* cr, litehtml::web_color color)	{ cairo_set_source_rgba(cr, color.red / 255.0, color.green / 255.0, color.blue / 255.0, color.alpha / 255.0); }
};
