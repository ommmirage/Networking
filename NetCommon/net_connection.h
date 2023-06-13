#pragma once
#include "net_common.h"
#include "net_tsque.h"
#include "net_message.h"

class connection : public std::enable_shared_from_this<connection>
{
public:
    // A connection is "owned" by either a server or a client, and its
    // behaviour is slightly different bewteen the two.
    enum class owner
    {
        server,
        client
    };

public:
    // Constructor: Specify Owner, connect to context, transfer the socket
    //				Provide reference to incoming message queue
    connection(owner parent, asio::io_context &asioContext, asio::ip::tcp::socket socket, tsqueue<owned_message> &qIn)
        : sock(std::move(socket)), m_asioContext(asioContext), qMessagesIn(qIn)
    {
        ownerType = parent;
    }

    void ConnectToClient(uint32_t uid = 0)
    {
        if (ownerType == owner::server)
        {
            if (sock.is_open())
            {
                id = uid;
            }
        }
    }

    bool ConnectToServer(const asio::ip::tcp::resolver::results_type &endpoints);

    bool Send(const message &msg);

    bool IsConnected()
    {
        return sock.is_open();
    }

    // Each connection has a unique socket to a remote
    asio::ip::tcp::socket sock;

    // This context is shared with the whole asio instance
    asio::io_context &m_asioContext;

    // This queue golds all messages to be sent to the remote side
    // of this connection
    tsqueue<message> qMessagesOut;

    // This queue holds all messages that have been received from
    // the remote side of this connection. Note it is a reference
    // as the "owner" of this connection is expected to provide a queue
    tsqueue<owned_message> &qMessagesIn;

    // The "owner" decides how some of the connection behaves
	owner ownerType = owner::server;

    uint32_t id = 0;
};