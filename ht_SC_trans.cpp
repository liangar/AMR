#include <xsys.h>

#include <ss_service.h>
#include <l_str.h>

#include "ht_inivars.h"
#include "ht_show.h"
#include "ht_trans.h"

// #include "db/xha_cmd.h"

extern void AddMessageText(const char * msg);

static char s_msg[1024];
static char s_showmsg[1024];
void WriteToLog(const char * pFormat, ...)
{
    va_list args;

	va_start(args, pFormat);
	vsprintf(s_msg, pFormat, args);
	char sznow[32];
	getnowtime(sznow);
	sprintf(s_showmsg, "%s\t%s", sznow, s_msg);
	WriteToEventLog(s_msg);
	AddMessageText(s_showmsg);
    va_end(args);
}

void WriteRecvError(int i, char * buf)
{
	char msg[2048];

	int l = sprintf(msg, "接收错误: %6d ", i);
	if (i > 1 || -i > 1){
		if (i < 0)  i = -i + 1;
		for (int j = 0; j < i; j++){
			l += sprintf(msg+l, "%02X ", (unsigned char)buf[j]);
		}
	}
	WriteToLog(msg);
}

void HX_recover_bcd(unsigned char * d, int len)
{
	for (int i = 0; i < len; i++)
		d[i] -= 0x33;
}

kh_trans::kh_trans()
	: CComm(0, 0)
	, m_bdialed(false)
	, m_recvbuf(0)
{
	m_pcmd = 0;
	memset(m_sendbuf, 0, sizeof(m_sendbuf));
	memset(m_serialport_string, 0, sizeof(m_serialport_string));
	memset(m_telnumber, 0, sizeof(m_telnumber));
}

int kh_trans::trans(void)
{
	int i;
	unsigned char c;

	m_msg[0] = m_recvbuf[0] = 0;
	clear_all_recv();
	send_record();

	if (g_cmdTypes[g_icmd].pcmd_format[8] == '\01')
		Sleep(100);
	else
		Sleep(400);

	m_recs = 0;
	m_steps= 0;

	WriteToLog("正在接收.....");
	memset(&m_result, 0, sizeof(m_result));
	m_result.pbuf = m_recvbuf + 1024;

	clear_recv_buf();

	m_isSingle = true;
	m_singleState = '0';

	switch(g_cmdTypes[g_icmd].nrcv_type) {
	case '0':	// get ack
		m_singleState = 'F';

		/* for test
		memcpy(m_recvbuf, "\xFE\xFE\x68\xF1\x16", 5);
		i = 5;
		// end for test */

		i = recv_bytes(5);
		if (i < 0){
			strcpy(m_msg, "执行失败...");
		}if (memcmp(m_recvbuf, KT_HEAD, 3) != 0 || m_recvbuf[4] != KT_TAIL){
			i = -100;
			strcpy(m_msg, "执行失败.");
		}else if (m_recvbuf[3] == KT_ACK){
			m_result.rows = 1;
			strcpy(m_msg, "执行正确");
			m_singleState = '0';
		}else{
			strcpy(m_msg, "执行失败");
			i = -5;
		}
		break;
	case 'n':	// n
		WriteToLog("正在抄收: %6s", g_params[0].value);
		i = recv_recs();
		if (i == 0){
			WriteToLog("没有数据");
		}else if (i < 0){
			WriteToLog("出错: %d", i);
		}else{
			WriteToLog("正确: 共抄得(%d)个表度数", i);
		}
		break;
	default:	// 1
		/* for test
		i = 5;
		memcpy(m_recvbuf+1, "\x00\x12\x81", 4);
		goto for_test_end;
		//*/

		i = recv_rec();
		if (i < 0 /*||
			((unsigned char)m_sendbuf[12] | '\x80') != (unsigned char)m_recvbuf[8]*/
			)
		{
			sprintf(m_msg, "接收错误: %d", i);
			WriteRecvError(i, m_recvbuf);
			m_singleState = -1;
			i = -101;
		}else{
			c = ir_sumchk((unsigned char *)m_recvbuf, i-2);
			if (c != (unsigned char)m_recvbuf[i-2]){
				WriteRecvError(i, m_recvbuf);
				m_singleState = -2;
				i = -100;
			}else{
for_test_end:
				m_result.rows = 1;
				m_singleState = '0';

				// 返回的地址字段在 2, 7
				ht_bcd2str(m_result.pbuf, m_recvbuf+2,  7);
				// memmove(m_result.pbuf, m_result.pbuf + 4, 10);

				// 调试用单表抄收
				if (g_icmd == 0){
					m_result.pbuf[14] = ',';
					// 度数
					ht_bcd2str(m_result.pbuf+14, m_recvbuf+14, 4);
					m_result.pbuf[23] = '\0';
				}else
					m_result.pbuf[14] = '\0';
				WriteToEventLog(m_result.pbuf);

				// if (g_icmd == 0)  m_result.pbuf[6] = 0;	// 去掉后面2位数字

				sprintf(m_msg, "OK: %s", m_result.pbuf);
			}
		}
		break;
	}

	return i;
}

