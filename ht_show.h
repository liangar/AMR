#pragma once

/// 将数据解析入缓冲区
/// param d: 缓冲区
/// param s: 数据
/// param recs: 数据包含的记录数
/// return : 不能解析的记录数
int db_put_recs(char * d, char * s, int recs);
