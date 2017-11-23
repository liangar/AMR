#include <stdio.h>
#include <string.h>

#include "ht_cmn.h"
#include "ht_show.h"

/// ��ʽ(19 bytes):
/// <1 ������> 0
/// <3 ���̺�> 1 - 3
/// <8 ��ߺ�> 4 - 11
/// <1 ��չλ> 12 = '0'
/// <5 ��  ��> 13 - 17
/// <2 ״̬λ> 18 - 19
/// <1 ����λ> 18 = '\0'
int db_put_rec(char * d, char * s)
{
	// ����Ѿ���������, �ͷ��سɹ�
	if (*d != 'E')
		return KT_RECLEN_GY;

	// ����¼�����Ƿ���ȷ
	if (ir_sumchk((unsigned char *)s, KT_REC_RECVLEN-1) != (unsigned char)s[KT_REC_RECVLEN-1]){
		*d = 'E';
		return -1;
	}

	ht_bcd2str(d, s+7, 1);	// ������

	// ������
	switch (d[1]){
	case '0':  d[0] = 'S';  break;
	case '1':  d[0] = 'D';  break;
	case '2':  d[0] = 'Q';  break;
	default :  d[0] = d[1]; break;
	}
	ht_bcd2str(d+2, s+5, 2);	// ���̱��
	memmove(d+1, d+5, 1);

	ht_bcd2str(d+2, s, 5);	// ���,��8λ
	// memmove(d+4, d+6, 8);

	ht_bcd2str(d+12,s+8, 3);
	d[18] = '0';
	d[19] = d[12];
	d[12] = '0';

	d[20] = '\n';
	d[21] = 0;

	return KT_RECLEN_GY;
}

int db_put_recs(char * d, char * s, int recs)
{
	int i, r, errors;
	for (i = errors = 0; i < recs; i++, d+=KT_RECLEN_GY, s+=KT_REC_RECVLEN){
		r = db_put_rec(d, s);
		if (r < KT_RECLEN_GY){
			errors++;
		}
	}
	*d = 0;
	return errors;
}

/*
static void show_data(const char * s)
{
	char t[4];

	EL_puts(s+4, 8);
	saystr("  ",2);
	saystr(s+13,5);

	switch(s[19]){
	case '0':  t[0] = ' ';  break;
	case '9':  t[0] = 'A';  break;  // ͨѶ���ϣ�1001  ===A
	case '8':  t[0] = 'B';  break;  // ���չ��ϣ�1000  ===B
	case '2':  t[0] = 'C';  break;  // �����ݣ�  0010  ===C
	case '6':  t[0] = 'D';  break;  // ���ݹ��ϣ�0110  ===D
	case '4':  t[0] = 'E';  break;  // ���ݷ�����0100  ===E
	case '1':  t[0] = 'F';  break;  // ��ת    ��0001  ===F
	default :  t[0] = '#';  break;
	}
	t[1] = '\0';

	saystr(t, 1);
}

void db_show_n_recs(const char * p, int n, int begin_row)
{
	int j, l;

	l = strlen(p);

	for (j = 0; j < l; j+=KT_RECLEN_GY, p+=KT_RECLEN_GY){
		toxy(1, begin_row+j);
		show_data(p);
	}
}
//*/
