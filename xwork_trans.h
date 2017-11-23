#ifndef _xwork_trans_H_
#define _xwork_trans_H_

#include <xwork_server.h>
#include <ss_service.h>

#include "ht_trans.h"

typedef enum TRANS_STATE{
	trans_null, trans_start, trans_doing, trans_end, trans_ok, trans_error
};

/// \class xwork_trans
/// ����ִ����
class xwork_trans : public xwork_server
{
public:
	/// ����
	xwork_trans();

	bool close(int secs = 5);	/// �ر��ͷ�

	/// ��������
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
		case trans_null :  return "����";
		case trans_start:  return "��ʼִ��";
		case trans_doing:  return "ִ����...";
		case trans_ok   :  return "�ɹ�";
		case trans_error:  return "ͨѶʧ��";
		}
		return "����";
	}
protected:
	int trans(const char * parms, int steps);	/// ִ������
	int trans_cmd(void);	/// ִ������

	int find_cmd(const char * cmd);	/// �������Ʋ�������

	int do_list(const char * fileName);

protected:
	kh_trans	m_trans;
	int			m_icmd;	/// ��ǰ����

	HWND		m_hwnd;	/// ��Ϣ֪ͨ����
	unsigned int m_wnd_message_id;	/// ��Ϣ�¼�ID

	char	m_port[64];			/// ͨѶ�˿�
	char	m_telnumber[32];	/// �绰����
	char	m_params[64];		/// ִ�в���<x,y,z,...>
	bool	m_bneedBackupData;	/// ��Ҫ�����ϴγ�������

	TRANS_STATE	m_state;		/// ״̬
	TRANS_STATE m_lastCmdState;	/// ���ִ��״̬
};

extern xwork_trans g_worker;

void AddMessageText(const char * msg);

#endif // _xwork_trans_H_
