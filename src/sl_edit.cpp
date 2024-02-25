#include "globals.h"
#include "sl_edit.h"
#include "ctrl_container.h"

CSingleLineEditCtrl::CSingleLineEditCtrl(HWND parent, windows_container* container) : m_textColor(0, 0, 0)
{
	m_parent			= parent;
	m_container			= container;
	m_leftPos			= 0;
	m_parent			= m_parent;
	m_caretPos			= 0;
	m_caretIsCreated	= FALSE;
	m_selStart			= -1;
	m_selEnd			= -1;
	m_inCapture			= FALSE;
	m_startCapture		= -1;
	m_width				= 0;
	m_height			= 0;
	m_caretX			= 0;
	m_showCaret			= TRUE;
	m_hFont				= NULL;
	m_lineHeight		= 0;
}

CSingleLineEditCtrl::~CSingleLineEditCtrl(void)
{
	Stop();
}

BOOL CSingleLineEditCtrl::OnKeyDown( WPARAM wParam, LPARAM lParam )
{
	UINT key = (UINT) wParam;
	switch(key)
	{
	case 'A': //A
		if(GetKeyState(VK_CONTROL) & 0x8000)
		{
			setSelection(0, -1);
		}
	case 'C': //C
		if(GetKeyState(VK_CONTROL) & 0x8000)
		{
			if (OpenClipboard(m_parent))
			{
				EmptyClipboard();
				std::wstring strCopy;
				if(m_selStart >= 0)
				{
					int start = std::min(m_selStart, m_selEnd);
					int end   = std::max(m_selStart, m_selEnd);
					strCopy = m_text.substr(start, end - start);
				} else
				{
					strCopy = m_text;
				}
				HGLOBAL hText = GlobalAlloc(GHND, (strCopy.length() + 1) * sizeof(TCHAR));
				LPWSTR text = (LPWSTR) GlobalLock((HGLOBAL) hText);
				lstrcpy(text, strCopy.c_str());
				GlobalUnlock(hText);
				SetClipboardData(CF_UNICODETEXT, hText);
				CloseClipboard();
			}
			return 0;
		}
		break;
	case 0x58: //X
		if(GetKeyState(VK_CONTROL) & 0x8000)
		{
			if (OpenClipboard(m_parent))
			{
				EmptyClipboard();
				std::wstring strCopy;
				if(m_selStart >= 0)
				{
					int start = std::min(m_selStart, m_selEnd);
					int end   = std::max(m_selStart, m_selEnd);
					strCopy = m_text.substr(start, end - start);
					delSelection();
				} else
				{
					strCopy = m_text;
					setText(L"");
				}
				HGLOBAL hText = GlobalAlloc(GHND, (strCopy.length() + 1) * sizeof(TCHAR));
				LPWSTR text = (LPWSTR) GlobalLock((HGLOBAL) hText);
				lstrcpy(text, strCopy.c_str());
				GlobalUnlock(hText);
				SetClipboardData(CF_UNICODETEXT, hText);
				CloseClipboard();
			}
			return 0;
		}
		break;
	case 0x56: //V
		if(GetKeyState(VK_CONTROL) & 0x8000)
		{
			if (OpenClipboard(m_parent))
			{
				HANDLE hText = GetClipboardData(CF_UNICODETEXT);
				if(hText)
				{
					LPWSTR text = (LPWSTR) GlobalLock((HGLOBAL) hText);
					replaceSel(text);
					m_caretPos += lstrlen(text);
					GlobalUnlock(hText);
					UpdateCarret();
				}
				CloseClipboard();
			}
			return 0;
		}
		break;
	case 0x2D: //Insert
		if(GetKeyState(VK_SHIFT) & 0x8000)
		{
			if (OpenClipboard(m_parent))
			{
				HANDLE hText = GetClipboardData(CF_UNICODETEXT);
				if(hText)
				{
					LPWSTR text = (LPWSTR) GlobalLock((HGLOBAL) hText);
					replaceSel(text);
					m_caretPos += lstrlen(text);
					GlobalUnlock(hText);
					UpdateCarret();
				}
				CloseClipboard();
			}
			return 0;
		}
		break;
	case VK_RETURN:
		PostMessage(m_parent, WM_EDIT_ACTIONKEY, VK_RETURN, 0);
		return 0;
	case VK_ESCAPE:
		PostMessage(m_parent, WM_EDIT_ACTIONKEY, VK_ESCAPE, 0);
		return 0;
	case VK_BACK:
		Stop();
		if(m_text.length() && m_caretPos > 0)
		{
			if(m_selStart < 0)
			{
				m_text.erase(m_caretPos - 1, 1);
				m_caretPos--;
				UpdateCarret();
			} else
			{
				delSelection();
			}
		}
		return 0;
	case VK_DELETE:
		Stop();
		if(m_selStart < 0)
		{
			if(m_caretPos < (int) m_text.length())
			{
				m_text.erase(m_caretPos, 1);
				UpdateControl();
			}
		} else
		{
			delSelection();
		}
		return 0;
	case VK_UP:
	case VK_LEFT:
		Stop();
		if(m_caretPos > 0)
		{
			int oldCaretPos = m_caretPos;
			if(GetKeyState(VK_CONTROL) & 0x8000)
			{
				int newPos = m_caretPos - 1;
				for(;newPos > 0; newPos--)
				{
					if(!_istalnum(m_text[newPos]))
					{
						break;
					}
				}
				m_caretPos = newPos;
			} else
			{
				m_caretPos--;
			}
			if(GetKeyState(VK_SHIFT) & 0x8000)
			{
				if(m_selStart >= 0)
				{
					setSelection(m_selStart, m_caretPos);
				} else
				{
					setSelection(oldCaretPos, m_caretPos);
				}
			} else
			{
				setSelection(-1, 0);
			}
			UpdateCarret();
		} else
		{
			if(!(GetKeyState(VK_SHIFT) & 0x8000))
			{
				setSelection(-1, 0);
			}
		}
		return 0;
	case VK_DOWN:
	case VK_RIGHT:
		Stop();
		if(m_caretPos < (int) m_text.length())
		{
			int oldCaretPos = m_caretPos;
			if(GetKeyState(VK_CONTROL) & 0x8000)
			{
				int newPos = m_caretPos + 1;
				for(;newPos < (int) m_text.length(); newPos++)
				{
					if(!_istalnum(m_text[newPos]))
					{
						break;
					}
				}
				m_caretPos = newPos;
			} else
			{
				m_caretPos++;
			}
			if(GetKeyState(VK_SHIFT) & 0x8000)
			{
				if(m_selStart >= 0)
				{
					setSelection(m_selStart, m_caretPos);
				} else
				{
					setSelection(oldCaretPos, m_caretPos);
				}
			} else
			{
				setSelection(-1, 0);
			}
			UpdateCarret();
		} else
		{
			if(!(GetKeyState(VK_SHIFT) & 0x8000))
			{
				setSelection(-1, 0);
			}
		}
		return 0;
	case VK_HOME:
		{
			int oldCaretPos = m_caretPos;
			m_caretPos = 0;
			UpdateCarret();

			if(GetKeyState(VK_SHIFT) & 0x8000)
			{
				if(m_selStart >= 0)
				{
					setSelection(m_selStart, m_caretPos);
				} else
				{
					setSelection(oldCaretPos, m_caretPos);
				}
			} else
			{
				setSelection(-1, 0);
			}
		}
		return 0;
	case VK_END:
		{
			int oldCaretPos = m_caretPos;
			m_caretPos = (int) m_text.length();
			UpdateCarret();

			if(GetKeyState(VK_SHIFT) & 0x8000)
			{
				if(m_selStart >= 0)
				{
					setSelection(m_selStart, m_caretPos);
				} else
				{
					setSelection(oldCaretPos, m_caretPos);
				}
			} else
			{
				setSelection(-1, 0);
			}
		}
		return 0;
	}
	return TRUE;
}

