#ifndef _HT_INIVARS_H_
#define _HT_INIVARS_H_

#include <xlist.h>
#include <xini.h>

#include "ht_cmn.h"

extern char		gini_DependOnService[256];
extern char		gini_dsnstring[128];
extern char		gini_record_sql[512];
extern char		gini_record_parms[64];

extern char		gini_datapath[MAX_PATH];
extern char		gini_padpath[MAX_PATH];	/// �ֳ����Ĺ���Ŀ¼
extern int      gini_padport;	/// �ֳ���ͨѶ�˿�

extern int		gini_startwait;
extern int		gini_idle;

extern int		gini_tryTimes;	/// ִ���������Դ���
extern int		gini_timeLimit;	/// ���Լ��ʱ��
extern int		gini_dialwait;	/// ���ŵȴ�ʱ��

extern int		gini_cmdLogDays;	/// �����¼��������
extern int		gini_recLogDays;	/// �����¼��������

extern int		gini_listen_port;

extern char		gini_dbver[16];	/// ���ݿ�汾��ȱʡΪ1.0

extern xini		g_ini;

void gini_getvars(const char * inifilename);

#endif // _VQ_INIVARS_H_