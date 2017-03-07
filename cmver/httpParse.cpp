#include "httpParser.h"
#include <sstream>
#include <map>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <iostream>
#include <algorithm>
const std::string servdir = "/home/ubuntu/webserver/servfiles";
using namespace  cm_http;
using std::string;
void headerConnection(handleHttpRequest hp,const string&);
void headerIgnore(handleHttpRequest hp, const string&data);
void headerIfModifySince(handleHttpRequest hp, const string& data);
const std::map<const string, const string> filetypes = {
	{".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/msword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css"},
    {"" ,"text/plain"}
	};
const std::map<const string, std::function<void(handleHttpRequest,const string&)>> headerHandles = { 
	{"connection",headerConnection},
	{"host",headerIgnore},
	{"if-modified-since",headerIfModifySince},
	{"",headerIgnore}
};
const string& getFileType(const string& type) {
	auto it = filetypes.find(type);
	if (it != filetypes.end())
		return it->second;
	else
		return filetypes.find("")->second;
}
Method parseMethod(const char *s) {
	if (str3Cmp(s, 'G', 'E', 'T', '\0'))
		return HTTP_GET;
	else if (str4Cmp(s, 'P', 'O', 'S', 'T'))
		return HTTP_POST;
	else if (str4Cmp(s, 'H', 'E', 'A', 'D'))
		return HTTP_HEAD;
	return HTTP_UNKNOW_METHOD;
}
void parseUri(const char*uri, std::string &filename, std::string &cgiargs) {
	assert(uri != NULL);
	const char *t = strchr(uri, '?');
	size_t len = strlen(uri);
	if (t)
		len = t - uri;
	filename = servdir;
	filename.append(uri,len);
	if (filename.back() == '/')
		filename.append("index.html");
	log_info("filename: %s", filename.c_str());
}
cm_http::ParseState cm_http::httpParseHeader(handleHttpRequest hp) {
	check(hp->pos != hp->last, "");
	ParseHeaderState state = hp->headerState;
	for (size_t t = hp->pos; t < hp->last; ++t) {
		char ch = hp->buf[t];
		char *p = &hp->buf[t];
		switch (state) {
		case sw_hstart:
			if (isCRLF(ch))
				break;
			hp->keyStart = p;
			state = sw_key;
			break;
		case sw_key:
			if (ch == ' ') {
				state = sw_spaces_before_colon;
				*p = '\0';
			}
			else if (ch == ':') {
				state = sw_spaces_after_colon;
				*p = '\0';
			 }
			break;
		case sw_spaces_before_colon:
			if (ch == ':')
				state = sw_spaces_after_colon;
			else if (ch != ' ')
				return HTTP_PARSE_INVALID_KEY;
			break;
		case sw_spaces_after_colon:
			if (ch != ' ') {
				hp->valueStart = p;
				state = sw_value;
			}
			break;
		case sw_value:
			if (ch == CR) {
				state = sw_cr;
				*p = '\0';
			}
			else if (ch == LF) {
				state = sw_crlf;
				*p = '\0';
			}
			break;
		case sw_cr:
			if (ch == LF) {
				state = sw_crlf;
				hp->headerstrs.push_back(std::make_pair(hp->keyStart, hp->valueStart));
				break;
			}
			return HTTP_PARSE_INVALID_VALUE;
		case sw_crlf:
			if (ch == CR)
				state = sw_crlfcr;
			else {
				state = sw_key;
				hp->keyStart = p;
			 }
			break;
		case sw_crlfcr:
			if (ch == LF) {
				hp->pos = t;
				hp->headerState = sw_hstart;
				return HTTP_PARSE_OK;
			}
			else
				return HTTP_PARSE_INVALD_HEADER;
			break;
		default:break;
		}
	}
	hp->pos = hp->last - 1;
	hp->headerState = state;
	return HTTP_AGAIN;
}
cm_http::ParseState cm_http::httpParseRequestLine(handleHttpRequest hp){
	check(hp->pos != hp->last, "");
	ParseRequestLineState state = hp->requestState;
	for (size_t t = hp->pos; t < hp->last; ++t) {
		char ch = hp->buf[t];
		char *p = &hp->buf[t];
		switch (state) {
		case sw_start:
			if (isupper(ch)) {
				hp->methodStart = p;
				state = sw_method;
				break;
			}
			else if (isCRLF(ch))
				break;
			else
				return HTTP_PARSE_INVALID_METHOD;
		case sw_method:
			if (ch == ' ') {
				*p = '\0';
				hp->method = parseMethod(hp->methodStart);
				state = sw_spaces_before_uri;
				break;
			}
			else if (isupper(ch))
				break;
			else
				return HTTP_PARSE_INVALID_METHOD;
		case sw_spaces_before_uri:
			if (ch == ' ')
				break;
			else if (ch == '/') {
				hp->uriStart = p;
				state = sw_after_slash_in_uri;
				break;
			}else
				return HTTP_PARSE_INVALID_URL;
		case sw_after_slash_in_uri:
			if (ch == ' ') {
				*p = '\0';
				state = sw_http;
				break;
			}else 
				break;
		case sw_http:
			if (ch == 'H') {
				state = sw_http_H;
				break;
			}else if (ch == ' ')
				break;
			else
				return HTTP_PARSE_INVALID_URL;
		case sw_http_H:
			if (ch == 'T') {
				state = sw_http_HT;
				break;
			}else
				return HTTP_PARSE_INVALID_PROTO;
		case sw_http_HT:
			if (ch == 'T') {
				state = sw_http_HTT;
				break;
			}else
				return HTTP_PARSE_INVALID_PROTO;
		case sw_http_HTT:
			if (ch == 'P') {
				state = sw_http_HTTP;
				break;
			}else
				return HTTP_PARSE_INVALID_PROTO;
		case sw_http_HTTP:
			if (ch == '/') {
				state = sw_first_major_digit;
				break;
			}else
				return HTTP_PARSE_INVALID_PROTO;
		case sw_first_major_digit:
			if (isdigit(ch)) {
				hp->httpMajor = ch - '0';
				state = sw_major_digit;
				break;
			}else
				return HTTP_PARSE_INVALID_PROTO;
		case sw_major_digit:
			if (isdigit(ch)) {
				hp->httpMajor = hp->httpMajor * 10 + ch - '0';
				break;
			}else if (ch == '.') {
				state = sw_first_minor_digit;
				break;
			}else
				return HTTP_PARSE_INVALID_PROTO;
		case sw_first_minor_digit:
			if (isdigit(ch)) {
				hp->httpMinor = ch - '0';
				state = sw_minor_digit;
				break;
			}else
				return HTTP_PARSE_INVALID_PROTO;
		case sw_minor_digit:
			if (isdigit(ch)) {
				hp->httpMinor = hp->httpMinor * 10 + ch - '0';
				break;
			}
			else if (ch == '.') {
				state = sw_spaces_after_digit;
				break;
			}
			else if (ch == CR) {
				state = sw_almost_done;
				break;
			}else if (ch == LF) {
				state = sw_done;
			}else
				return HTTP_PARSE_INVALID_PROTO;
		case sw_spaces_after_digit:
			switch (ch) {
			case ' ':
					break;
			case CR:
				state = sw_almost_done;break;
			case LF:
				state = sw_done;break;
			default:
				return HTTP_PARSE_INVALID_PROTO;
			}
			break;
		case sw_almost_done:
			if (ch == LF) {
				state = sw_done;
				break;
			}else
				return HTTP_PARSE_INVALID_PROTO;
		case sw_done:
			hp->pos = t;
			hp->requestState = sw_start;
			return HTTP_PARSE_OK;
		default :
			break;
		}
	}
	hp->pos = hp->last - 1;
	return HTTP_AGAIN;
}