int kh_trans::init(int id)
{
	static const char szFunctionName[] = "kh_trans::init";

	int j;
	for (j = 0; j < g_centors.m_count; j++){
		if (g_centors[j].id == id)
			break;
	}
	if (j == g_centors.m_count){
		sprintf(m_msg, "%s: 找不到集中器定义(编号 = %d)", szFunctionName, id);
		WriteToLog(m_msg);
		return -1;
	}
	if (init(g_centors[j].description, g_centors[j].telnumber) < 0)
		return -2;

	return j;
}

int kh_trans::init(const char * serialParam, const char * telnumber)	/// 初始化通讯端口
{
	static const char szFunctionName[] = "kh_trans::init";

	WriteToLog("%s: 初始化通讯端口...", szFunctionName);

	if ((_stricmp(serialParam, m_serialport_string) != 0 && m_serialport_string[0] != 0) ||
		(_stricmp(telnumber, m_telnumber) != 0 && m_telnumber[0] != 0) ){
		if (IsOpen())
			down();
	}

	if (IsOpen()){
		sprintf(m_msg, "%s: 端口已经打开", szFunctionName);
		WriteToLog(m_msg);
		// g_xcmd.report_do(m_msg);
		return 0;
	}

	if (m_recvbuf == 0){
		m_recvbuf = new char[64 * 1024];
		memset(m_recvbuf, 0, 64*1024);
	}
	m_bdialed = false;
	int r = Open(serialParam);
	if (r == 0){
		strcpy(m_recvbuf, xlasterror());
		trim_tail(m_recvbuf, m_recvbuf);
		WriteToLog(m_recvbuf);
		sprintf(m_msg, "%s: 不能打开串口(%s)", szFunctionName, serialParam);
		WriteToLog(m_msg);
		// g_xcmd.report_do(m_msg);
		return -1;
	}
	strcpy(m_serialport_string, serialParam);
	Sleep(200);

	strcpy(m_telnumber, telnumber);

	if (m_telnumber && m_telnumber[0]){
		char b[64];
		r = send_AT_cmd("ATH1\r\n", "OK", 2500);
		if (r > 0){
			Sleep(m_hook_wait);
			sprintf(b, "ATDT%s\r\n", m_telnumber);
			sprintf(m_msg, "正在拨号(%s)...", m_telnumber);
			// g_xcmd.report_do(m_msg);
			WriteToLog(m_msg);

			r = send_AT_cmd(b, "CONNECT", m_dail_wait*1000);
			if (r <= 0){
				sprintf(m_msg, "%s: 拨号出错, (%s)", szFunctionName, m_recvbuf);
				send_AT_cmd("ATH0\r\n", "OK", m_hangup_wait);
			}else{
				m_bdialed = true;
				strcpy(m_msg, "拨号完成,正在抄收...");
			}
			{
				char t0[32];
				getnowtime(t0);
				WriteToLog("%s: %s", szFunctionName, t0);
			}

			// 等待对方稳定(处理RING等指令)
			Sleep(1500);
		}else{
			if (m_recvbuf[0] == 0)
				sprintf(m_msg, "%s: 不能摘机, 估计Modem没有连接上", szFunctionName);
			else
				sprintf(m_msg, "%s: 不能摘机, (%s)", szFunctionName, m_recvbuf);
		}
		WriteToLog(m_msg);
		// g_xcmd.report_do(m_msg);
	}

	if (r >= 0){
		WriteToLog("%s: 已打开通讯端口...", szFunctionName);
		memcpy(g_sendbuf, KT_HEAD, 3);
	}else{
		WriteToLog("%s: 打开通讯端口出错", szFunctionName);
		Close();
	}

	return r;
}

void kh_trans::down(void)	/// 关闭关闭通讯端口
{
	int r = 0;
	if (m_bdialed){
		WriteToLog("开始挂断");
		for (int i = 0; i < 3 && r <= 0; i++){
			r = send_AT_cmd("+++", "OK", 2000);
			Sleep(1000);
			if (r > 0)
				r = send_AT_cmd("ATH0\r\n", "OK", m_hangup_wait);
		}
		m_bdialed = false;
	}
	Close();
	WriteToLog("通讯端口关闭");

	if (m_recvbuf){
		delete[] m_recvbuf;  m_recvbuf = 0;
		WriteToLog("内存释放完毕");
	}
}

