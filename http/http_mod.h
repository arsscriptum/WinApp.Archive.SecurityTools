#pragma once
// HTTP V1.1 Implementation
// POST/GET requests/responses
//--------------------------
// According to RFC 2616
// Request: Method SP Request-URI SP HTTP-Version CRLF
// Response: HTTP-Version SP Status-Code SP Reason-Phrase CRLF

#include <iostream>
#include <string>

using namespace std;

static const int HTTP_DEFAULT_PORT = 80;
static const char* HTTP_FULL_HEADER = "HTTP/1.1 ";

static const char HTTP_SP = ' ';

static const int HTTP_STATUS_CODE_LENGTH = 3;

static const char HTTP_CR = '\r';
static const char HTTP_LF = '\n';
static const char* HTTP_CRLF = "\r\n";
static const char* HTTP_CRLFCRLF = "\r\n\r\n";

static const char* HTTP_GET_REQUEST = "GET";
static const char* HTTP_POST_REQUEST = "POST";

static const char* HTTP_CONTENT_TYPE = "Content-Type: ";
static const char* HTTP_CONTENT_TYPE_DEF = "application/x-www-form-urlencoded";
static const char* HTTP_CONTENT_LENGTH = "Content-Length: ";
static const char* HTTP_TRANSFER_ENCODING_CHUNKED = "Transfer-Encoding: chunked\r\n";
static const char* HTTP_CONNECTION_CLOSED = "Connection: closed\r\n";

static const char* HTTP_HOST = "Host: ";
static const char* HTTP_ACCEPT = "Accept: */*\r\n";
static const char* HTTP_ACCEPT_LANG = "Accept-Language: en-us\r\n";

static const long HTTP_RESPONSE_INFORMATIONAL_CONTINUE = 100;
static const long HTTP_RESPONSE_SUCCESSFUL_OK = 200;

enum dataMethodT {C_NONE, C_LENGTH, C_CHUNKED};
enum RequestMethodT {REQUEST_GET, REQUEST_POST};

int SendHTTPRequest(const char* address, const char* URI,
																				RequestMethodT requestMethod, int paramsNo, char* paramsNames[], char* values[],
																				const char* hostName = "", int defaultPort = HTTP_DEFAULT_PORT);