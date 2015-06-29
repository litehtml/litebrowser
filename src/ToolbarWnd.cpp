#include "globals.h"
#include "ToolbarWnd.h"
#include <WindowsX.h>
#include "BrowserWnd.h"
#include "el_omnibox.h"

#ifdef LITEHTML_UTF8
	#define str_cmp	strcmp
#else
	#define str_cmp	lstrcmp
#endif

CToolbarWnd::CToolbarWnd( HINSTANCE hInst, CBrowserWnd* parent )
{
	m_inCapture = FALSE;
	m_omnibox	= nullptr;
	m_parent	= parent;
	m_hInst		= hInst;
	m_hWnd		= NULL;

	WNDCLASS wc;
	if(!GetClassInfo(m_hInst, TOOLBARWND_CLASS, &wc))
	{
		ZeroMemory(&wc, sizeof(wc));
		wc.style          = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc    = (WNDPROC)CToolbarWnd::WndProc;
		wc.cbClsExtra     = 0;
		wc.cbWndExtra     = 0;
		wc.hInstance      = m_hInst;
		wc.hIcon          = NULL;
		wc.hCursor        = NULL/*LoadCursor(NULL, IDC_ARROW)*/;
		wc.hbrBackground  = (HBRUSH) (COLOR_WINDOW + 1);
		wc.lpszMenuName   = NULL;
		wc.lpszClassName  = TOOLBARWND_CLASS;

		RegisterClass(&wc);
	}

	m_context.load_master_stylesheet(_t("html,div,body { display: block; } head,style { display: none; }"));
}
CToolbarWnd::~CToolbarWnd(void)
{
	if (m_omnibox)
	{
		m_omnibox = nullptr;
	}
}

LRESULT CALLBACK CToolbarWnd::WndProc( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam )
{
	CToolbarWnd* pThis = NULL;
	if(IsWindow(hWnd))
	{
		pThis = (CToolbarWnd*)GetProp(hWnd, TEXT("toolbar_this"));
		if(pThis && pThis->m_hWnd != hWnd)
		{
			pThis = NULL;
		}
	}

	if(pThis || uMessage == WM_CREATE)
	{
		switch (uMessage)
		{
		case WM_EDIT_CAPTURE:
			if (wParam)
			{
				SetCapture(hWnd);
				pThis->m_inCapture = TRUE;
			}
			else
			{
				ReleaseCapture();
				pThis->m_inCapture = FALSE;
			}
			break;
		case WM_EDIT_ACTIONKEY:
			switch (wParam)
			{
			case VK_RETURN:
				{
					std::wstring url = pThis->m_omnibox->get_url();
					pThis->m_omnibox->select_all();
					pThis->m_parent->open(url.c_str());
				}
				break;
			}
			return 0;
		case WM_OMNIBOX_CLICKED:
			pThis->OnOmniboxClicked();
			break;
		case WM_UPDATE_CONTROL:
			{
				LPRECT rcDraw = (LPRECT)lParam;
				InvalidateRect(hWnd, rcDraw, FALSE);
			}
			break;
		case WM_SETCURSOR:
			pThis->update_cursor();
			break;
		case WM_ERASEBKGND:
			return TRUE;
		case WM_CREATE:
			{
				LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
				pThis = (CToolbarWnd*)(lpcs->lpCreateParams);
				SetProp(hWnd, TEXT("toolbar_this"), (HANDLE) pThis);
				pThis->m_hWnd = hWnd;
				pThis->OnCreate();
			}
			break;
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);

				simpledib::dib dib;

				dib.beginPaint(hdc, &ps.rcPaint);
				pThis->OnPaint(&dib, &ps.rcPaint);
				dib.endPaint();

				EndPaint(hWnd, &ps);
			}
			return 0;
		case WM_KILLFOCUS:
			if (pThis->m_omnibox && pThis->m_omnibox->have_focus())
			{
				pThis->m_omnibox->KillFocus();
			}
			break;
		case WM_SIZE:
			pThis->OnSize(LOWORD(lParam), HIWORD(lParam));
			return 0;
		case WM_DESTROY:
			RemoveProp(hWnd, TEXT("toolbar_this"));
			pThis->OnDestroy();
			delete pThis;
			return 0;
		case WM_MOUSEMOVE:
			{
				TRACKMOUSEEVENT tme;
				ZeroMemory(&tme, sizeof(TRACKMOUSEEVENT));
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags		= TME_QUERY;
				tme.hwndTrack	= hWnd;
				TrackMouseEvent(&tme);
				if(!(tme.dwFlags & TME_LEAVE))
				{
					tme.dwFlags		= TME_LEAVE;
					tme.hwndTrack	= hWnd;
					TrackMouseEvent(&tme);
				}
				pThis->OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			}
			return 0;
		case WM_MOUSELEAVE:
			pThis->OnMouseLeave();
			return 0;
		case WM_LBUTTONDOWN:
			pThis->OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		case WM_LBUTTONUP:
			pThis->OnLButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		case WM_KEYDOWN:
			if (pThis->m_omnibox && pThis->m_omnibox->have_focus())
			{
				if (pThis->m_omnibox->OnKeyDown(wParam, lParam))
				{
					return 0;
				}
			}
			break;
		case WM_KEYUP:
			if (pThis->m_omnibox && pThis->m_omnibox->have_focus())
			{
				if (pThis->m_omnibox->OnKeyUp(wParam, lParam))
				{
					return 0;
				}
			}
			break;
		case WM_CHAR:
			if (pThis->m_omnibox && pThis->m_omnibox->have_focus())
			{
				if (pThis->m_omnibox->OnChar(wParam, lParam))
				{
					return 0;
				}
			}
			break;
		}
	}

	return DefWindowProc(hWnd, uMessage, wParam, lParam);
}

