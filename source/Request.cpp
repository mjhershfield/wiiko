#include "Request.h"
#include "picohttpparser.h"
#include "rapidjson/document.h"
#include <iostream>
#include <string.h>

#define str_cmp_const(ptr, len, constant) !strncasecmp(ptr, constant, sizeof(constant) - 1)
#define str_cmp_const_check_size(ptr, len, constant) len == sizeof(constant)-1 && str_cmp_const(ptr, len, constant)


// TODO: see if cpp-httplib or mongoose have better replacements for request parsing and 
// response creation.
Request::Request(char* request_string, size_t request_len)
{
	char *method, *full_path, *queries;
	int pret, minor_version;
	phr_header headers[20];
	size_t method_len, full_path_len, path_len, static_path_len, query_len, num_headers;

	num_headers = sizeof(headers) / sizeof(headers[0]);
	pret = phr_parse_request(request_string, request_len, (const char**) &method, &method_len, (const char**) &full_path, &full_path_len,
							&minor_version, headers, &num_headers, 0);

	#define CHECK_VERB(S, E) if (str_cmp_const(method, method_len, S))\
	{\
		this->verb = E;\
	}

	CHECK_VERB("GET", Get)
	else CHECK_VERB("POST", Post)
	else
	{
		this->verb = UnsupportedVerb;
	}

	// Null terminate path
	full_path[full_path_len] = 0;
	queries = strchr(full_path, '?');
	// Use null terminator separator between path and queries, move query pointer forward
	if (queries != 0)
	{
		path_len = queries - full_path;
		*queries++ = 0;
		query_len = full_path_len - path_len - 1;
	}
	else
	{
		path_len = full_path_len;
		query_len = 0;
	}

	char* start_of_static_path = strchr(full_path + 1, '/');
	if (start_of_static_path == NULL || start_of_static_path > full_path + full_path_len)
	{
		static_path_len = 0;
	}
	else
	{
		*start_of_static_path = 0;
		this->static_file_path = start_of_static_path + 1;
		static_path_len = path_len - (start_of_static_path - full_path) - 1;
		path_len = start_of_static_path - full_path;
	}

	if (static_path_len == 0)
	{
		this->static_file_path = NULL;
	}
	
	printf("Path length: %d\n", path_len);
	printf("Path: %s\n", full_path);
	printf("Static file path length: %d\n", static_path_len);
	printf("Static file path: %s\n", static_file_path);
	printf("Query length: %d\n", query_len);
	printf("Queries: %s\n", queries);

	#define SET_PATH_IF_MATCHES(S, E) if (str_cmp_const_check_size(full_path, path_len, S))\
	{\
		std::cout << "path identified as " << #E << std::endl;\
		this->path = E;\
	}

	if (str_cmp_const_check_size(full_path, path_len, "/"))
	{
		std::cout << "path identified as Root" << std::endl;
		this->path = Root;
	}
	else SET_PATH_IF_MATCHES("/join", Join)
	else SET_PATH_IF_MATCHES("/state", State)
	else SET_PATH_IF_MATCHES("/start", Start)
	else SET_PATH_IF_MATCHES("/lobby", Lobby)
	else SET_PATH_IF_MATCHES("/draw", Draw)
	else SET_PATH_IF_MATCHES("/write", Write)
	else SET_PATH_IF_MATCHES("/image", Image)
	else
	{
		std::cout << "Unsupported Path" << std::endl;
		this->path = UnsupportedPath;
	}

	// Parse query for IDs
	if (query_len > 3 && queries[0] == 'i' && queries[1] == 'd' && queries[2] == '=')
	{
		queries[query_len] = 0;
		this->user_id = atoi(queries + 3);
		std::cout << "User id is " << this->user_id << std::endl;
	}
	else
	{
		this->user_id = 0;
		std::cout << "User is unauthenticated" << std::endl;
	}


	this->raw_body_ptr = request_string + pret;
	this->raw_body_len = request_len - pret;

	std::cout << "Body length: " << raw_body_len << std::endl;
	// std::cout << "BODY:" << std::string(raw_body_ptr, raw_body_len);
	// std::cout << "END BODY" << std::endl;

	// Parse body type
	for (uint32_t i = 0; i < num_headers; i++)
	{
		if (str_cmp_const_check_size(headers[i].name, headers[i].name_len, "Content-Type"))
		{
			if (strnstr(headers[i].value, "application/json", headers[i].value_len))
			{
				std::cout << "Body content JSON" << std::endl;
				this->body_type = Json;
				this->json_body.ParseInsitu(this->raw_body_ptr);
			}
			else if (strnstr(headers[i].value, "image/jpeg", headers[i].value_len))
			{
				std::cout << "Body content JPEG" << std::endl;
				this->body_type = Jpeg;
			}
			else
			{
				std::cout << "Body type unknown";
				this->body_type = UnsupportedBody;
			}
		}
	}

	if (raw_body_len == 0)
	{
		this->body_type = UnsupportedBody;
	}
}