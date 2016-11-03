#pragma once
#include <cctype>
#include <string.h>
#include <string>
#include <functional>
#include <memory>
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
		HTTP_PARSE_OK, HTTP_PARSE_INVALID_METHOD, HTTP_PARSE_INVALID_REQUEST, 
		HTTP_PARSE_INVALID_URL, HTTP_PARSE_INVALID_PROTO, HTTP_AGAIN, HTTP_URL_STATIC, HTTP_URL_DYNAMIC,
	};
	enum Method {
		HTTP_GET, HTTP_POST, HTTP_HEAD, HTTP_UNKNOW,
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

	struct httpRequest {
		int fd;
		char buf[MAX_BUF];
		size_t pos, last;
		Method method;
		ParseRequestLineState state;
		char *requestStart;
		char *requestEnd;
		char *methodStart;
		char *methodEnd;
		char *uriStart;
		char *uriEnd;
		int httpMajor;
		int httpMinor;
		explicit  httpRequest(int _fd)
			: fd(_fd)
			, pos(0)
			, last(0)
			, method(HTTP_UNKNOW)
			, state(sw_start)
			, requestStart(nullptr)
			, requestEnd(nullptr)
			, methodStart(nullptr)
			, methodEnd(nullptr)
			, uriStart(nullptr)
			, uriEnd(nullptr)
			, httpMajor(0)
			, httpMinor(0) {}
	};
	using handleHttpRequest = std::shared_ptr<httpRequest>;
	inline bool  str3Cmp(char *m, char c0, char c1, char c2, char c3) {
		return *reinterpret_cast<uint32_t*>(m) == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0);
	}
	inline bool str4Cmp(char *m, char c0, char c1, char c2, char c3) {
		return *reinterpret_cast<uint32_t*>(m) == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0);
	}
	inline bool isCRLF(char ch) {return ch == CR || ch == LF;}
	ParseState httpParseRequestLine(handleHttpRequest);
	
	void doRequest(handleHttpRequest hp);
//	void doError(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
	void doError(int fd, const string &cause, const string &errnum, const string &shortmsg, const string &longmsg);
	void serveStatic(int fd, string &filename, int filesize);
	
}
