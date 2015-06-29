#include "globals.h"
#include "el_omnibox.h"
#include <Richedit.h>
#include <strsafe.h>

el_omnibox::el_omnibox(std::shared_ptr<litehtml::document>& doc, HWND parent, cairo_container* container) : litehtml::html_tag(doc), m_edit(parent, container)
{
	m_hWndParent = parent;
	m_haveFocus = FALSE;
}

el_omnibox::~el_omnibox()
{

}

void el_omnibox::update_position()
{
	litehtml::position pos = litehtml::element::get_placement();
	RECT rcPos;
	rcPos.left = pos.left();
	rcPos.right = pos.right();
	rcPos.top = pos.top();
	rcPos.bottom = pos.bottom();
	m_edit.setRect(&rcPos);
}

void el_omnibox::set_url(LPCWSTR url)
{
	m_edit.setText(url);
}

void el_omnibox::draw(litehtml::uint_ptr hdc, int x, int y, const litehtml::position* clip)
{
	litehtml::html_tag::draw(hdc, x, y, clip);

	m_edit.draw((cairo_t*)hdc);
}

void el_omnibox::parse_styles(bool is_reparse)
{
	litehtml::html_tag::parse_styles(is_reparse);

	m_edit.setFont((cairo_font*)get_font(), get_color(_t("color"), true));
}

void el_omnibox::set_parent(HWND parent)
{
	m_hWndParent = parent;
	m_edit.set_parent(parent);
}

void el_omnibox::on_click()
{
	if (!m_haveFocus)
	{
		SendMessage(m_hWndParent, WM_OMNIBOX_CLICKED, 0, 0);
	}
}

void el_omnibox::SetFocus()
{
	m_edit.showCaret();
	m_edit.setSelection(0, -1);
	m_haveFocus = TRUE;
}

void el_omnibox::KillFocus()
{
	m_edit.setSelection(0, 0);
	m_edit.hideCaret();
	m_haveFocus = FALSE;
}

std::wstring el_omnibox::get_url()
{
	std::wstring str = m_edit.getText();

	if (!PathIsURL(str.c_str()))
	{
		DWORD sz = (DWORD) str.length() + 32;
		LPWSTR outUrl = new WCHAR[sz];
		HRESULT res = UrlApplyScheme(str.c_str(), outUrl, &sz, URL_APPLY_DEFAULT);
		if (res == E_POINTER)
		{
			delete outUrl;
			LPWSTR outUrl = new WCHAR[sz];
			if (UrlApplyScheme(str.c_str(), outUrl, &sz, URL_APPLY_DEFAULT) == S_OK)
			{
				str = outUrl;
			}
		}
		else if (res == S_OK)
		{
			str = outUrl;
		}
		delete outUrl;
	}

	return str;
}

BOOL el_omnibox::OnLButtonDown(int x, int y)
{
	if (have_focus())
	{
		litehtml::position pos = litehtml::element::get_placement();
		if (m_edit.in_capture() || pos.is_point_inside(x, y))
		{
			m_edit.OnLButtonDown(x, y);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL el_omnibox::OnLButtonUp(int x, int y)
{
	if (have_focus())
	{
		litehtml::position pos = litehtml::element::get_placement();
		if (m_edit.in_capture() || pos.is_point_inside(x, y))
		{
			m_edit.OnLButtonUp(x, y);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL el_omnibox::OnLButtonDblClick(int x, int y)
{
	if (have_focus())
	{
		litehtml::position pos = litehtml::element::get_placement();
		if (pos.is_point_inside(x, y))
		{
			m_edit.OnLButtonDblClick(x, y);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL el_omnibox::OnMouseMove(int x, int y)
{
	if (have_focus())
	{
		litehtml::position pos = litehtml::element::get_placement();
		if (m_edit.in_capture() || pos.is_point_inside(x, y))
		{
			m_edit.OnMouseMove(x, y);
			return TRUE;
		}
	}
	return FALSE;
}
