#pragma once
#include "net_common.h"
#include "net_tsque.h"
#include "net_message.h"

class server_interface;

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
    connection(owner parent, asio::io_context &m_asioContext, asio::ip::tcp::socket socket, tsqueue<owned_message> &qIn)
        : sock(std::move(socket)), asioContext(m_asioContext), qMessagesIn(qIn)
    {
        ownerType = parent;

        // Construct validation check data
        if (ownerType == owner::server)
        {
            // Construct random data for the client to transform and send back
            handshakeOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());
        }
    }

    void ConnectToClient(server_interface* server, uint32_t uid = 0)
    {
        if (ownerType == owner::server)
        {
            if (sock.is_open())
            {
                id = uid;

                WriteValidation();

                // Issue a task to wait asynchronously for right validation data
                // sent back from the client
                ReadValidation(server);
            }
        }
    }

    void Send(const message &msg)
    {
        asio::post(asioContext,
            [this, msg]()
            {
                bool bWritingMessage = !qMessagesOut.empty();
                qMessagesOut.push_back(msg);
                if (!bWritingMessage)
                {
                    WriteHeader();
                }
            }
        );
    }

    // ASYNC - Prime context ready to read a message header
    void ReadHeader()
    {
        asio::async_read(sock, asio::buffer(&msgTemporaryIn.header, sizeof(message_header)),
            [this](std::error_code ec, std::size_t length)
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

        // AddToIncomingMessageQueue всегда вызывается, когда мы прочли сообщение.
        // Поэтому мы можем prime asio context with another task to perform
        ReadHeader();
    }

    uint64_t encrypt(uint64_t input)
    {
        return input * 2;
    }

    void WriteValidation()
    {
        asio::async_write(sock, asio::buffer(&handshakeOut, sizeof(handshakeOut)),
            [this](std::error_code ec, std::size_t length)
            {
                if (!ec)
                {
                    if (ownerType == owner::client)
                    {
                        ReadHeader();
                    }
                }
                else
                {
                    sock.close();
                }
            }
        );
    }

    void ReadValidation(server_interface* server = nullptr)
    {
        asio::async_read(sock, asio::buffer(&handshakeIn, sizeof(handshakeIn)),
            [this, server](std::error_code ec, std::size_t length)
            {
                if (!ec)
                {
                    if (ownerType == owner::client)
                    {
                        handshakeOut = encrypt(handshakeIn);
                        WriteValidation();
                    }
                    else
                    {
                        if (handshakeIn == encrypt(handshakeOut))
                        {
                            std::cout << "Client Validated\n";
                            // server->OnClientValidated(this->shared_from_this());

                            ReadHeader();
                        }
                        else
                        {
                            std::cout << "Validation failed. Client disconnected\n";
                            sock.close();
                        }
                    }
                }
                else
                {
                    std::cout << "ReadValidation() failed\n";
                    sock.close();
                }
            }
        );
    }

    void ConnectToServer(const asio::ip::tcp::resolver::results_type &endpoints)
    {
        // Only clients can connect to servers
        if (ownerType == owner::client)
        {
            // Request asio attempts to connect to an endpoint
            asio::async_connect(sock, endpoints,
                [this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
                {
                    if (!ec)
                    {
                        ReadValidation();
                    }
                }
            );
        }
    }

    void Disconnect()
    {
        if (IsConnected())
        {
            asio::post(asioContext, [this]() { sock.close(); });
        }
    }

    bool IsConnected()
    {
        return sock.is_open();
    }

    // Each connection has a unique socket to a remote
    asio::ip::tcp::socket sock;

    // This context is shared with the whole asio instance
    asio::io_context &asioContext;

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

    // Handshake validation
    uint64_t handshakeOut = 0;
    uint64_t handshakeIn = 0;
};