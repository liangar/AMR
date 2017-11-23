#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "ht_cmn.h"

int ht_str2bcd(char * d, const char * s, int len)
{
	int l, i, j;

	memset(d, 0, (len+1)/2);

	l = strlen(s);
	for (i = j = 0; s[i] != '\0' && i+1 < l; i += 2, j++) {
		d[j] = (s[i+1] & 0xf) | ((s[i] & 0xf) << 4);
	}
	if (s[i] != '\0') {
		d[j++] = (s[i] & 0xf);
	}
	return j;
}

int ht_bcd2str(char * d, const char * s, int len)
{
	int i;
	for (i = len-1; i >= 0; i--) {
		if ((*d = ((s[i] & 0xf0) >> 4) & 0x0f) <= 9)
			*d |= 0x30;
		else
			*d = 'A' + (*d - 0x0a);
		d++;
		*d++ = (s[i] & 0x0f) | 0x30;
	}
	*d = 0;
	return 2*len;
}

int ht_bcd2long(const char *s, int bytes)
{
	int l = 0, v = 1;
	int i;
	for (i = 0; i < bytes; i++){
		l = l + (((s[0] & 0xf0) >> 4) * 10 + (s[0] & 0x0f)) * v;
		v *= 100;
	}
	return l;
}

void ht_long2bcd(char * d, long v, int bytes)
{
	int i, j;
	memset(d, 0, bytes);
	for (i = 0; v > 0 && i < bytes; i++){
		j = v % 100;
		d[i] = ((j / 10) << 4) | (j % 10);
		v /= 100;
	}
}

static int hasi_hex2long(char c)
{
	if (c >= 'A' && c <= 'Z'){
		return (int)(c - 'A' + 10);
	}else if (c >= 'a' && c <= 'z'){
		return (int)(c - 'a' + 10);
	}else if (c >= '0' && c <= '9'){
		return (int)(c - '0');
	}

	return -1;
}

int ht_hex2long(const char * s)
{
	int l0,l1;
	l0 = hasi_hex2long(s[0]);
	if (l0 < 0)
		return l0;
	l1 = hasi_hex2long(s[1]);
	if (l1 < 0)
		return l1;
	return ((l0 << 4) & l1);
}

void ht_long2hex(char * d, long l)
{
	int n = (l & 0xf0);
	if (n >= 10)
		*d++ = n - 10 + 'A';
	else
		*d++ = n - 10 + '0';
	n = (l & 0x0f);

	if (n >= 10)
		*d = n - 10 + 'A';
	else
		*d = n - 10 + '0';
}

#ifndef isxdigit
bool isxdigit(unsigned char h)
{
	return ((h >= '0' && h <= '9') || (h >= 'A' && h <= 'F') || (h >= 'a' && h <= 'f'));
}
#endif // isxdigit

/// "1A2B3C4D" -> 0x1A 0x2B 0x3C 0x4D
/// "1A2B3C4D5" -> 0x1A 0x2B 0x3C 0x4D 0x05
int ht_hex2array(char * d, const char * s, int len)
{
	int l, i, j;

	memset(d, 0, (len+1)/2);

	l = strlen(s);
	for (i = j = 0; s[i] != '\0' && i+1 < l;) {
		int s0 = hasi_hex2long(s[i++]);
		int s1 = hasi_hex2long(s[i++]);
		if (s0 < 0 || s1 < 0)
			return -1;

		d[j++] = s1 | (s0 << 4);
		if (s[i] && !isxdigit(s[i]))
			++i;
//		printf("%02X ", d[j]);
	}
	if (s[i] != '\0') {
		int s0 = hasi_hex2long(s[i]);
		if (s0 < 0)
			return -1;

		d[j++] = (s0 & 0xf);
	}
//	printf("\n");

	return j;
}

// 0x1A 0x2B 0x3C 0x4D 0x05 -> "1A2B3C4D05"
int ht_array2hex(char * d, const unsigned char * s, int len)
{
	int l = (len + 1) / 2;
	int i, j;
	for (i = j = 0; i < l; i++){
		sprintf(d+j, "%02X", s[i]);  j += 2;
	}
	d[len] = 0;

	return len;
}

