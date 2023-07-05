#pragma once
#include "rapidjson/document.h"

enum RequestVerb
{
    Get,
    Post,
    UnsupportedVerb
};

// TODO: Add a reset and a next state path. Maybe also a freeze time path? idk
// TODO: Implement pausing (probably in Game object)
enum RequestPath
{
    Root,
    Join,
    State,
    Lobby,
    Start,
    Draw,
    Write,
    Shirt,
    Battle,
    Image,
    UnsupportedPath,
};

enum RequestBodyType
{
    Json,
    Jpeg,
    UnsupportedBody
};

struct Request
{
    RequestVerb verb;
    RequestPath path;
    char* static_file_path;
    RequestBodyType body_type;
    rapidjson::Document json_body;
    char* raw_body_ptr;
    size_t raw_body_len;
    uint32_t user_id;

    Request(char* request_string, size_t request_len);
};