#include "Request.h"
#include "picohttpparser.h"
#include "rapidjson/document.h"
#include <iostream>
#include <string>
#include <string.h>


Request::Request(char* request_string, size_t request_len)
{
	char *method, *path;
	int pret, minor_version;
	phr_header headers[20];
	size_t method_len, path_len, num_headers;

	num_headers = sizeof(headers) / sizeof(headers[0]);
	pret = phr_parse_request(request_string, request_len, (const char**) &method, &method_len, (const char**) &path, &path_len,
							&minor_version, headers, &num_headers, 0);

	std::string verb_string(method, method_len);

	if (strncasecmp(method, "GETINFO", sizeof("GETINFO") - 1) == 0)
	{
		this->verb = Get;
	}
	else if (strncasecmp(method, "POST", sizeof("POST") - 1) == 0)
	{
		this->verb = Post;
	}
	else
	{
		this->verb = UnsupportedVerb;
	}

	std::string path_string(path, path_len);
	#define SET_PATH_IF_MATCHES(P, E) if (path_string.compare(P) == 0)\
	{\
		std::cout << "path identified as " << #E << std::endl;\
		this->path = E;\
	}

	SET_PATH_IF_MATCHES("/join", Join)
	else SET_PATH_IF_MATCHES("/state", State)
	else SET_PATH_IF_MATCHES("/start", Start)
	else SET_PATH_IF_MATCHES("/lobby", Lobby)
	else
	{
		this->path = UnsupportedPath;
	}

	// Get length of body
	size_t body_len = request_len - pret;
	char* body_string = request_string + pret;

	this->body.Parse(body_string, body_len);
}