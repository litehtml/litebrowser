#include "globals.h"
#include "TxThread.h"

CTxThread::CTxThread(void)
{
	m_hThread	= NULL;
	m_hStop		= NULL;
	m_trdID		= NULL;
}

CTxThread::~CTxThread(void)
{
	Stop();
	if(m_hThread) CloseHandle(m_hThread);
	if(m_hStop)   CloseHandle(m_hStop);
}

void CTxThread::Run()
{
	Stop();
	m_hStop	  = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hThread = CreateThread(NULL, 0, sThreadProc, (LPVOID) this, 0, &m_trdID);
}

void CTxThread::Stop()
{
	if(m_hThread)
	{
		if(m_hStop)
		{
			SetEvent(m_hStop);
		}
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	if(m_hStop)   CloseHandle(m_hStop);
	m_hStop = NULL;
}

DWORD WINAPI CTxThread::sThreadProc( LPVOID lpParameter )
{
	CTxThread* pThis = (CTxThread*) lpParameter;
	return pThis->ThreadProc();
}

BOOL CTxThread::WaitForStop( DWORD ms )
{
	if(WaitForSingleObject(m_hStop, ms) != WAIT_TIMEOUT)
	{
		return TRUE;
	}
	return FALSE;
}

void CTxThread::postMessage( UINT Msg, WPARAM wParam, LPARAM lParam )
{
	if(m_hThread)
	{
		PostThreadMessage(m_trdID, Msg, wParam, lParam);
	}
}