char * ht_lpad(char * d, char l)
{
	char len = (char)(strlen(d));
	l -= len;
	if (l > 0){
		memmove(d+l, d, len+1);
		memcpy(d, "00000000000000000000000000", l);
	}
	return d;
}

unsigned char ir_sumchk(unsigned char * b, int l)
{
	unsigned int s = 0;
	for (int i = 0; i < l; i++) {
		s += b[i];
	}
	return (unsigned char)(s & 0xFF);
}

void trimstr0(char * d)
{
	char * s;
	for (s = d; *s == '0' || *s == ' '; s++)
		;
	if (d != s){
		if (*s == '\0'){
			d[1] = '\0';	// ֻ����һ��0
		}else{
			do{
				*d++ = *s++;
			}while(*s);
			*d = '\0';
		}
	}
}

// �Ƿ�����
bool ht_isok(char state)
{
	return (state == '0' || state == '4' ||
			(state > '9' && state != 'W' && state != 'w' && state != 'A' && state != 'E'));
}

// �Ƿ�Ϸ�
bool ht_isOpened(char state)
{
	return (((state-'0') & 0x04) != 0);
}

// �Ƿ���
bool ht_isChecked(char state)
{
	return (state != 'w' && state != 'W');
}

static const char * s_meterStates[] = {
	"����", "��߹���", "���ݸ���", "��߹���",
	"�ط�", "�ط�����", "��ͨ������", "��߹���",
	"ǿ�����", "ͨѶ����",
	0
};

const char * ht_flag2state(char flag)
{
	switch(flag){
	case '0':  return "����";
	case '4':  return "�ط�";	// 0100  ===E

	case '1':  return "��߹���";	// 0001  ===F
	case '2':  return "���ݸ���";	// 0010  ===C
	case '5':  return "�ط�����";	// 0101  ===5
	case '6':  return "��ͨ������";	// 0110  ===D
	case '3':						// 0011  ===C
	case '7':  return "��߹���";	// 0111  ===7
	case '8':  return "ǿ�����";	// 1000  ===B
	case '9':  return "ͨѶ����";	// 1001  ===A
	case 'A':  return "��������";	// 1001  ===A
	case 'E':  return "�ط�����";	// 1001  ===A

	case 'w':  return "δ��";
	case 'M':  return "��ʾ";
	case 'W':  return "δ��";
	case 'G':  return "����";
	case 'Z':  return "�Ա�";
	case 'T':  return "ͣ��";
	}
	return "δ֪����";
}

char ht_state2flag(const char * state)
{
	if (strcmp(state, "����") == 0)  return '0';
	if (strcmp(state, "��߹���") == 0)  return '1';
	if (strcmp(state, "��ͨ������") == 0)  return '2';
	if (strcmp(state, "��߹���") == 0)  return '3';
	if (strcmp(state, "�ط�") == 0)  return '4';
	if (strcmp(state, "�ط�����") == 0)  return '5';
	if (strcmp(state, "��ͨ������") == 0)  return '6';
	if (strcmp(state, "��߹���") == 0)  return '7';

	if (strcmp(state, "ǿ�����") == 0)  return '8';
	if (strcmp(state, "ͨѶ����") == 0)  return '9';

	if (strcmp(state, "δ��") == 0)  return 'w';
	if (strcmp(state, "��ʾ") == 0)  return 'M';
	if (strcmp(state, "δ��") == 0)  return 'W';
	if (strcmp(state, "����") == 0)  return 'G';
	if (strcmp(state, "�Ա�") == 0)  return 'Z';
	if (strcmp(state, "ͣ��") == 0)  return 'T';

	return 'W';
}

void ht_revers(unsigned char * s , int n)
{
	unsigned char t;
	for (int i = 0, j = n - 1; i < j; i++, j--){
		t = s[i];
		s[i] = s[j];
		s[j] = t;
	}
}
