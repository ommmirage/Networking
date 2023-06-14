#pragma once
#include "net_common.h"

enum class MsgType : uint32_t
{
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage
};

struct message_header
{
    MsgType type{};
    uint32_t size = 0;
};

struct message
{
    message_header header{};
    std::vector<uint8_t> body;
};

class connection;

// чтобы сервер понимал от кого сообщение
struct owned_message
{
    std::shared_ptr<connection> remote = nullptr;
    message msg;
};