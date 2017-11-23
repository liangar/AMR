#pragma once

#include <windows.h>
#include <comm.h>

#include "ht_cmn.h"

class kh_trans : public CComm
{
public:
	kh_trans();

	/// 初始化
	int  init(const char * serialParam, const char * telnumber);
	int  init(int id);	/// 打开指定id号的设备
	void down(void);	/// 关闭

	/// 通讯
	/// \param recvbuf	接收缓冲
	/// \param sendbuf	发送缓冲
	/// \param sendlen	发送长度(由命令决定)
	/// \param cmdtype	命令类型:0|1|n = ACK|1|n
	/// \return >=0/errcode 返回的字节数/出错代码
	int trans(void);

	void show_recv_buf(void);
	void set_cmdType(ha_cmdType * pcmd)  {  m_pcmd = pcmd;  }

	const char * get_lastmsg(void)  {  return m_msg;  }

	void set_hook_wait(int ms)  {  if (ms < 500)  ms = 500;  if (ms > 10000)  ms = 10000;  m_hook_wait = ms;  }
	void set_dail_wait(int s)  {  if (s < 5)  s = 5;  if (s > 90)  s = 90;  m_dail_wait = s;   }
	void set_hangup_wait(int ms)  {  if (ms < 500)  ms = 500;  if (ms > 10000)  ms = 10000;  m_hangup_wait = ms;  }
	void set_no_meter_state(bool s)  {  m_no_state = s;  }

protected:
	int  send_record(void);	/// 发送命令
	int  recv_recs(void);	/// 接收多条记录返回
	int  recv_rec(void);	/// 接收单条记录返回

protected:
	int  clear_recv_buf(void);
	int  clear_all_recv(void);

	int  peek_byte(char * c);
	int  recv_byte(char * c, int timeout);
	int  recv_bytes(int n);
	int  send_byte(char c);
	int  send_bytes(const char * p, int l);
	int  send_bytes(const char * p);
	int  send_bytes(int l);

	/// AT命令
	/// return <0/else = error/ok
	int  send_AT_cmd(const char * p, const char * okstring, int timeout = 1500);
	void re_send(char recs);

	int  sendack(char isok);

	void showProcessing(char i, char n);

public:
	char m_sendbuf[1024];
	char * m_recvbuf;
	long m_recs;
	char m_steps;
	ha_cmdType * m_pcmd;
	char m_telnumber[32];
	char m_serialport_string[128];
	bool m_bdialed;
	bool m_no_state;	/// 无表状态

	bool m_isSingle;	/// 是否是单表操作
	char m_singleState;	/// 单表状态

	char m_msg[1024];

	int  m_hook_wait;	/// 摘机等待(ms)
	int  m_dail_wait;	/// 拨号握手等待(s)
	int  m_hangup_wait;	/// 挂断等待(ms)

	ha_cmd_result m_result;
};

extern kh_trans	g_trans;