void CToolbarWnd::render_toolbar(int width)
{
	if (m_doc)
	{
		m_doc->render(width);
		m_omnibox->update_position();
	}
}

void CToolbarWnd::update_cursor()
{
	LPCWSTR defArrow = IDC_ARROW;

	if (m_cursor == _t("pointer"))
	{
		SetCursor(LoadCursor(NULL, IDC_HAND));
	}
	else if (m_cursor == _t("text"))
	{
		SetCursor(LoadCursor(NULL, IDC_IBEAM));
	}
	else
	{
		SetCursor(LoadCursor(NULL, defArrow));
	}
}

void CToolbarWnd::OnCreate()
{

}

void CToolbarWnd::OnPaint( simpledib::dib* dib, LPRECT rcDraw )
{
	if(m_doc)
	{
		cairo_surface_t* surface = cairo_image_surface_create_for_data((unsigned char*) dib->bits(), CAIRO_FORMAT_ARGB32, dib->width(), dib->height(), dib->width() * 4);
		cairo_t* cr = cairo_create(surface);

		POINT pt;
		GetWindowOrgEx(dib->hdc(), &pt);
		if(pt.x != 0 || pt.y != 0)
		{
			cairo_translate(cr, -pt.x, -pt.y);
		}
		cairo_set_source_rgb(cr, 1, 1, 1);
		cairo_paint(cr);

		litehtml::position clip(rcDraw->left, rcDraw->top, rcDraw->right - rcDraw->left, rcDraw->bottom - rcDraw->top);
		m_doc->draw((litehtml::uint_ptr) cr, 0, 0, &clip);

		cairo_destroy(cr);
		cairo_surface_destroy(surface);
	}
}

void CToolbarWnd::OnSize( int width, int height )
{

}

void CToolbarWnd::OnDestroy()
{

}

void CToolbarWnd::create( int x, int y, int width, HWND parent )
{
#ifndef LITEHTML_UTF8
	LPWSTR html = NULL;

	HRSRC hResource = ::FindResource(m_hInst, L"toolbar.html", RT_HTML);
	if(hResource)
	{
		DWORD imageSize = ::SizeofResource(m_hInst, hResource);
		if(imageSize)
		{
			LPCSTR pResourceData = (LPCSTR) ::LockResource(::LoadResource(m_hInst, hResource));
			if(pResourceData)
			{
				html = new WCHAR[imageSize * 3];
				int ret = MultiByteToWideChar(CP_UTF8, 0, pResourceData, imageSize, html, imageSize * 3);
				html[ret] = 0;
			}
		}
	}
#else
	LPSTR html = NULL;

	HRSRC hResource = ::FindResource(m_hInst, L"toolbar.html", RT_HTML);
	if(hResource)
	{
		DWORD imageSize = ::SizeofResource(m_hInst, hResource);
		if(imageSize)
		{
			LPCSTR pResourceData = (LPCSTR) ::LockResource(::LoadResource(m_hInst, hResource));
			if(pResourceData)
			{
				html = new CHAR[imageSize + 1];
				lstrcpynA(html, pResourceData, imageSize);
				html[imageSize] = 0;
			}
		}
	}
#endif
	m_hWnd = CreateWindow(TOOLBARWND_CLASS, L"toolbar", WS_CHILD | WS_VISIBLE, x, y, width, 1, parent, NULL, m_hInst, (LPVOID) this);

	m_doc = litehtml::document::createFromString(html, this, &m_context);
	delete html;
	render_toolbar(width);
	MoveWindow(m_hWnd, x, y, width, m_doc->height(), TRUE);
}

