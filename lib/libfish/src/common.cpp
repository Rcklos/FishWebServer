/*================================================================*
        Copyright (C) 2021 All rights reserved, www.lentme.cn
      	文件名称：common.cpp
      	创 建 者：fish
      	创建日期：2022年9月18日
 *================================================================*/

#include "common.h"

FISH_NAMESPACE_START

static int g_fish_log_level = 1;

void FormatDate(std::ostream& os){
    char buf[64];
    time_t t = time(0);
    struct tm *tm = localtime(&t);
    strftime(buf, sizeof(buf), "%X", tm);
    os << buf;
}

int GetLogLevel() {
    return g_fish_log_level;
}

void SetLogLevel(int level) {
    g_fish_log_level = level;
}

FISH_NAMESPACE_END