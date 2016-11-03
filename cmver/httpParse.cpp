#include "httpParser.h"
#include <sstream>
#include <map>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
const std::string servdir = "/home/ubuntu/webserver/servfiles";
using namespace  cm_http;
using std::string;
const std::map<string, string> filetypes = {
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
const string& getFileType(const string& type) {
	auto it = filetypes.find(type);
	if (it != filetypes.end())
		return it->second;
	else
		return filetypes.find("")->second;
}
Method parseMethod(char *s, char *t) {
	assert(t > s);
	switch (t - s) {
	case 3:
		if (str3Cmp(s, 'G', 'E', 'T', ' '))
			return HTTP_GET;
	case 4:
		if (str4Cmp(s, 'P', 'O', 'S', 'T'))
			return HTTP_POST;
		if (str4Cmp(s, 'H', 'E', 'A', 'D'))
			return HTTP_HEAD;
		return HTTP_UNKNOW;
	default :
		return HTTP_UNKNOW;
	}
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
cm_http::ParseState cm_http::httpParseRequestLine(handleHttpRequest hp){
	check(hp->pos != hp->last, "");
	ParseRequestLineState state = hp->state;
	check(state == sw_start, "state should be sw_start");
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
				hp->methodEnd = p;
				hp->method = parseMethod(hp->methodStart, hp->methodEnd);
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
				hp->uriEnd = p;
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
			return HTTP_PARSE_OK;
		default :
			break;
		}
	}
	return HTTP_AGAIN;
}

void cm_http::doRequest(handleHttpRequest hp) {
	size_t n = sockets::read(hp->fd, hp->buf, MAX_BUF);
	log_info("%d read %d bytes\n", hp->fd, n);
	if (n == 0) {
		log_info("close fd %d",hp->fd);
		sockets::close(hp->fd);
		return;
	}
	hp->last += n;
	auto ps = httpParseRequestLine(hp);
	check(ps == HTTP_PARSE_OK, "request line parse result %d", ps);
	log_info("method: %d", hp->method);
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
	sockets::rio_writen(fd, scrp, filesize);
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
//void cm_http::doError(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg){
//    char header[MAX_BUF], body[MAX_BUF];
//
//    sprintf(body, "<html><title>cmver Error</title>");
//    sprintf(body, "%s<body bgcolor=""ffffff"">\n", body);
//    sprintf(body, "%s%s: %s\n", body, errnum, shortmsg);
//    sprintf(body, "%s<p>%s: %s\n</p>", body, longmsg, cause);
//    sprintf(body, "%s<hr><em>Cmver web server</em>\n</body></html>", body);
//
//    sprintf(header, "HTTP/1.1 %s %s\r\n", errnum, shortmsg);
//    sprintf(header, "%sServer: Cmver\r\n", header);
//    sprintf(header, "%sContent-type: text/html\r\n", header);
//    sprintf(header, "%sConnection: close\r\n", header);
//    sprintf(header, "%sContent-length: %d\r\n\r\n", header, (int)strlen(body));
//	sockets::rio_writen(fd, header, strlen(header));
//    sockets::rio_writen(fd, body, strlen(body));
//    return;
//}