int kh_trans::send_byte(char c)
{
	/*
	WriteToLog("%02X ", (unsigned char)c);
	//*/

	if (!WriteComm(&c, 1))
		return -1;

	return 1;
}

int kh_trans::send_AT_cmd(const char * p, const char * okstring, int timeout)
{
	clear_all_recv();

	int r = send_bytes(p);
	if (r < 0)
		return r;
	/// 大多数指令会将原命令返回
	if (memcmp(p, "ATDT", 4) == 0)
		Sleep(5000);
	else
		Sleep(500);

	if (strcmp(p, "+++") != 0){
		r += 2;
		r = Recv(m_recvbuf, r, timeout);
	}else
		r = 0;

	/// 接收指令执行结果
	if (r >= 0){
		m_recvbuf[r] = 0;
		r += RecvByEnd(m_recvbuf+r, 64, "\r\n", timeout);
	}
	if (r < 0){
		strcpy(m_msg, xlasterror());
		trim_tail(m_msg, m_msg);
		WriteToLog(m_msg);
		return r;
	}

	m_recvbuf[r] = 0;

	/// 判断执行结果
	if (strstr(m_recvbuf, okstring) != 0)
		return r;

	return (r == 0)?-1:-r;
}

int kh_trans::send_bytes(const char * p)
{
	return send_bytes(p, strlen(p));
}

int kh_trans::send_bytes(const char * p, int l)
{
	//*
	int k = 0;
	for (int i = 0; i < l; i++){
		k += sprintf(m_msg+k, "%02X ", (unsigned char)p[i]);
	}
	WriteToEventLog(m_msg);
	//*/

	//*
	if (!WriteComm(p, l)){
		sprintf(m_msg, "发送错误: %s", xlasterror());
		trim_tail(m_msg, m_msg);
		WriteToLog(m_msg);
		return -1;
	}
	//*/

	return l;
}

int kh_trans::send_bytes(int l)
{
	return send_bytes(m_sendbuf, l);
}

int kh_trans::send_record(void)
{
	WriteToLog("正在发送.....");

	int l = 15 + m_sendbuf[12];

	l = send_bytes(m_sendbuf, l);

	return l;
}

int kh_trans::recv_byte(char * c, int timeout)
{
	return Recv(c, 1, timeout);
}

int kh_trans::peek_byte(char * c)
{
	return Peek(c, 1);
}

int kh_trans::clear_recv_buf(void)
{
	char c;

	int i;
	for (i = 0; i < 350; ++i) {
		if (peek_byte(&c) <= 0)
			break;

		if (c == '\xFE')
			break;

		recv_byte(m_recvbuf+i, 0);	// 读取字符
	}
	return i;
}

int kh_trans::clear_all_recv(void)
{
	char c;
	int i;
	for (i = 0; i < 350; ++i) {
		if (peek_byte(&c) <= 0)
			break;

		recv_byte(m_recvbuf+i, 0);	// 读取字符
	}
	return i;
}

int kh_trans::recv_bytes(int n)
{
	int i;
	int k = sprintf(m_msg, "recv: ");

	for (i = 0; i < n; i++) {
		if (recv_byte(m_recvbuf + i, 3000) <= 0){
			n = -(i+1);
			break;
		}
		k += sprintf(m_msg+k, "%02X ", (unsigned char)m_recvbuf[i]);
	}
	sprintf(m_msg+k, " All = %d/%d", i, n);
	WriteToEventLog(m_msg);

	return n;
}

int kh_trans::sendack(char isok)
{
	m_sendbuf[13] = '\x01';
	m_sendbuf[14] = (isok ? '\xF1' : '\xF2');	// set ack
	m_sendbuf[15] = ir_sumchk((unsigned char *)(m_sendbuf+2), 11);
	m_sendbuf[16] = KT_TAIL;

	return send_record();
}

void kh_trans::show_recv_buf(void)
{
	char i, j;

	strcpy(m_msg, "-> : ");
	int l = strlen(m_msg);
	for (i = 0; i < 2; i++){
		for (j = 0; j < 8; j++)
			l += sprintf(m_msg+l, "%02X", (unsigned char)m_sendbuf[i*8 + j]);
	}
	WriteToLog(m_msg);

	strcpy(m_msg, "<- : ");
	l = strlen(m_msg);
	for (i = 0; i < 6; i++){
		for (j = 0; j < 8; j++)
			l += sprintf(m_msg+l, "%02X", (unsigned char)m_recvbuf[i*8 + j]);
	}
	WriteToLog(m_msg);

	WriteToLog("ERR: steps = %d", m_steps);
}