BOOL CSingleLineEditCtrl::OnChar( WPARAM wParam, LPARAM lParam )
{
	WCHAR ch = (WCHAR) wParam;
	if(ch > 13 && ch != 27 && !(GetKeyState(VK_CONTROL) & 0x8000))
	{
		delSelection();
		m_text.insert(m_text.begin() + m_caretPos, ch);
		m_caretPos++;
		UpdateCarret();
	}
	return TRUE;
}

void CSingleLineEditCtrl::setRect(LPRECT rcText)
{
	m_width		= rcText->right - rcText->left;
	m_height	= rcText->bottom - rcText->top;
	m_rcText	= *rcText;
	m_rcText.top = rcText->top + m_height / 2 - m_lineHeight / 2;
	m_rcText.bottom = m_rcText.top + m_lineHeight;
	//createCaret();
}

void CSingleLineEditCtrl::draw(cairo_t* cr)
{
	int selStart	= std::min(m_selStart, m_selEnd);
	int selEnd		= std::max(m_selStart, m_selEnd);

	RECT rcText = m_rcText;

	if(m_selStart >= 0)
	{
		if(selStart < m_leftPos)
		{
			selStart = m_leftPos;
		}

		int left = 0;
		// draw left side of the text
		if(selStart > 0)
		{
			if(selStart > m_leftPos)
			{
				rcText.left = m_rcText.left + left;
				SIZE sz = {0, 0};
				getTextExtentPoint(m_text.c_str() + m_leftPos, selStart - m_leftPos, &sz);
				drawText(cr, m_text.c_str() + m_leftPos, selStart - m_leftPos, &rcText, m_textColor);
				left += sz.cx;
			}
		}
		// draw the selection
		if(selStart < selEnd)
		{
			SIZE sz = {0, 0};
			getTextExtentPoint(m_text.c_str() + selStart, selEnd - selStart, &sz);
			rcText = m_rcText;
			RECT rcFill;
			rcFill.left		= m_rcText.left + left;
			rcFill.right	= rcFill.left + sz.cx;
			rcFill.top		= m_rcText.top + (m_rcText.bottom - m_rcText.top) / 2 - m_lineHeight / 2;
			rcFill.bottom	= rcFill.top + m_lineHeight;
			if(rcFill.right > m_rcText.right)
			{
				rcFill.right = m_rcText.right;
			}
			fillSelRect(cr, &rcFill);

			rcText.left		= m_rcText.left + left;
			rcText.right	= rcText.left + sz.cx;
			if(rcText.right > m_rcText.right)
			{
				rcText.right = m_rcText.right;
			}
			COLORREF clr = GetSysColor(COLOR_HIGHLIGHTTEXT);
			drawText(cr, m_text.c_str() + selStart, selEnd - selStart, &rcText, litehtml::web_color(GetRValue(clr), GetGValue(clr), GetBValue(clr)));

			left += sz.cx;
		}
		// draw the right side of the text
		if(selEnd <= m_text.length())
		{
			rcText.left		= m_rcText.left + left;
			rcText.right	= m_rcText.right;
			if(rcText.left < rcText.right)
			{
				drawText(cr, m_text.c_str() + selEnd, -1, &rcText, m_textColor);
			}
		}
	} else
	{
		drawText(cr, m_text.c_str() + m_leftPos, -1, &rcText, m_textColor);
	}

	if(m_showCaret && m_caretIsCreated)
	{
		cairo_save(cr);

		int caretWidth = GetSystemMetrics(SM_CXBORDER);
		int caretHeight = m_lineHeight;
		int top = m_rcText.top + (m_rcText.bottom - rcText.top) / 2 - caretHeight / 2;

		cairo_set_source_rgba(cr, m_textColor.red / 255.0, m_textColor.green / 255.0, m_textColor.blue / 255.0, m_textColor.alpha / 255.0);
		cairo_rectangle(cr, m_rcText.left + m_caretX, top, caretWidth, caretHeight);
		cairo_fill(cr);

		cairo_restore(cr);
	}
}

