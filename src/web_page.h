#pragma once

#include "../containers/windows/cairo/windows_container.h"
#include "../containers/windows/cairo/cairo_font.h"
#include "../containers/cairo/cairo_images_cache.h"
#include "tordexhttp.h"

class CHTMLViewWnd;

class web_page : public windows_container
{
	CHTMLViewWnd*				m_parent;
	LONG						m_refCount;
public:
	tordex::http				m_http;
	std::string					m_url;
	litehtml::document::ptr		m_doc;
	std::wstring				m_caption;
	std::wstring				m_cursor;
	std::string					m_base_path;
	HANDLE						m_hWaitDownload;
	std::wstring				m_waited_file;
	std::wstring				m_hash;
	cairo_images_cache			m_images;
public:
	web_page(CHTMLViewWnd* parent);
	virtual ~web_page();

	void load(LPCWSTR url);
	// encoding: as specified in Content-Type HTTP header
	//   it is NULL for local files or if Content-Type header is not present or Content-Type header doesn't contain "charset="
	void on_document_loaded(LPCWSTR file, LPCWSTR encoding, LPCWSTR realUrl);
	void on_image_loaded(LPCWSTR file, LPCWSTR url, bool redraw_only);
	void on_document_error(DWORD dwError, LPCWSTR errMsg);
	void on_waited_finished(DWORD dwError, LPCWSTR file);
	void add_ref();
	void release();
	void get_url(std::wstring& url);

	// litehtml::document_container members
	void set_caption(const char* caption) override;
	void set_base_url(const char* base_url) override;
	void import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl) override;
	void on_anchor_click(const char* url, const litehtml::element::ptr& el) override;
	void set_cursor(const char* cursor) override;
	void load_image(const char* src, const char* baseurl, bool redraw_on_ready) override;
	void make_url(const char* url, const char* basepath, litehtml::string& out) override;

	cairo_surface_t* get_image(const std::string& url) override;
	void get_client_rect(litehtml::position& client) const  override;
private:
	char*	load_text_file(LPCWSTR path, bool is_html, LPCWSTR defEncoding = L"UTF-8", LPCWSTR forceEncoding = NULL);
	BOOL	download_and_wait(LPCWSTR url);
};

enum web_file_type
{
	web_file_document,
	web_file_image_redraw,
	web_file_image_rerender,
	web_file_waited,
};

class web_file : public tordex::http_request
{
	WCHAR			m_file[MAX_PATH];
	web_page*		m_page;
	web_file_type	m_type;
	HANDLE			m_hFile;
	LPVOID			m_data;
	std::wstring	m_realUrl;
	std::wstring	m_encoding;
public:
	web_file(web_page* page, web_file_type type, LPVOID data = NULL);
	virtual ~web_file();

	virtual void OnFinish(DWORD dwError, LPCWSTR errMsg);
	virtual void OnData(LPCBYTE data, DWORD len, ULONG64 downloaded, ULONG64 total);
	virtual void OnHeadersReady(HINTERNET hRequest);

};
