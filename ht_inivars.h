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
extern char		gini_padpath[MAX_PATH];	/// 手抄器的工作目录
extern int      gini_padport;	/// 手抄器通讯端口

extern int		gini_startwait;
extern int		gini_idle;

extern int		gini_tryTimes;	/// 执行命令重试次数
extern int		gini_timeLimit;	/// 重试间隔时间
extern int		gini_dialwait;	/// 拨号等待时间

extern int		gini_cmdLogDays;	/// 命令记录保持天数
extern int		gini_recLogDays;	/// 抄表记录保持天数

extern int		gini_listen_port;

extern char		gini_dbver[16];	/// 数据库版本，缺省为1.0

extern xini		g_ini;

void gini_getvars(const char * inifilename);

#endif // _VQ_INIVARS_H_