void CSingleLineEditCtrl::setFont(cairo_font* font, litehtml::web_color& color)
{
	m_hFont = font;
	m_textColor = color;
	m_lineHeight = font->metrics().height;
}

void CSingleLineEditCtrl::UpdateCarret()
{
	if(m_caretPos < m_leftPos)	m_leftPos = m_caretPos;

	SIZE sz = {0, 0};
	getTextExtentPoint(m_text.c_str() + m_leftPos, m_caretPos - m_leftPos, &sz);

	m_caretX = sz.cx;

	while(m_caretX > m_width - 2 && m_leftPos < m_text.length())
	{
		m_leftPos++;
		getTextExtentPoint(m_text.c_str() + m_leftPos, m_caretPos - m_leftPos, &sz);
		m_caretX = sz.cx;
	}

	if(m_caretX < 0) m_caretX = 0;

	m_showCaret = TRUE;
	UpdateControl();
}

void CSingleLineEditCtrl::UpdateControl()
{
	if (m_parent)
	{
		SendMessage(m_parent, WM_UPDATE_CONTROL, 0, 0);
	}
}

void CSingleLineEditCtrl::delSelection()
{
	if(m_selStart < 0) return;
	int start	= std::min(m_selStart, m_selEnd);
	int end		= std::max(m_selStart, m_selEnd);

	m_text.erase(start, end - start);

	m_caretPos = start;
	m_selStart = -1;
	UpdateCarret();
	UpdateControl();
}

