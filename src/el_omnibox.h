#pragma once
#include "sl_edit.h"

#define WM_OMNIBOX_CLICKED	(WM_USER + 10002)

class el_omnibox : public litehtml::html_tag
{
	CSingleLineEditCtrl m_edit;
	HWND m_hWndParent;
	BOOL m_haveFocus;
public:
	el_omnibox(const std::shared_ptr<litehtml::document>& doc, HWND parent, cairo_container* container);
	~el_omnibox();

	virtual void draw(litehtml::uint_ptr hdc, int x, int y, const litehtml::position* clip, const std::shared_ptr<litehtml::render_item>& ri) override;
	virtual void parse_styles(bool is_reparse);
	virtual void on_click();

	BOOL have_focus()
	{
		return m_haveFocus;
	}
	void update_position();
	void set_url(LPCWSTR url);
	std::wstring get_url();
	void set_parent(HWND parent);
	void SetFocus();
	void KillFocus();
	void select_all()
	{
		m_edit.setSelection(0, -1);
	}
	BOOL OnKeyDown(WPARAM wParam, LPARAM lParam)
	{
		return m_edit.OnKeyDown(wParam, lParam);
	}
	BOOL OnKeyUp(WPARAM wParam, LPARAM lParam)
	{
		return m_edit.OnKeyUp(wParam, lParam);
	}
	BOOL OnChar(WPARAM wParam, LPARAM lParam)
	{
		return m_edit.OnChar(wParam, lParam);
	}
	BOOL OnLButtonDown(int x, int y);
	BOOL OnLButtonUp(int x, int y);
	BOOL OnLButtonDblClick(int x, int y);
	BOOL OnMouseMove(int x, int y);

};
