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

    size_t size()
    {
        return body.size();
    }

    // Pushes any Plain Old Data (POD) data into the message buffer
    template <typename T>
    void add(message& msg, const T &data)
    {
        // Check that the type of the data being pushed is trivially copyable
        static_assert(std::is_standard_layout<T>::value, "Data is too complex to be pushed into vector");

        // Cache current size of vector, as this will be the point we insert the data
        size_t i = msg.body.size();

        // Resize the vector by the size of the data being pushed
        msg.body.resize(msg.body.size() + sizeof(T));

        // Physically copy the data into the newly allocated vector space
        std::memcpy(msg.body.data() + i, &data, sizeof(T));

        // Recalculate the message size
        msg.header.size = msg.size();
    }

    // Pulls any POD-like data form the message buffer
    template <typename T>
    T get(message& msg)
    {
        T data;
        // Check that the type of the data being pushed is trivially copyable
        static_assert(std::is_standard_layout<T>::value, "Data is too complex to be pulled from vector");

        // Cache the location towards the end of the vector where the pulled data starts
        size_t i = msg.body.size() - sizeof(T);

        // Physically copy the data from the vector into the user variable
        std::memcpy(&data, msg.body.data() + i, sizeof(T));

        // Shrink the vector to remove read bytes, and reset end position
        msg.body.resize(i);

        // Recalculate the message size
        msg.header.size = msg.size();

        return data;
    }
};

class connection;

// чтобы сервер понимал от кого сообщение
struct owned_message
{
    std::shared_ptr<connection> remote = nullptr;
    message msg;
};