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
    UnsupportedPath,
};

struct Request
{
    RequestVerb verb;
    RequestPath path;
    rapidjson::Document body;

    Request(char* request_string, size_t request_len);
};