void CToolbarWnd::make_url( LPCWSTR url, LPCWSTR basepath, std::wstring& out )
{
	out = url;
}

cairo_container::image_ptr CToolbarWnd::get_image(LPCWSTR url, bool redraw_on_ready)
{
	cairo_container::image_ptr img = cairo_container::image_ptr(new CTxDIB);
	if(!img->load(FindResource(m_hInst, url, RT_HTML), m_hInst))
	{
		img = nullptr;
	}

	return img;
}

void CToolbarWnd::set_caption( const litehtml::tchar_t* caption )
{

}

void CToolbarWnd::set_base_url( const litehtml::tchar_t* base_url )
{

}

void CToolbarWnd::link(std::shared_ptr<litehtml::document>& doc, litehtml::element::ptr el)
{

}

int CToolbarWnd::set_width( int width )
{
	if(m_doc)
	{
		render_toolbar(width);
		
		return m_doc->height();
	}
	return 0;
}

void CToolbarWnd::on_page_loaded(LPCWSTR url)
{
	if (m_omnibox)
	{
		m_omnibox->set_url(url);
	}
}

void CToolbarWnd::OnMouseMove(int x, int y)
{
	if(m_doc)
	{
		BOOL process = TRUE;
		if (m_omnibox)
		{
			m_omnibox->OnMouseMove(x, y);
		}
		if (!m_inCapture)
		{
			litehtml::position::vector redraw_boxes;
			if (m_doc->on_mouse_over(x, y, x, y, redraw_boxes))
			{
				for (litehtml::position::vector::iterator box = redraw_boxes.begin(); box != redraw_boxes.end(); box++)
				{
					RECT rcRedraw;
					rcRedraw.left = box->left();
					rcRedraw.right = box->right();
					rcRedraw.top = box->top();
					rcRedraw.bottom = box->bottom();
					InvalidateRect(m_hWnd, &rcRedraw, TRUE);
				}
				UpdateWindow(m_hWnd);
			}
		}
	}
	update_cursor();
}

void CToolbarWnd::OnMouseLeave()
{
	if(m_doc)
	{
		litehtml::position::vector redraw_boxes;
		if(m_doc->on_mouse_leave(redraw_boxes))
		{
			for(litehtml::position::vector::iterator box = redraw_boxes.begin(); box != redraw_boxes.end(); box++)
			{
				RECT rcRedraw;
				rcRedraw.left	= box->left();
				rcRedraw.right	= box->right();
				rcRedraw.top	= box->top();
				rcRedraw.bottom	= box->bottom();
				InvalidateRect(m_hWnd, &rcRedraw, TRUE);
			}
			UpdateWindow(m_hWnd);
		}
	}
}

void CToolbarWnd::OnOmniboxClicked()
{
	SetFocus(m_hWnd);
	m_omnibox->SetFocus();
}

void CToolbarWnd::OnLButtonDown(int x, int y)
{
	if(m_doc)
	{
		BOOL process = TRUE;
		if (m_omnibox && m_omnibox->OnLButtonDown(x, y))
		{
			process = FALSE;
		}
		if (process && !m_inCapture)
		{
			litehtml::position::vector redraw_boxes;
			if (m_doc->on_lbutton_down(x, y, x, y, redraw_boxes))
			{
				for (litehtml::position::vector::iterator box = redraw_boxes.begin(); box != redraw_boxes.end(); box++)
				{
					RECT rcRedraw;
					rcRedraw.left = box->left();
					rcRedraw.right = box->right();
					rcRedraw.top = box->top();
					rcRedraw.bottom = box->bottom();
					InvalidateRect(m_hWnd, &rcRedraw, TRUE);
				}
				UpdateWindow(m_hWnd);
			}
		}
	}
}