// 0100120012345678:1,00456
static char szProcessing[] = "-\\|-";
void kh_trans::showProcessing(char i, char n)
{
	char * p = m_recvbuf + 1024;

	sprintf(m_msg, "%c - %3d : %2d, %0X",
		szProcessing[(i&0x03)],
		n,
		i,
		(unsigned char)(m_sendbuf[13])
	);
	WriteToLog(m_msg);
	// g_xcmd.report_do(m_msg);

	if (n > 6){
		n = 5;
	}
}

void kh_trans::re_send(char recs)
{
	// printf("recs = %d", recs);

	int r;
	if (recs == 0)
		r = send_record();
	else{
		r = send_bytes(16);
		/*
		for (int i = 0; i < 16; i++){
			printf("%02X ", (unsigned char)m_sendbuf[i]);
		}
		printf("\n");
		//*/
	}
	if (r < 0){
		WriteToLog("re_send: error(%d)", r);
	}
}

// KT_HEAD <A> 68 <L> <T> <R> KT_TAIL
int kh_trans::recv_recs(void)
{
	int i, l = 1, recs;
	unsigned char cmd[16];

	memset(&m_result, 0, sizeof(m_result));
	m_result.pbuf = m_recvbuf + 1024;
	m_recs = 0;

	for (i = 0; i < 100; i++) {
		for (int j = 0; j < 10; j++){
			recs = 0;
			m_steps = 1;

			// 如果接收不成功，则重新发送命令
			Sleep(700);	// 由于之前发送过命令，这里需要等待一下对方准备数据

			// 清空不是开始字符的数据
			clear_recv_buf();

			memset(m_recvbuf, 0, 64);
			int r = recv_bytes(13);
			if (r < 0) {
				sprintf(m_msg, "收不到回应(%d)", r);
				WriteToLog(m_msg);

				strcpy(m_msg, xlasterror());
				trim_tail(m_msg, m_msg);
				WriteToLog(m_msg);

				re_send(recs);
				continue;
			}

			++m_steps;

			// check head
			if (memcmp(m_recvbuf, KT_HEAD, 3) != 0 ||
				m_recvbuf[11] != '\x68')
			{
				strcpy(m_msg, "回应出错");
				WriteToLog(m_msg);
				re_send(recs);
				continue;
			}

			// 缓存命令
			memcpy(cmd, m_recvbuf, 13);

			l = (int)(cmd[12]);

			++m_steps;

			if (l > 16*KT_REC_RECVLEN){
				strcpy(m_msg, "回应头要求数据过长");
				WriteToLog(m_msg);
				re_send(recs);
				continue;
			}

			// get recs
			if ((r = recv_bytes(l+2)) < 0) {
				sprintf(m_msg, "不能读回应数据(%d)", r);
				WriteToLog(m_msg);
				re_send(recs);
				continue;
			}

			++m_steps;

			// 准备发送回应
			memcpy(m_sendbuf, cmd, 12);
			m_sendbuf[12] = '\x01';
			m_sendbuf[15] = KT_TAIL;
			// check tail
			if (m_recvbuf[l+1] == KT_TAIL) {
				recs = l/KT_REC_RECVLEN;
				m_sendbuf[13] = KT_ACK;
			}else{
				m_sendbuf[13] = KT_ERR;
			}
			m_sendbuf[14] = ir_sumchk((unsigned char *)m_sendbuf+3, 11);

			showProcessing(i, recs);

			if (m_sendbuf[13] == KT_ERR){
				// 出错要打印接收情况
				show_recv_buf();
			}else if (recs > 0){
				// 保存数据
				db_put_recs(m_result.pbuf + m_recs * KT_RECLEN_GY, m_recvbuf, recs);
				m_recs += recs;
			}

			// will send ack
			send_bytes(16);
			if (m_sendbuf[13] == KT_ACK)
				break;
		}
		if (recs <= 0)
			break;
	}

	m_result.rows = m_recs;

	return m_recs;
}

int kh_trans::recv_rec(void)
{
	char c = 0;

	for (int i = 0; i < 350; ++i) {
		if (recv_byte(&c, 100) <= 0)
			break;
		
		if (c != '\xFE')
			break;
	}
	if (c != '\x68')
		return -1;

	int n = recv_bytes(g_cmdTypes[g_icmd].nrcv_record_length + 15);
	if (n > 0){
		memmove(m_recvbuf+1, m_recvbuf, n);
		m_recvbuf[0] = c;

		return n+1;
	}
	return n;
}