void CSingleLineEditCtrl::setSelection( int start, int end )
{
	m_selStart	= start;
	m_selEnd	= end;
	if(m_selEnd < 0) m_selEnd = (int) m_text.length();
	if(m_selStart >= 0)
	{
		m_caretPos = m_selEnd;
		if(m_caretPos < 0)
		{
			m_caretPos = (int) m_text.length();
		} else
		{
			if(m_caretPos > (int) m_text.length())
			{
				m_caretPos = (int) m_text.length();
			}
		}
		if(m_selStart > (int) m_text.length())
		{
			m_selStart = (int) m_text.length();
		}
		if(m_selEnd > (int) m_text.length())
		{
			m_selEnd = (int) m_text.length();
		}
		if(m_selEnd == m_selStart)
		{
			m_selStart = -1;
		}
	}
	UpdateCarret();
}

void CSingleLineEditCtrl::replaceSel( LPCWSTR text )
{
	delSelection();
	m_text.insert(m_caretPos, text);
}

void CSingleLineEditCtrl::createCaret()
{
	m_caretIsCreated = TRUE;
	Run();
}

void CSingleLineEditCtrl::destroyCaret()
{
	m_caretIsCreated = FALSE;
}

void CSingleLineEditCtrl::setCaretPos( int pos )
{
	m_caretPos = pos;
	UpdateCarret();
}

void CSingleLineEditCtrl::fillSelRect(cairo_t* cr, LPRECT rcFill)
{
	cairo_save(cr);

	COLORREF clr = GetSysColor(COLOR_HIGHLIGHT);
	litehtml::web_color color(GetRValue(clr), GetGValue(clr), GetBValue(clr));

	cairo_set_source_rgba(cr, color.red / 255.0, color.green / 255.0, color.blue / 255.0, color.alpha / 255.0);
	cairo_rectangle(cr, rcFill->left, rcFill->top, rcFill->right - rcFill->left, rcFill->bottom - rcFill->top);
	cairo_fill(cr);

	cairo_restore(cr);
}

int CSingleLineEditCtrl::getCaretPosXY( int x, int y )
{
	int pos	= -1;
	int w = 0;

	for(int i=1; i < (int) m_text.length(); i++)
	{
		SIZE sz;
		getTextExtentPoint(m_text.c_str(), i, &sz);
		if (x > w && x < w + (sz.cx - w) / 2)
		{
			pos = i - 1;
			break;
		}
		else if (x >= w + (sz.cx - w) / 2 && x <= sz.cx)
		{
			pos = i;
			break;
		}
		w = sz.cx;
	}
	if(pos < 0)
	{
		if (x > 0)
		{
			pos = (int)m_text.length();
		}
		else
		{
			pos = 0;
		}
	}

	return pos;
}

void CSingleLineEditCtrl::setText( LPCWSTR text )
{
	m_caretPos	= 0;
	m_text		= text;
	m_selStart	= -1;
	m_selEnd	= -1;
	m_leftPos	= 0;

	SIZE sz;
	getTextExtentPoint(m_text.c_str(), -1, &sz);
	m_lineHeight = sz.cy;

	UpdateControl();
}