void CToolbarWnd::OnLButtonUp( int x, int y )
{
	if(m_doc)
	{
		BOOL process = TRUE;
		if (m_omnibox && m_omnibox->OnLButtonUp(x, y))
		{
			process = FALSE;
		}
		if (process && !m_inCapture)
		{
			litehtml::position::vector redraw_boxes;
			if (m_doc->on_lbutton_up(x, y, x, y, redraw_boxes))
			{
				for (litehtml::position::vector::iterator box = redraw_boxes.begin(); box != redraw_boxes.end(); box++)
				{
					RECT rcRedraw;
					rcRedraw.left = box->left();
					rcRedraw.right = box->right();
					rcRedraw.top = box->top();
					rcRedraw.bottom = box->bottom();
					InvalidateRect(m_hWnd, &rcRedraw, TRUE);
				}
				UpdateWindow(m_hWnd);
			}
		}
	}
}

struct
{
	LPCWSTR	name;
	LPCWSTR	url;
} g_bookmarks[] = 
{
	{L"DMOZ",					L"http://www.dmoz.org/"},
	{L"litehtml project",		L"https://github.com/litehtml/litehtml"},
	{L"litehtml website",		L"http://www.litehtml.com/"},
	{L"True Launch Bar",		L"http://www.truelaunchbar.com/"},
	{L"Tordex",					L"http://www.tordex.com/"},
	{L"True Paste",				L"http://www.truepaste.com/"},
	{L"Text Accelerator",		L"http://www.textaccelerator.com/"},
	{L"Wiki: Web Browser",		L"http://en.wikipedia.org/wiki/Web_browser"},
	{L"Wiki: Obama",			L"http://en.wikipedia.org/wiki/Obama"},
	{L"Code Project",			L"http://www.codeproject.com/"},

	{NULL,						NULL},
};

void CToolbarWnd::on_anchor_click( const litehtml::tchar_t* url, litehtml::element::ptr el )
{
	if(!str_cmp(url, _t("back")))
	{
		m_parent->back();
	} else if(!str_cmp(url, _t("forward")))
	{
		m_parent->forward();
	} else if(!str_cmp(url, _t("reload")))
	{
		m_parent->reload();
	} else if(!str_cmp(url, _t("bookmarks")))
	{
		litehtml::position pos = el->get_placement();
		POINT pt;
		pt.x	= pos.right();
		pt.y	= pos.bottom();
		MapWindowPoints(m_hWnd, NULL, &pt, 1);

		HMENU hMenu = CreatePopupMenu();

		for(int i = 0; g_bookmarks[i].url; i++)
		{
			InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING, i + 1, g_bookmarks[i].name);
		}

		int ret = TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, m_hWnd, NULL);
		DestroyMenu(hMenu);

		if(ret)
		{
			m_parent->open(g_bookmarks[ret - 1].url);
		}
	} else if(!str_cmp(url, _t("settings")))
	{
		litehtml::position pos = el->get_placement();
		POINT pt;
		pt.x	= pos.right();
		pt.y	= pos.bottom();
		MapWindowPoints(m_hWnd, NULL, &pt, 1);

		HMENU hMenu = CreatePopupMenu();

		InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING,	1, L"Calculate Render Time");
		InsertMenu(hMenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, L"");
		InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING,	2, L"Exit");

		int ret = TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, m_hWnd, NULL);
		DestroyMenu(hMenu);

		switch(ret)
		{
		case 2:
			PostQuitMessage(0);
			break;
		case 1:
			m_parent->calc_time();
			break;
		}
	}
}

void CToolbarWnd::set_cursor( const litehtml::tchar_t* cursor )
{
	m_cursor = cursor;
}

std::shared_ptr<litehtml::element> CToolbarWnd::create_element(const litehtml::tchar_t* tag_name, const litehtml::string_map& attributes, std::shared_ptr<litehtml::document>& doc)
{
	if (!t_strcasecmp(tag_name, _t("input")))
	{
		auto iter = attributes.find(_t("type"));
		if (iter != attributes.end())
		{
			if (!t_strcasecmp(iter->second.c_str(), _t("text")))
			{
				if (m_omnibox)
				{
					m_omnibox = nullptr;
				}

				m_omnibox = std::make_shared<el_omnibox>(doc, m_hWnd, this);
				return m_omnibox;
			}
		}
	}
	return 0;
}

void CToolbarWnd::import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl)
{

}

void CToolbarWnd::get_client_rect( litehtml::position& client ) const
{
	RECT rcClient;
	GetClientRect(m_hWnd, &rcClient);
	client.x		= rcClient.left;
	client.y		= rcClient.top;
	client.width	= rcClient.right - rcClient.left;
	client.height	= rcClient.bottom - rcClient.top;
}
