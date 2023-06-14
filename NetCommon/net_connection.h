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

    // ASYNC - Prime context ready to read a message header
    void ReadHeader()
    {
        asio::async_read(sock, asio::buffer(&msgTemporaryIn.header, sizeof(message_header)),
            [this](std::error_code ec, std::size_t lenght)
            {
                if (!ec)
                {
                    if (msgTemporaryIn.header.size > 0)
                    {
                        msgTemporaryIn.body.resize(msgTemporaryIn.header.size);
                        ReadBody();
                    }
                    else
                    {
                        // If the msg has no body
                        AddToIncomingMessageQueue();
                    }
                }
                else
                {
                    std::cout << "[" << id << "] Read Header fail.\n";
                    sock.close();
                }
            }
        );
    }

    // ASYNC - Prime context ready to read a message body
    void ReadBody()
    {
        // data() returns a pointer to the array in vector
        asio::async_read(sock, asio::buffer(msgTemporaryIn.body.data(), msgTemporaryIn.body.size()),
            [this](std::error_code ec, std::size_t lenght)
            {
                if (!ec)
                {
                    AddToIncomingMessageQueue();
                }
                else
                {
                    std::cout << "[" << id << "] Read body fail.\n";
                    sock.close();
                }
            }
        );
    }

    void WriteHeader()
    {
        asio::async_write(sock, asio::buffer(&qMessagesOut.front().header, sizeof(message_header)),
            [this](std::error_code ec, std::size_t length)
            {
                if (!ec)
                {
                    if (qMessagesOut.front().body.size() > 0)
                    {
                        WriteBody();
                    }
                    else
                    {
                        qMessagesOut.pop_front();

                        if (!qMessagesOut.empty())
                        {
                            WriteHeader();
                        }
                    }
                }
                else
                {
                    std::cout << "[" << id << "] Write Header fail.\n";
                    sock.close();
                }
            }
        );
    }

    void WriteBody()
    {
        asio::async_write(sock,
            asio::buffer(qMessagesOut.front().body.data(), qMessagesOut.front().body.size()),
            [this](std::error_code ec, std::size_t length)
            {
                if (!ec)
                {
                    qMessagesOut.pop_front();

                    if (!qMessagesOut.empty())
                    {
                        WriteHeader();
                    }
                }
                else
                {
                    std::cout << "[" << id << "] Write body fail.\n";
                    sock.close();
                }
            }
        );
    }

    void AddToIncomingMessageQueue()
    {
        if (ownerType == owner::server)
        {
            // { initialization of owned_message struct }
            qMessagesIn.push_back({ this->shared_from_this(), msgTemporaryIn });
        }
        else
        {
            // if ownerType is client then tagging the connection dosen't have
            // any sense, because clients have only one connection
            qMessagesIn.push_back({ nullptr, msgTemporaryIn });
        }

        // AddToIncomingMessageQueue всегда вызывается, когда мы прочти сообщение.
        // Поэтому мы можем prime asio context with another task to perform
        ReadHeader();
    }

    bool IsConnected()
    {
        return sock.is_open();
    }

    // Each connection has a unique socket to a remote
    asio::ip::tcp::socket sock;

    // This context is shared with the whole asio instance
    asio::io_context &m_asioContext;

    // This queue holds all messages to be sent to the remote side
    // of this connection
    tsqueue<message> qMessagesOut;

    // This queue holds all messages that have been received from
    // the remote side of this connection. Note it is a reference
    // as the "owner" of this connection is expected to provide a queue
    tsqueue<owned_message> &qMessagesIn;

    message msgTemporaryIn;

    // The "owner" decides how some of the connection behaves
	owner ownerType = owner::server;

    uint32_t id = 0;
};