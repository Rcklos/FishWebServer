/*================================================================*
        Copyright (C) 2021 All rights reserved, www.lentme.cn
      	文件名称：common.h
      	创 建 者：fish
      	创建日期：2022年9月18日
 *================================================================*/

#pragma once


#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
#include <cstring>
#include <algorithm>
#include <string>
#include <iostream>
#include <cassert>
#include <errno.h>
#include <assert.h>
#include <stdint.h>
#include <csignal>

#include <byteswap.h>

#define FISH_NAMESPACE_START namespace fish {
#define FISH_NAMESPACE_END }

// 字节序
#define FISH_ENDIAN_LITTLE 1
#define FISH_ENDIAN_BIG 2

// 日志
FISH_NAMESPACE_START

void FormatDate(std::ostream& os);
int GetLogLevel();
void SetLogLevel(int level);
#define FISH_DEBUG 1
#define FISH_WARN  2
#define FISH_ERROR 3
#define FISH_FATAL 4
#define FISH_LOG_IF_LEVEL(level, msg)                                \
        if (level >= fish::GetLogLevel()){                            \
             std::cout  << __FILE__ << ":" << __LINE__ << "|";  \
             fish::FormatDate(std::cout); std::cout << "|";          \
             std::cout  << #level << "|"                        \
                        << msg << std::endl << std::flush;      \
        }
#define FISH_LOGDEBUG(msg)   FISH_LOG_IF_LEVEL(FISH_DEBUG, msg)
#define FISH_LOGWARN(msg)    FISH_LOG_IF_LEVEL(FISH_WARN, msg)
#define FISH_LOGERROR(msg)   FISH_LOG_IF_LEVEL(FISH_ERROR, msg)
#define FISH_LOGFATAL(msg)   FISH_LOG_IF_LEVEL(FISH_FATAL, msg)


/**
 * @brief 提供GCC预测信息(CPU流水线技术下提高性能)
 */
#if defined(__GNUC__) || defined(__llvm__)
#   define FISH_LICKLY(x)     __builtin_expect(!!(x), 1)
#   define FISH_UNLICKLY(x)   __builtin_expect(!!(x), 0)
#else
#   define FISH_LICKLY(x)     (x)
#   define FISH_UNLICKLY(x)   (x)
#endif


//================================= 断言 start =================================//
/**
 * @brief 断言，自动输出最多100层堆栈信息
 */
#define FISH_ASSERT(arg)           \
    if (FISH_UNLICKLY(!(arg))){ \
        FISH_LOGFATAL("ASSERTION:  "#arg" " << "backtrace: " << BackTraceString(100, 1, "\n        ")); \
        assert(0);                                            \
    }

/**
 * @brief 断言，自动输出最多100层堆栈信息
 */
#define FISH_ASSERT_MSG(arg, msg)                   \
    if (FISH_UNLICKLY(!(arg))){                                    \
        FISH_LOG_FATAL("ASSERTION:  "#arg" "\
                << "\nbacktrace: "                              \
                << BackTraceString(100, 1, "\n        ")); \
        assert(0);                                            \
    }
//================================= 断言 end =================================//

//=================================字节序 start=================================//
/**
 * @brief 字节序转换
 * @param[in]
 * @return
 */
template<typename T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type ByteSwap(T value) {
    return static_cast<T>(bswap_16((uint16_t)value));
}

template<typename T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type ByteSwap(T value) {
    return static_cast<T>(bswap_32((uint32_t)value));
}

template<typename T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type ByteSwap(T value) {
    return static_cast<T>(bswap_64((uint64_t)value));
}

#if BYTE_ORDER == LITTLE_ENDIAN
#define FISH_ENDIAN FISH_ENDIAN_LITTLE
#else
#define FISH_ENDIAN FISH_ENDIAN_BIG
#endif

#if FISH_ENDIAN == FISH_ENDIAN_LITTLE
template<typename T> T ByteSwapOnLittleEndian(T t) {
    return ByteSwap(t);
}
template<typename T> T ByteSwapOnBigEndian(T t) {
    return t;
}
#else
template<typename T> T ByteSwapOnLittleEndian(T t) {
    return t;
}
template<typename T> T ByteSwapOnBigEndian(T t) {
    return ByteSwap(t);
}
#endif
//=================================字节序 end=================================//

FISH_NAMESPACE_END
