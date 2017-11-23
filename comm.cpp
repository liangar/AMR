#include <windows.h>
#include <stdio.h>

#include <l_str.h>

#include "comm.h"

static LPSTR  defaultSetting = "COM1\x0 baud=9600 parity=N data=8 stop=1";

#define INCREASE_BUF_LEN 1024

CComm::CComm(CommCallbackFunc fOnReceive, void *parm)
    : m_hHasData(NULL)
	, m_hPort(INVALID_HANDLE_VALUE) // invalidate to start
	, m_baudRate(9600)
	, m_Parity(NOPARITY)
	, m_StopBits(ONESTOPBIT)
	, m_nPort(1)
    , m_hThread(NULL)
	, m_bWantExitThread(FALSE)
	, m_bThreadExited(TRUE)
	, m_fOnReceive(fOnReceive)
	, m_parm(parm)
	, m_nBytesUnread(0)
	, m_nBufLen(INCREASE_BUF_LEN)
{
    m_ovRead.hEvent = m_ovWrite.hEvent = NULL;

    memset(&m_ovRead, 0, sizeof(m_ovRead));
    memset(&m_ovWrite, 0, sizeof(m_ovWrite));
    if ((m_ovRead.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL )) == NULL)
		throw "can't create event1 in CComm::CComm";

    if ((m_ovWrite.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL )) == NULL)
		throw "can't create event2 in CComm::CComm";

	m_hHasData = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_pRecvBuf = 0;
}


CComm::~CComm()
{
	Close();
    if (m_ovRead.hEvent != NULL)
    	CloseHandle(m_ovRead.hEvent);
    if (m_ovWrite.hEvent != NULL)
    	CloseHandle(m_ovWrite.hEvent);
	if (m_pRecvBuf)
		free(m_pRecvBuf);
}



void WINAPI CComm::THWorker(CComm *comm)
{
	try {
		comm->Worker();
	}catch(...){  // const char *s) {
		// TRACE("Exception in CComm::THworker %s", s);
		comm->m_bThreadExited = TRUE;
	}
}

void CComm::Worker()
{
	m_bThreadExited = TRUE;

	if (m_hPort == INVALID_HANDLE_VALUE ||
        !SetCommMask(m_hPort, EV_RXCHAR ) ) {
		return;
	}

    m_bThreadExited = FALSE;
	m_bWantExitThread = FALSE;
	while ( !m_bWantExitThread ) {
		DWORD dwEvtMask = 0;

		WaitCommEvent( m_hPort, &dwEvtMask, NULL );

		if(m_bWantExitThread)
			break;

		if ((dwEvtMask & EV_RXCHAR) == EV_RXCHAR) {
			Sleep(10);
			OnReceive();
			if(m_fOnReceive != NULL)
				m_fOnReceive(this, m_parm);
		}
	}

	m_bThreadExited = TRUE;
}


void CComm::SetBaud( int baud )
{
	if(m_baudRate != baud) {
		m_baudRate = baud;
		if(IsOpen()) {
			Close();
			Open();
		}
	}
}


void CComm::SetPort( int n )
{
	if(m_nPort != n) {
		m_nPort = n;
		if(IsOpen()) {
			Close();
			Open();
		}
	}
}


int CComm::GetInCount()
{
	COMSTAT statPort;
	DWORD dwErrorCode;

	if(IsOpen()) {
		ClearCommError(m_hPort, &dwErrorCode, &statPort);
		return statPort.cbInQue;
	}
	return 0;
}


