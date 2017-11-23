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
/// �ڲ���������
///
struct ha_cmdType{
	const char *	pname;				/// ����
	const char *	pcmd_format;		/// ��ʽ˵��
	char			nrcv_type;			/// ��������: ״̬/����/���¼ = '0'/'1'/'n'
	int				nrcv_record_length;	/// ���ռ�¼����
	char *			pdescription;		/// ����
};

/// \struct ha_cmd
/// ���������
typedef struct tag_ha_cmd{
	long	id;				/// ���
	char	cmd[64];		/// ����
	char	parms[64];		/// ����
	long	centorid;		/// ������ID
	char	uid[64];		/// �û�
	char	createtime[24];	/// ����ʱ��
	char	state[16];		/// ״̬:��ִ��/������/����
	char	substate[16];	/// ��״̬:�ȴ�/ִ����,����������:ʧ��/���
	long	tryTimes;		/// ���Դ���:���3��
	char	lastEndTime[24];	/// �����������ʱ��
	char	lastBeginTime[24];	/// �����ʼ����ʱ��
	char	processing[256];	/// ��������
} ha_cmd;

struct ha_record{	/// �����¼
	long	itemid;			/// ���
	long	cmdid;			/// ������
	long	meterid;		/// ���
	long	rate;			/// ����
	double	lf_number;		/// �ϴζ���
	char	lf_time[24];	/// �ϴγ���ʱ��
	double	th_number;		/// ���ζ���
	char	th_time[24];	/// ���γ���ʱ��
	double	delta;			/// ����
	char	state[16];		/// ״̬:����/��ת/���̫ǿ/ͨѶ����/δ֪
};

struct ha_centor{	/// ������
	long id;				/// ��ţ�ʵ�ʱ��ַ���Լ����centorid���������
	long centorid;			/// �豸���
	char ver[16];			/// �汾
	char rtime[24];			/// �̶���������ʱ��:2 11:50
	char setuptime[24];		/// ��װ����ʱ��
	char addr[64];			/// ��װ�ص�:���������컨԰С��
	char description[128];	/// ˵��:COM1:baud=9600 parity=N data=8 stop=1
	char state[32];			/// ����״̬
	char telnumber[32];		/// ���ŵ绰����
};

/// ����ִ�н��
struct ha_cmd_result{
	long rows;		/// ���ؽ����¼��
	char * pbuf;	/// ���ؽ��
};


#pragma pack(push, 1)

/////////////////////////////////////////////////////////////////////////
// �����������
//
struct mt_rgn{		/// ������
	char id[2];				/// ID
	char name[32];			/// ����
	char shortName[9];		/// ���
	uint16  iArea;			/// ��������
	uint16  nArea;			/// ������
};


struct mt_area{		/// ����
	char no[6];				/// ID
	char shortName[16];		/// ���
	uint16  iCentorLine;	/// ������·����
	uint16  nCentorLines;	/// ������·��
	uint16  extPos;			/// ��ϸ���ϵ�ַ����
};

struct mt_area_ext{	///   ��������չ��Ϣ
	char name[64];			/// ����
	char addr[128];			/// ��ַ
};

struct mt_centorLine{	/// ������·
	char no[11];			/// ���
	uint16  iMeter;			/// ��ʼ���¼
	uint16  nMeters;		/// ����
	uint16  nUnuse;			/// δ������
	uint16  nChecked;		/// �ѳ���
};

/// �����ϸ˵����ʽ���£�
/// ��������,ƽ������,������,������ϵ��ʽ
struct mt_meter{	/// ���
	char	meterID[13];	/// ����
	char	state;			/// ״̬ G/M/Z/0/1/9 = ����/����/�Ա�/δ��/��ȷ/ͨѶʧ��
							/// ���ֳ�������G��һ����ĸ��ʾ,��������ݿ�˴�ȫ����Ϣ
	int32	n;				/// ����
	int32	lastUsage;		/// ������� 
	int32	lastNumber;		/// ����
	uint16	pos;			/// ���������ļ�λ��
};

struct mt_meter_ext{/// ��ߵĸ�������
	char centorNo[11];		/// ������·��ID
	char roomID[32];		/// �����
	char user[32];			/// ҵ��
	char tel[32];			/// ��ϵ�绰
};

#pragma pack(pop)

extern ha_cmd_param_type g_params_types[];
extern ha_cmdType g_cmdTypes[];

extern ha_cmd_param g_params[8];	/// �������

extern char * g_sendbuf;	/// ���ͻ���
extern char * g_recvbuf;	/// ���ջ���

extern int	g_icmd;		/// ��ǰ����
extern int	g_nparms;	/// ��ǰ����Ĳ�������

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

void trimstr0  (char * d);  // ȥ��ǰ���0�ַ�

void ht_revers(unsigned char * s , int n);

/// ���� b ���� l �������ݵ� CRC ֵ
unsigned char ir_sumchk(unsigned char * b, int l);

/////////////////////////////////////////////////
// ��״̬
// 'w': δ��
// 'M': ��ʾ
// 'W': δ��
// 'G': ����
// 'Z': �Ա�
// 'T': ͣ��
// '0': ����
// '1': ����һ���ܻ���������ȷ
// '2': �����������͸�����
// '3': ����
// '8': ���������ǿ�����
// '9': ���ͨѶ����
// 
// ����ʱ,��3λΪ���Ϸ�״̬

bool ht_isok(char state);		// �Ƿ�����
bool ht_isOpened(char state);	// �Ƿ�Ϸ�
bool ht_isChecked(char state);	// �Ƿ���

const char * ht_flag2state(char flag);
char ht_state2flag(const char * state);