void cm_http::doRequest(handleHttpRequest hp) {
	size_t n = sockets::read(hp->fd, hp->buf, MAX_BUF);
	log_info("%d read %ld bytes\n", hp->fd, n);
	if (n == 0) {
		sockets::close(hp->fd);
		return;
	}
	hp->last += n;
	std::cout << hp->buf << std::endl;
	auto ps = httpParseRequestLine(hp);
	check(ps == HTTP_PARSE_OK, "request line parse result %d", ps);
	log_info("method: %d", hp->method);
	
	if (hp->method != HTTP_GET) {
		doError(hp->fd, "not implement", "200", "cmver", "can't do this");
		sockets::close(hp->fd);
		return;
	 }
	
	ps = httpParseHeader(hp);
	check(ps == HTTP_PARSE_OK, "header parse result %d", ps);
	doHeaderProcess(hp);
	
	string filename, cigargs;
	parseUri(hp->uriStart, filename, cigargs);
	struct stat sbuf;
	if (stat(filename.c_str(), &sbuf)) {
		doError(hp->fd, filename, "404", "Not Found", "cmver can't find the file");
		sockets::close(hp->fd);
		return;
	}
	if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)){
            doError(hp->fd, filename, "403", "Forbidden", "cmver can't read the file");
			sockets::close(hp->fd);
			return;
    }
	serveStatic(hp->fd, filename, sbuf.st_size);
	sockets::close(hp->fd);
}
	
