#include <xsys.h>

#include <ss_service.h>
#include <l_str.h>

#include "ht_pms.h"

int kh_inparams(ha_cmdType * pcmd, const char * pparms)
{
	int i, j;

	// ȡ�����붨��
	for (i = 0; pcmd->pdescription[i]; i++) {
		g_params[i].ptype = g_params_types + (pcmd->pdescription[i]-'0');
	}
	g_nparms = i;

	// ��ʾ����
	WriteToEventLog("%s", pcmd->pname);

	const char * p = (char *)pparms;
	// ��ʾ������ʾ
	for (i = 0; i < g_nparms && *p; i++) {
		p = getaword(g_params[i].value, p, ',');
		WriteToEventLog("%s:%s", g_params[i].ptype->pname, g_params[i].value);
	}

	// �����������
	int len = 10 + pcmd->pcmd_format[9];
	memcpy(g_sendbuf+3, pcmd->pcmd_format, len+1);
	p = strchr(g_sendbuf, 'P');
	if (p == 0){
		p = strchr(g_sendbuf+16, 'T');
	}

	if (p){
		int pos = int(p-g_sendbuf);
		j = 0;
		for (i = 0; i < g_nparms && j < 8; i++) {
			ht_lpad(g_params[i].value, g_params[i].ptype->len);
			ht_str2bcd(g_sendbuf+j+pos, g_params[i].value, g_params[i].ptype->len);
			j += g_params[i].ptype->len / 2;
		}

		ht_revers((unsigned char *)(g_sendbuf+pos), j);
	}

	// calc crc, include 68
	g_sendbuf[len + 3] = ir_sumchk((unsigned char*)g_sendbuf+2, len+1);
	g_sendbuf[len + 4] = KT_TAIL;

	/*
	for (i = 0; i < j+2; i++){
		printf("%02X ", (unsigned char)g_sendbuf[i]);
	}
	printf("\n");
	//*/

	return j;
}
