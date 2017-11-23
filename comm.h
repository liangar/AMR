#ifndef COMM_H
#define COMM_H

#define COMM_BUF_LEN 2*1024

class CComm;
typedef void (*CommCallbackFunc)(CComm *comm,  void *param);

class CComm
{
public:
	CComm(CommCallbackFunc fOnReceive, void *parm = 0);
	virtual ~CComm();

	BOOL IsOpen() { return m_hPort != INVALID_HANDLE_VALUE; }
	void SetBaud( int baud );
	void SetPort( int n );
	BOOL Open();
    BOOL Open(const char * p);
	void Close();

	BOOL Send(const void *buf, int len) { return WriteComm(buf, len); }
	int  Recv(void *buf, int len, int timeout = 0);
    void SetNotify(HANDLE hHasData) {  m_hHasData = hHasData;  }

	int  RecvByEnd(char * buf, int maxlen, char * endstr, int timeout);
	int  RecvByEnd(char * buf, int maxlen, char endch, int timeout);
	int  RecvByTimeout(char * buf, int maxlen, int timeout);

	int BufLen() { return m_nBufLen; }
	int BytesUnread() { return m_nBytesUnread; }

	int Skip(int nBytes);
	int Peek(void *buf, int len);
	const char *InBuf() { return m_pRecvBuf; }

public:
    HANDLE      m_hHasData;

protected:
	int  GetInCount();
	int  ReadComm(void *lpszBlock, int nMaxLength);
	BOOL WriteComm(const void *lpByte , int dwBytesToWrite);

protected:
	void OnReceive();
	static void WINAPI THWorker(CComm *comm);
	void Worker();

	CRITICAL_SECTION m_BufSection;
	void lock(void)  { EnterCriticalSection(&m_BufSection); }
	void unlock(void){ LeaveCriticalSection(&m_BufSection); }

protected:
	CommCallbackFunc m_fOnReceive;

	void        *m_parm;


    HANDLE      m_hPort;
	int         m_baudRate;
	BYTE		m_Parity;
	int         m_nPort;
	BYTE		m_StopBits;
	BOOL		m_XonXoffControl;

	OVERLAPPED  m_ovRead;
	OVERLAPPED  m_ovWrite;
	OVERLAPPED  m_ovReadThread;

	HANDLE      m_hThread;
	DWORD       m_dwThreadId;
	BOOL        m_bWantExitThread;
	BOOL        m_bThreadExited;

	int         m_nBytesUnread;
	int         m_nBufLen;
	// the receive buf is dynamically allocated and, if the buf is full,
	// the buf will automatically increase.
	char        *m_pRecvBuf;
};

#endif