void cm_http::serveStatic(int fd, string &filename, int filesize) {
	std::stringstream out;
	const string& type = getFileType(filename.substr(filename.find_last_of('.')));
	out << "HTTP/1.0 200 OK\r\n";
	out << "Server:cmver\r\n";
	out << "Content-length: " << filesize << "\r\n";
	out << "Content-type: " << type << "\r\n\r\n";
	string req = out.str();
	sockets::rio_writen(fd, req.c_str(),req.size());
	
	int srcfd = open(filename.c_str(), O_RDONLY, 0);
	char* scrp = (char*)mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
	close(srcfd);
	ssize_t n = sockets::rio_writen(fd, scrp, filesize);
	check(n == filesize, "writen %ld bytes", n);
	munmap(scrp, filesize);
}
void cm_http::doError(int fd, const string &cause, const string &errnum, const string &shortmsg, const string &longmsg) {
	string header, body;
	std::stringstream out;
	out << "<html><title>cmver Error</title>" << "<body bgcolor=""ffffff"">\n";
	out << errnum << ": " << shortmsg << "\n<p>" << longmsg << ": " << cause << "\n</p>";
	out << "<hr><em>Cmver web server</em>\n</body></html>";
	body = out.str();
	
	out.str("");
	out << "HTTP/1.1 " << errnum << " " << shortmsg << "\r\n";
	out << "Server: Cmver\r\n" << "Content-type: text/html\r\n";
	out << "Connection: close\r\n";
	out << "Content-length: " << body.size() << "\r\n\r\n";
	header = out.str();

	sockets::rio_writen(fd, header.c_str(), header.size());
	sockets::rio_writen(fd, body.c_str(), body.size());
}

void cm_http::doHeaderProcess(handleHttpRequest hp) {
	auto it = hp->headerstrs.begin();
	while (it != hp->headerstrs.end()) {
		string key(it->first);
		std::transform(key.begin(),key.end(),key.begin(),::tolower);
		auto r = headerHandles.find(key);
		if (r != headerHandles.end()) {
			r->second(hp, it->second);
		}
		else {
			headerHandles.find("")->second(hp, it->second);
		 }
		it = hp->headerstrs.erase(it);
	}
}

void headerConnection(handleHttpRequest hp, const string &data) {
	if (strcasecmp("Keep-Alive", data.c_str())==0) {
		hp->headers.keepAlive = true;
		log_info("keep-alive");
	}
}

void headerIgnore(handleHttpRequest hp, const string&data) {
	//log_warn("header value %s ignored", data.c_str());
	return;
}

void headerIfModifySince(handleHttpRequest hp, const string& data) {
	
}