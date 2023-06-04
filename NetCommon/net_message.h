#pragma once
#include "net_common.h"

enum class MsgType : uint32_t
{
    MovePlayer
};

struct message_header
{
    MsgType type;
    uint32_t size = 0;
};

struct message
{
    message_header header;
    std::vector<uint8_t> body;
};