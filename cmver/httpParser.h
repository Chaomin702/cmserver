#pragma once
#include <cctype>
#include <string.h>
#include <string>
#include <functional>
#include <memory>
#include <list>
#include <time.h>
#include "dbg.h"
#include "socketOps.h"
namespace cm_http {
	
	const int MAX_BUF = 8124;
	const char CR = '\r';
	const char LF = '\n';
	using std::isupper;
	using std::isalpha;
	using std::isdigit;
	using std::string;
	enum ParseState {
		HTTP_PARSE_OK = 0,
		HTTP_PARSE_INVALID_METHOD, 
		HTTP_PARSE_INVALID_REQUEST, 
		HTTP_PARSE_INVALID_URL,
		HTTP_PARSE_INVALID_PROTO, 
		HTTP_AGAIN, 
		HTTP_URL_STATIC, 
		HTTP_URL_DYNAMIC,
		HTTP_PARSE_INVALID_KEY, 
		HTTP_PARSE_INVALID_VALUE, 
		HTTP_PARSE_INVALD_HEADER,
	};
	enum Method {
		HTTP_GET, HTTP_POST, HTTP_HEAD, HTTP_UNKNOW_METHOD,
	};
	enum Header { 
		HTTP_HOST, HTTP_CONNECTION, HTTP_IF_MODIFIED_SINCE, HTTP_UNKNOW_HEADER,
	};
	enum ParseRequestLineState{
		sw_start              = 0,
		sw_method,
		sw_spaces_before_uri,
		sw_after_slash_in_uri,
		sw_http,
		sw_http_H,
		sw_http_HT,
		sw_http_HTT,
		sw_http_HTTP,
		sw_first_major_digit,
		sw_major_digit,
		sw_first_minor_digit,
		sw_minor_digit,
		sw_spaces_after_digit,
		sw_almost_done,
		sw_done,
	};
	enum ParseHeaderState { 
		sw_hstart = 0,
        sw_key,
        sw_spaces_before_colon,
        sw_spaces_after_colon,
        sw_value,
		sw_spaces_after_value,
        sw_cr,
        sw_crlf,
        sw_crlfcr,
		sw_hdone,
	};
	struct httpHeader {
		bool keepAlive;
		bool modified;
		time_t mtime;
		httpHeader():keepAlive(false), modified(false), mtime(0) {}
	};
	using headerStr = std::pair<const char*, const char*>;
	struct httpRequest {
		const int fd;
		char buf[MAX_BUF];
		size_t pos, last;
		Method method;
		ParseRequestLineState requestState;
		ParseHeaderState headerState;
		char *requestStart;
		char *methodStart;
		char *uriStart;
		char *keyStart;
		char *valueStart;
		int httpMajor;
		int httpMinor;
		std::list<headerStr> headerstrs;
		httpHeader headers;
		explicit  httpRequest(int _fd)
			: fd(_fd)
			, pos(0)
			, last(0)
			, method(HTTP_UNKNOW_METHOD)
			, requestState(sw_start)
			, headerState(sw_hstart)
			, requestStart(nullptr)
			, methodStart(nullptr)
			, uriStart(nullptr)
			, keyStart(nullptr)
			, valueStart(nullptr)
			, httpMajor(0)
			, httpMinor(0) {}
	};
	
	using handleHttpRequest = std::shared_ptr<httpRequest>;
	inline bool  str3Cmp(const char *m, char c0, char c1, char c2, char c3) {
		return *reinterpret_cast<uint32_t*>(const_cast<char*>(m)) == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0);
	}
	inline bool str4Cmp(const char *m, char c0, char c1, char c2, char c3) {
		return *reinterpret_cast<uint32_t*>(const_cast<char*>(m)) == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0);
	}
	inline bool isCRLF(char ch) {return ch == CR || ch == LF;}
	ParseState httpParseRequestLine(handleHttpRequest);
	ParseState httpParseHeader(handleHttpRequest);
	void doRequest(handleHttpRequest hp);
	void doError(int fd, const string &cause, const string &errnum, const string &shortmsg, const string &longmsg);
	void serveStatic(int fd, string &filename, int filesize);
	void doHeaderProcess(handleHttpRequest hp);
}
