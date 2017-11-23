#include <xsys.h>

#include <ss_service.h>
#include <l_str.h>

#include "ht_pms.h"

int kh_inparams(ha_cmdType * pcmd, const char * pparms)
{
	int i, j;

	memset(g_params, 0, sizeof(g_params));

	// 取得输入定义
	for (i = 0; pcmd->pdescription[i]; i++) {
		g_params[i].ptype = g_params_types + (pcmd->pdescription[i]-'0');
	}
	g_nparms = i;

	// 取得输入
	// 提示命令
	WriteToEventLog("%s", pcmd->pname);

	const char * p = pparms;
	// 提示输入显示
	for (i = 0; i < g_nparms && *p; i++) {
		p = getaword(g_params[i].value, p, ',');
		WriteToEventLog("%s:%s", g_params[i].ptype->pname, g_params[i].value);
	}

	// 构造命令缓冲区
	memset(g_sendbuf+3, 0, 8);
	g_sendbuf[11] = pcmd->pcmd_format[8];
	g_sendbuf[12] = pcmd->pcmd_format[9];

	if (g_nparms > 0 && pcmd->pdescription[0] == '0'){
		j = 6;
		g_sendbuf[3] = '\x80';
	}else
		j = 0;
	for (i = 0; i < g_nparms && j < 8; i++) {
		ht_lpad(g_params[i].value, g_params[i].ptype->len);
		if (g_params[i].ptype->type != 'H')
			ht_str2bcd  (g_sendbuf+j+3, g_params[i].value, g_params[i].ptype->len);
		else
			ht_hex2array(g_sendbuf+j+3, g_params[i].value, g_params[i].ptype->len);

		j += g_params[i].ptype->len / 2;
	}
	for (j = 10; i < g_nparms; i++) {
		ht_lpad(g_params[i].value, g_params[i].ptype->len);
		ht_str2bcd(g_sendbuf+j+3, g_params[i].value, g_params[i].ptype->len);
		j += g_params[i].ptype->len / 2;
	}
	// calc crc
	g_sendbuf[13 + g_sendbuf[12]] = ir_sumchk((unsigned char*)g_sendbuf+3, g_sendbuf[12]+10);
	g_sendbuf[14 + g_sendbuf[12]] = KT_TAIL;

	/*
	for (i = 0; i < j+2; i++){
		printf("%02X ", (unsigned char)g_sendbuf[i]);
	}
	printf("\n");
	//*/

	return j;
}
