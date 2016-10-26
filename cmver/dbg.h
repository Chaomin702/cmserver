#ifndef __dbg_h__
#define __dbg_h__

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
//#ifdef NDEBUG
//#define debug(M, ...)
//#else
//#define debug(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
//#endif
//
//#define clean_errno() (errno == 0 ? "None" : strerror(errno))
//
//#define log_err(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
//
//#define log_warn(M, ...) fprintf(stderr, "[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
//
//#define log_info(M, ...) fprintf(stderr, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
//
//#define check(A, M, ...) if(!(A)) { log_err(M, ##__VA_ARGS__); exit(1); }
//
//#define check_debug(A, M, ...) if(!(A)) { debug(M, ##__VA_ARGS__); exit(1); }

template<typename T>
std::ostream& print(std::ostream &os, const T &t) {
	return os << t <<"\n";
}

template<typename T, typename... Args>
std::ostream& print(std::ostream &os, const T &t, const Args &... rest) {
	os << t << ", ";
	return print(os, rest...);
}

template <typename T, typename... Args>
void errorMsg(const T &t, const Args &... rest) {
	std::cout << "[ERROR] " << __FILE__ << " " << __LINE__ << " ";
	print(std::cout, t, rest...);
}
#endif
#pragma once