void CSingleLineEditCtrl::drawText(cairo_t* cr, LPCWSTR text, int cbText, LPRECT rcText, litehtml::web_color textColor)
{
	std::wstring str;
	if (cbText < 0)
	{
		str = text;
	}
	else
	{
		str.append(text, cbText);
	}

	litehtml::position pos;
	pos.x = rcText->left;
	pos.y = rcText->top;
	pos.width = rcText->right - rcText->left;
	pos.height = rcText->bottom - rcText->top;

	auto str_utf8 = cairo_font::wchar_to_utf8(str.c_str());
	m_container->draw_text((litehtml::uint_ptr) cr, str_utf8.c_str(), (litehtml::uint_ptr)m_hFont, textColor, pos);
}

void CSingleLineEditCtrl::getTextExtentPoint( LPCWSTR text, int cbText, LPSIZE sz )
{
	std::wstring str;
	if (cbText < 0)
	{
		str = text;
	}
	else
	{
		str.append(text, cbText);
	}
	auto str_utf8 = cairo_font::wchar_to_utf8(str);
	sz->cx = m_container->text_width(str_utf8.c_str(), (litehtml::uint_ptr)m_hFont);
	sz->cy = m_hFont->metrics().height;
}

DWORD CSingleLineEditCtrl::ThreadProc()
{
	m_showCaret = TRUE;
	UINT blinkTime = GetCaretBlinkTime();
	if(!blinkTime)	blinkTime = 500;
	while(!WaitForStop(blinkTime))
	{
		m_showCaret = m_showCaret ? FALSE : TRUE;
		UpdateControl();
	}
	m_showCaret = TRUE;
	return 0;
}

BOOL CSingleLineEditCtrl::OnKeyUp( WPARAM wParam, LPARAM lParam )
{
	UINT key = (UINT) wParam;
	switch(key)
	{
	case VK_BACK:
		Run();
		return 0;
	case VK_DELETE:
		Run();
		return 0;
	case VK_DOWN:
		Run();
		return 0;
	case VK_UP:
		Run();
		return 0;
	case VK_LEFT:
		Run();
		return 0;
	case VK_RIGHT:
		Run();
		return 0;
	}
	return TRUE;
}

void CSingleLineEditCtrl::OnLButtonDown( int x, int y )
{
	m_caretPos = getCaretPosXY(x - m_rcText.left, y - m_rcText.top);
	setSelection(-1, m_caretPos);
	SendMessage(m_parent, WM_EDIT_CAPTURE, TRUE, 0);
	m_inCapture	= TRUE;
	m_startCapture = m_caretPos;
}

void CSingleLineEditCtrl::OnLButtonUp( int x, int y )
{
	if(m_inCapture)
	{
		SendMessage(m_parent, WM_EDIT_CAPTURE, FALSE, 0);
		m_inCapture = FALSE;
	}
}

void CSingleLineEditCtrl::OnLButtonDblClick( int x, int y )
{
	int pos = getCaretPosXY(x - m_rcText.left, y - m_rcText.top);
	int start = pos;
	int end = pos;
	for(;start > 0; start--)
	{
		if(!_istalnum(m_text[start]))
		{
			start++;
			break;
		}
	}
	for(;end < (int) m_text.length(); end++)
	{
		if(!_istalnum(m_text[end]))
		{
			break;
		}
	}
	if(start < end)
	{
		setSelection(start, end);
	}
}

void CSingleLineEditCtrl::OnMouseMove( int x, int y )
{
	if(m_inCapture)
	{
		m_caretPos = getCaretPosXY(x - m_rcText.left, y - m_rcText.top);
		setSelection(m_startCapture, m_caretPos);
	}
}

void CSingleLineEditCtrl::hideCaret()
{
	Stop();
	m_showCaret = FALSE;
	UpdateControl();
}

void CSingleLineEditCtrl::showCaret()
{
	m_showCaret = TRUE;
	createCaret();
	UpdateControl();
}

void CSingleLineEditCtrl::set_parent(HWND parent)
{
	m_parent = parent;
}
