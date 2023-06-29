#pragma once
#include "cJSON.h"

enum RequestVerb
{
    Get,
    Post,
    UnsupportedVerb
};

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
    UnsupportedPath,
};

struct Request
{
    RequestVerb verb;
    RequestPath path;
    cJSON* body;

    Request(char* request_string, size_t request_len);
    ~Request();
};