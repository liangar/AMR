#pragma once

#include <xlist.h>

typedef long           int32;	/* 32-bit signed integer */
typedef unsigned int   uint;	/* 16 or 32-bit unsigned integer */
typedef unsigned long  uint32;	/* 32-bit unsigned integer */
typedef unsigned short uint16;	/* 16-bit unsigned integer */
typedef unsigned char  byte_t;	/*  8-bit unsigned integer */
typedef unsigned char  uint8;	/* 8-bit unsigned integer */

#define KT_HEAD	"\xFE\xFE\x68"
#define KT_TAIL	'\x16'
#define KT_ACK	'\xF1'
#define KT_ERR	'\xF4'

#define KT_RECLEN	26
#define KT_RECLEN_GY	21
#define KT_REC_RECVLEN	12

/// \struct ha_cmd_param_type
struct ha_cmd_param_type{
	const char*	pname;
	char		len;
	char		type;
};

/// \struct ha_cmd_param
struct ha_cmd_param{
	ha_cmd_param_type * ptype;
	char value[16];
};

/// \struct ha_cmdType
/// 内部命令类型
///
struct ha_cmdType{
	const char *	pname;				/// 名称
	const char *	pcmd_format;		/// 格式说明
	char			nrcv_type;			/// 接收类型: 状态/单笔/多记录 = '0'/'1'/'n'
	int				nrcv_record_length;	/// 接收记录长度
	char *			pdescription;		/// 解释
};

/// \struct ha_cmd
/// 命令操作表
typedef struct tag_ha_cmd{
	long	id;				/// 编号
	char	cmd[64];		/// 命令
	char	parms[64];		/// 参数
	long	centorid;		/// 集中器ID
	char	uid[64];		/// 用户
	char	createtime[24];	/// 创建时间
	char	state[16];		/// 状态:待执行/处理中/结束
	char	substate[16];	/// 子状态:等待/执行中,结束有两种:失败/完成
	long	tryTimes;		/// 重试次数:最多3次
	char	lastEndTime[24];	/// 最近结束操作时间
	char	lastBeginTime[24];	/// 最近开始操作时间
	char	processing[256];	/// 进度描述
} ha_cmd;

struct ha_record{	/// 抄表记录
	long	itemid;			/// 编号
	long	cmdid;			/// 命令编号
	long	meterid;		/// 表号
	long	rate;			/// 倍率
	double	lf_number;		/// 上次度数
	char	lf_time[24];	/// 上次抄表时间
	double	th_number;		/// 本次度数
	char	th_time[24];	/// 本次抄表时间
	double	delta;			/// 度数
	char	state[16];		/// 状态:正常/反转/外光太强/通讯出错/未知
};

struct ha_centor{	/// 集中器
	long id;				/// 编号，实际表地址中以及表的centorid都是用这个
	long centorid;			/// 设备编号
	char ver[16];			/// 版本
	char rtime[24];			/// 固定抄表日期时间:2 11:50
	char setuptime[24];		/// 安装日期时间
	char addr[64];			/// 安装地点:广州市蓝天花园小区
	char description[128];	/// 说明:COM1:baud=9600 parity=N data=8 stop=1
	char state[32];			/// 连接状态
	char telnumber[32];		/// 拨号电话号码
};

/// 命令执行结果
struct ha_cmd_result{
	long rows;		/// 返回结果记录数
	char * pbuf;	/// 返回结果
};


#pragma pack(push, 1)

/////////////////////////////////////////////////////////////////////////
// 表册所用资料
//
struct mt_rgn{		/// 行政区
	char id[2];				/// ID
	char name[32];			/// 名称
	char shortName[9];		/// 简称
	uint16  iArea;			/// 社区索引
	uint16  nArea;			/// 社区数
};


struct mt_area{		/// 社区
	char no[6];				/// ID
	char shortName[16];		/// 简称
	uint16  iCentorLine;	/// 集采线路索引
	uint16  nCentorLines;	/// 集采线路数
	uint16  extPos;			/// 详细资料地址索引
};

struct mt_area_ext{	///   社区的扩展信息
	char name[64];			/// 名称
	char addr[128];			/// 地址
};

struct mt_centorLine{	/// 集采线路
	char no[11];			/// 编号
	uint16  iMeter;			/// 起始表记录
	uint16  nMeters;		/// 表数
	uint16  nUnuse;			/// 未启用数
	uint16  nChecked;		/// 已抄数
};

/// 表的详细说明格式如下：
/// 上月用量,平均用量,户主名,户主联系方式
struct mt_meter{	/// 表具
	char	meterID[13];	/// 表编号
	char	state;			/// 状态 G/M/Z/0/1/9 = 估抄/门首/自报/未抄/正确/通讯失败
							/// 在手持器中用G等一个字母表示,计算机数据库端存全的信息
	int32	n;				/// 用量
	int32	lastUsage;		/// 最近用量 
	int32	lastNumber;		/// 度数
	uint16	pos;			/// 详情所在文件位置
};

struct mt_meter_ext{/// 表具的辅助资料
	char centorNo[11];		/// 集采线路的ID
	char roomID[32];		/// 房间号
	char user[32];			/// 业主
	char tel[32];			/// 联系电话
};

#pragma pack(pop)

extern ha_cmd_param_type g_params_types[];
extern ha_cmdType g_cmdTypes[];

extern ha_cmd_param g_params[8];	/// 输入参数

extern char * g_sendbuf;	/// 发送缓冲
extern char * g_recvbuf;	/// 接收缓冲

extern int	g_icmd;		/// 当前命令
extern int	g_nparms;	/// 当前命令的参数个数

extern xlist<ha_centor> g_centors;
extern xlist<ha_cmd>	g_cmds;

int ht_str2bcd(char * d, const char * s, int len);
int ht_bcd2str(char * d, const char * s, int len);
int ht_bcd2long(const char *s, int bytes);
void ht_long2bcd(char * d, long v, int bytes);
int ht_hex2long(const char * s);
int ht_hex2array(char * d, const char * s, int len);
void ht_long2hex(char * d, long l);
int ht_array2hex(char * d, const unsigned char * s, int len);
char * ht_lpad(char * d, char l);

void trimstr0  (char * d);  // 去掉前面的0字符

void ht_revers(unsigned char * s , int n);

/// 返回 b 缓冲 l 长度数据的 CRC 值
unsigned char ir_sumchk(unsigned char * b, int l);

/////////////////////////////////////////////////
// 表状态
// 'w': 未抄
// 'M': 门示
// 'W': 未用
// 'G': 估抄
// 'Z': 自报
// 'T': 停表
// '0': 正常
// '1': 表内一组光管坏，测量正确
// '2': 表具正发生光透射干扰
// '3': 故障
// '8': 表具正发生强光干扰
// '9': 表具通讯错误
// 
// 数字时,第3位为拉合阀状态

bool ht_isok(char state);		// 是否正常
bool ht_isOpened(char state);	// 是否合阀
bool ht_isChecked(char state);	// 是否抄收

const char * ht_flag2state(char flag);
char ht_state2flag(const char * state);