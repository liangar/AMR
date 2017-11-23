#ifndef _xwork_trans_H_
#define _xwork_trans_H_

#include <xwork_server.h>
#include <ss_service.h>

#include "ht_trans.h"

typedef enum TRANS_STATE{
	trans_null, trans_start, trans_doing, trans_end, trans_ok, trans_error
};

/// \class xwork_trans
/// 命令执行类
class xwork_trans : public xwork_server
{
public:
	/// 构造
	xwork_trans();

	bool close(int secs = 5);	/// 关闭释放

	/// 工作函数
	virtual void run(void);

	void set_notify_handle(HWND hwnd, unsigned int msgid)  {  m_hwnd = hwnd;  m_wnd_message_id = msgid;  }
	void set_config(const char * port, const char * telnumber, bool bneedBackupData = false)
	{
		strcpy(m_port, port);
		strcpy(m_telnumber, telnumber);
		m_bneedBackupData = bneedBackupData;
	}
	void set_msg(const char * msg)  {  strcpy(m_lastmsg, msg);  }

	void notify_wnd_message(void);

	const char * get_result(void)  {  return m_trans.m_result.pbuf;  }

	bool isend(void)  {  return (m_state == trans_end);  }
	bool isok(void)   {  return (m_lastCmdState == trans_ok);   }
	bool iserror(void){  return (m_lastCmdState == trans_error);  }

	bool issingle(void){ return m_trans.m_isSingle;  }
	char get_singleState(void)  {  return m_trans.m_singleState;  }
	const char * get_singleMsg(void){  return m_trans.m_msg;  }

	void set_noMeterState(bool s){  m_trans.set_no_meter_state(s);  }

	const char * get_state(void){
		switch (m_lastCmdState)
		{
		case trans_null :  return "就绪";
		case trans_start:  return "开始执行";
		case trans_doing:  return "执行中...";
		case trans_ok   :  return "成功";
		case trans_error:  return "通讯失败";
		}
		return "就绪";
	}
protected:
	int trans(const char * parms, int steps);	/// 执行命令
	int trans_cmd(void);	/// 执行命令

	int find_cmd(const char * cmd);	/// 根据名称查找命令

	int do_list(const char * fileName);

protected:
	kh_trans	m_trans;
	int			m_icmd;	/// 当前命令

	HWND		m_hwnd;	/// 消息通知窗口
	unsigned int m_wnd_message_id;	/// 消息事件ID

	char	m_port[64];			/// 通讯端口
	char	m_telnumber[32];	/// 电话号码
	char	m_params[64];		/// 执行参数<x,y,z,...>
	bool	m_bneedBackupData;	/// 需要备份上次抄收数据

	TRANS_STATE	m_state;		/// 状态
	TRANS_STATE m_lastCmdState;	/// 最近执行状态
};

extern xwork_trans g_worker;

void AddMessageText(const char * msg);

#endif // _xwork_trans_H_