BOOL CComm::Open()
{
	char sCom[20];
    COMMTIMEOUTS  CommTimeOuts ;

	if(IsOpen())
		Close();

	InitializeCriticalSection(&m_BufSection);

	m_pRecvBuf = (char *)calloc(m_nBufLen, 1);
	if (m_pRecvBuf == 0){
		return FALSE;
	}

    sprintf(sCom, "COM%d", m_nPort);
	m_hPort = CreateFile(sCom, GENERIC_READ | GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);
	if (m_hPort == INVALID_HANDLE_VALUE) {
		// TRACE("Error Open %s", sCom);
		return FALSE;
    }

	// get any early notifications
	SetCommMask( m_hPort, EV_RXCHAR ) ;

	// setup device buffers
	SetupComm(m_hPort, COMM_BUF_LEN, COMM_BUF_LEN);

	// purge any information in the buffer

	PurgeComm(m_hPort, PURGE_TXABORT | PURGE_RXABORT |
		PURGE_TXCLEAR | PURGE_RXCLEAR);

	// set up for overlapped I/O

	CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 1500;
	CommTimeOuts.ReadTotalTimeoutConstant = 15000;

	// write timeout = WriteTotalTimeoutMultiplier * bytes_to_send + WriteTotalTimeoutConstant
	// CBR_9600 is approximately 1byte/ms. so if baud rate < 9600
	// we can use CBR_9600/rate to estimate the timeout.
	// if > 9600, use 1. here, we use 10 to all.
	CommTimeOuts.WriteTotalTimeoutMultiplier = 100;
	CommTimeOuts.WriteTotalTimeoutConstant = 2000;	//-1; //!! orginal is 0 ;

	SetCommTimeouts( m_hPort, &CommTimeOuts ) ;	

	DCB dcbPort;
	memset(&dcbPort, 0, sizeof(DCB));
	dcbPort.DCBlength = sizeof( DCB ) ;
	if (GetCommState(m_hPort, &dcbPort)) {
		// fill in the fields of the structure
		dcbPort.BaudRate = m_baudRate;
		dcbPort.ByteSize = 8;
		dcbPort.Parity = m_Parity;
		dcbPort.StopBits = m_StopBits;

		/*
		// no flow control
		dcbPort.fOutxCtsFlow = FALSE;
		dcbPort.fOutxDsrFlow = FALSE;
		dcbPort.fOutX = FALSE;
		dcbPort.fInX = FALSE;
		*/

		/*
		// DsrDtrFlowControl
		dcbPort.fOutxCtsFlow = FALSE;
		dcbPort.fOutxDsrFlow = TRUE;
		dcbPort.fDtrControl = DTR_CONTROL_HANDSHAKE;
		dcbPort.fOutX = FALSE;
		dcbPort.fInX = FALSE;
		*/

		if (m_XonXoffControl){
			// xon/xoff control
			dcbPort.fOutxCtsFlow = FALSE;
			dcbPort.fOutxDsrFlow = FALSE;
			dcbPort.fOutX = TRUE;
			dcbPort.fInX = TRUE;
			dcbPort.XonChar = 0x11;
			dcbPort.XoffChar = 0x13;
			dcbPort.XoffLim = 32;
			dcbPort.XonLim = 32;
		}

		dcbPort.fBinary = TRUE ;
		// dcbPort.fParity = TRUE ;

		SetCommState(m_hPort, &dcbPort);
        EscapeCommFunction( m_hPort, SETDTR ) ;
    }

	DWORD dwThreadId;
	m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)THWorker,
		this, CREATE_SUSPENDED, &dwThreadId);

	if(m_hThread==NULL)
		throw "CComm::Open can't create worker thread";

	m_nBytesUnread = 0;

	//	SetThreadPriority(m_hThread, THREAD_PRIORITY_ABOVE_NORMAL);
	ResumeThread(m_hThread);

    return TRUE;
}

BOOL CComm::Open(const char * lpSetting)
{
    DCB   d;	char sCom[32];
    LPSTR lps = NULL;
	const char * lpCommName;

	memset(&d, 0, sizeof(d));

    lpCommName = lpSetting;
    if (lpSetting != NULL && (lps = strchr(lpSetting, ':')) != NULL)
        lps += 1;

    if (lpCommName == NULL)
		lpCommName = defaultSetting;
	if (lps == NULL){
        lps = defaultSetting + 6;
    }
    if (strnicmp(lpCommName, "COM", 3) != 0)
        return FALSE;

	COMMCONFIG cc;  DWORD dwSize = sizeof(cc);

	getaword(sCom, lpCommName, ':');

	memset(&cc, 0, sizeof(cc));
	cc.dcb.DCBlength = sizeof(DCB);
	BOOL bisok = GetDefaultCommConfig(sCom, &cc, &dwSize);
	memcpy(&d, &cc.dcb, sizeof(d));
    if (BuildCommDCB(lps, &d) == 0)     // ep. "baud=1200 parity=N data=8 stop=1 "
        return FALSE;

	m_Parity = d.Parity;
	m_StopBits = d.StopBits;
	m_nPort = int(lpCommName[3] - '0');
	m_baudRate = d.BaudRate;
	m_XonXoffControl = d.fOutX;
    
    return Open();
}

void  CComm::Close()
{
	if(!m_bThreadExited) {
		m_bWantExitThread = TRUE;
		SetCommMask( m_hPort,  0 ) ;
		if (WaitForSingleObject(m_hThread, 2000) != WAIT_OBJECT_0)
            TerminateThread(m_hThread, 0);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	if (m_hPort != INVALID_HANDLE_VALUE) {
		EscapeCommFunction( m_hPort, CLRDTR ) ;
		// purge any outstanding reads/writes and close device handle
		PurgeComm( m_hPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR ) ;
		CloseHandle(m_hPort);
		m_hPort = INVALID_HANDLE_VALUE;
    }
	m_nBytesUnread = 0;
	DeleteCriticalSection(&m_BufSection);

	if (m_pRecvBuf != 0){
		free(m_pRecvBuf);  m_pRecvBuf = 0;
	}
	m_nBufLen = INCREASE_BUF_LEN;
}


int CComm::ReadComm(void *lpszBlock, int nMaxLength)  // return bytes write
{
	BOOL       fReadStat ;
	COMSTAT    ComStat ;
	DWORD      dwErrorFlags;
	DWORD      dwLength;
	DWORD      dwError;

	// only try to read number of bytes in queue
	ClearCommError( m_hPort, &dwErrorFlags, &ComStat ) ;
	dwLength = min( (DWORD) nMaxLength, ComStat.cbInQue ) ;

	if(dwLength==0)
		return 0;

	if ((fReadStat = ReadFile(m_hPort, lpszBlock, dwLength, &dwLength, &m_ovRead)) == 0){
		if (GetLastError() == ERROR_IO_PENDING){
			//            OutputDebugString("\n\rIO Pending");
			// We have to wait for read to complete.
			// This function will timeout according to the
			// CommTimeOuts.ReadTotalTimeoutConstant variable
			// Every time it times out, check for port errors
			while(!GetOverlappedResult( m_hPort, &m_ovRead, &dwLength, TRUE )){
				dwError = GetLastError();
				if(dwError == ERROR_IO_INCOMPLETE)
					// normal result if not finished
					continue;
				else{
					ClearCommError( m_hPort, &dwErrorFlags, &ComStat ) ;
					// TRACE("Comm Err: %u",  dwErrorFlags);
					break;
				}
			}
		}else{
			// some other error occurred
			dwLength = 0 ;
			ClearCommError( m_hPort, &dwErrorFlags, &ComStat ) ;
			// TRACE("Comm Err: %u",  dwErrorFlags);
		}
	}
	return ( dwLength ) ;
}


BOOL CComm::WriteComm(const void *lpByte , int dwBytesToWrite)
{
	BOOL        fWriteStat ;
	DWORD       dwBytesWritten ;
	DWORD       dwErrorFlags;
	DWORD       dwBytesSent = 0;
	COMSTAT     ComStat;

	fWriteStat = WriteFile( m_hPort, lpByte, dwBytesToWrite, &dwBytesWritten, &m_ovWrite ) ;
	// return TRUE;

	FlushFileBuffers(m_hPort);
	// Note that normally the code will not execute the following
	// because the driver caches write operations. Small I/O requests
	// (up to several thousand bytes) will normally be accepted
	// immediately and WriteFile will return true even though an
	// overlapped operation was specified

	if (!fWriteStat){
		if(GetLastError() == ERROR_IO_PENDING){
			// We should wait for the completion of the write operation
			// so we know if it worked or not

			// This is only one way to do this. It might be beneficial to
			// place the write operation in a separate thread
			// so that blocking on completion will not negatively
			// affect the responsiveness of the UI

			// If the write takes too long to complete, this
			// function will timeout according to the
			// CommTimeOuts.WriteTotalTimeoutMultiplier variable.
			// This code logs the timeout but does not retry
			// the write.

			if (!GetOverlappedResult( m_hPort, &m_ovWrite, &dwBytesWritten, TRUE )){
				ClearCommError( m_hPort, &dwErrorFlags, &ComStat ) ;
				return FALSE;
			}

			dwBytesSent += dwBytesWritten;

			if( (int)dwBytesSent != dwBytesToWrite ) {
				// TRACE("CComm::Write: Probable Write Timeout: Total of %ld bytes sent", dwBytesSent);
				return FALSE;
			}
		}else{
			// some other error occurred
			ClearCommError( m_hPort, &dwErrorFlags, &ComStat ) ;
			// TRACE("Comm Err: %u",  dwErrorFlags);
			return FALSE;
		}
	}
	return TRUE;
}

int CComm::Recv(void *buf, int len, int timeout)
{
    if (!IsOpen())  return -1;
    if (len == 0)  return 0;

    // copy n bytes in buf
    DWORD dw = (timeout >= 0)?((DWORD)timeout):INFINITE;
    if (m_nBytesUnread == 0 &&
        WaitForSingleObject(m_hHasData, dw) != WAIT_OBJECT_0){ // wait
        return 0;
    }

	lock();

    if (len > m_nBytesUnread)
	 	len = m_nBytesUnread;

	CopyMemory(buf, m_pRecvBuf, len);

	// move buf
	m_nBytesUnread -= len;
	if (m_nBytesUnread)
		MoveMemory(m_pRecvBuf, m_pRecvBuf + len, m_nBytesUnread);

    if (m_nBytesUnread <= 0)
        ResetEvent(m_hHasData);

	unlock();

	return len;
}

int CComm::RecvByEnd(char * buf, int maxlen, char * endstr, int timeout)
{
	memset(buf, 0, maxlen);

	int i , r;
	for (i = r = 0; i < maxlen; ){
		if ((r = Recv(buf + i, maxlen-i, timeout)) <= 0){
			return r;
		}
		i += r;
		if (i < maxlen)
			buf[i] = 0;

		if (endcmp(buf, endstr) == 0){
			i -= (strlen(endstr) - 1);
			buf[i] = 0;
			return i;
		}
	}
	return i;
}

int CComm::RecvByEnd(char * buf, int maxlen, char endch, int timeout)
{
	memset(buf, 0, maxlen);
	
	int i , r;
	for (i = r = 0; i < maxlen; ){
		if ((r = Recv(buf + i, maxlen-i, timeout)) <= 0){
			return r;
		}
		i += r;
		if (i < maxlen)
			buf[i] = 0;
		
		if (buf[i-1] == endch){
			break;
		}
	}
	return i;
}

int CComm::RecvByTimeout(char * buf, int maxlen, int timeout)
{
	int i = 0, r;
	memset(buf, 0, maxlen);
	while (i < maxlen){
		if ((r = Recv(buf + i, maxlen-i, timeout)) <= 0){
			return i;
		}
		i += r;
	}
	return i;
}

int CComm::Skip(int nBytes)
{
	if (!IsOpen())  return -1;
	if (nBytes==0)  return 0;

	lock();
    if (nBytes > m_nBytesUnread)
		m_nBytesUnread = 0;
    else{
	    // move buf
	    m_nBytesUnread -= nBytes;
	    MoveMemory(m_pRecvBuf, m_pRecvBuf + nBytes, m_nBytesUnread);
    }
    if (m_nBytesUnread <= 0)
        ResetEvent(m_hHasData);
	unlock();

	return nBytes;
}

int CComm::Peek(void *buf, int len)
{
	if (!IsOpen())  return -1;
    if (len == 0 || m_nBytesUnread == 0)  return 0;

	lock();
	if(len > m_nBytesUnread)
		len = m_nBytesUnread;
	// copy n bytes in buf
	CopyMemory(buf, m_pRecvBuf, len);
	unlock();

	return len;
}


void CComm::OnReceive()
{
	if(!IsOpen())  return;

	int nBytesNew = GetInCount();

	if (nBytesNew < 1)
		return;

	lock();
	if(m_nBytesUnread + nBytesNew > m_nBufLen) {
		// if buf is full, enlarge buf by INCREASE_BUF_LEN
		m_nBufLen += INCREASE_BUF_LEN;
		char *p = (char *)realloc(m_pRecvBuf, m_nBufLen);
		if (p == 0){
			unlock();
			throw "CComm::OnReceive: Not enough memory";
		}
		m_pRecvBuf = p;
	}

	int ret = ReadComm(m_pRecvBuf + m_nBytesUnread, nBytesNew);
	m_nBytesUnread += ret;

//	if (m_nBytesUnread == ret && ret > 0)
	if (m_nBytesUnread == ret && ret > 0)
		if (m_hHasData)
			SetEvent(m_hHasData);
	unlock();
}
