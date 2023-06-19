#pragma once

#include "net_common.h"
#include "net_tsque.h"
#include "net_message.h"
#include "net_connection.h"

class server_interface
{
public:
    server_interface(uint16_t port)
        : asioAcceptor(asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
    {

    }

    ~server_interface()
    {
        Stop();
    }

    bool Start()
    {
        try
        {
            WaitForClientConnection();

            threadContext = std::thread([this]() { asioContext.run(); });
        }
        catch(const std::exception& e)
        {
            // Что-то мешает серверу слушать
            std::cerr << "[SERVER] Exception: " << e.what() << '\n';
            return false;
        }

        std::cout << "[SERVER] Started!\n";
        return true;
    }

    void Stop()
    {
        asioContext.stop();

        if (threadContext.joinable())
        {
            threadContext.join();
        }

        std::cout << "[SERVER] Stopped!\n";
    }

    // Instruct ASIO to wait for connection
    void WaitForClientConnection()
    {
        asioAcceptor.async_accept(
            [this](std::error_code ec, asio::ip::tcp::socket socket)
            {
                if (!ec)
                {
                    std::cout << "[SERVER] New connection: " << socket.remote_endpoint() << "\n";

                    std::shared_ptr<connection> newConnection =
                        std::make_shared<connection>(connection::owner::server,
                            asioContext, std::move(socket), qMessagesIn);

                    if (OnClientConnect(newConnection))
                    {
                        // Connection allowed, so add to container of new connections
                        deqConnections.push_back(newConnection);

                        deqConnections.back()->ConnectToClient(nIDCounter++);

                        std::cout << "[" << deqConnections.back()->id << "] connection approved\n";
                    }
                    else
                    {
                        std::cout << "[------] Conncetion denied\n";
                    }
                }
                else
                {
                    // Error has occured during acceptance
                    std::cout << "[SERVER] New connection error: " << ec.message() << "\n";
                }

                // Prime the asio context with more work
                WaitForClientConnection();
            }
        );
    }

    void MessageClient(std::shared_ptr<connection> client, message& msg)
    {
        // if client shared pointer is valid
        if (client && client->IsConnected())
        {
            client -> Send(msg);
        }
        else
        {
            OnClientDisconnect(client);
            // Delete shared ptr
            client.reset();
            deqConnections.erase(
                std::remove(deqConnections.begin(), deqConnections.end(), client),
                    deqConnections.end());
        }
    }

    void MessageAllClients(message& msg, std::shared_ptr<connection> ignoredClient)
    {
        bool bInvalidClientExists = false;

        for (auto& client : deqConnections)
        {
            if (client && client->IsConnected())
            {
                if (client != ignoredClient)
                    client -> Send(msg);
            }
            else
            {
                OnClientDisconnect(client);
                client.reset();
                bInvalidClientExists = true;
            }
        }

        if (bInvalidClientExists)
        {
            deqConnections.erase(
                std::remove(deqConnections.begin(), deqConnections.end(), nullptr),
                    deqConnections.end());
        }
    }

    // bWait - wait or not for a client to communicate with a server
    void Update(size_t maxMessages = -1, bool bWait = false)
    {
        if (bWait) qMessagesIn.wait();

        size_t messageCount = 0;
        while (messageCount < maxMessages && !qMessagesIn.empty())
        {
            // Grab the front message and remove it from queue
            auto msg = qMessagesIn.pop_front();

            // Pass to message handler
            OnMessage(msg.remote, msg.msg);

            messageCount++;
        }
    }

    virtual bool OnClientConnect(std::shared_ptr<connection> client)
    {
        return false;
    }

    virtual void OnClientDisconnect(std::shared_ptr<connection> client)
    {
    }

    // Called when a message arrived
    virtual void OnMessage(std::shared_ptr<connection> client, message& msg)
    {

    }

    // Thread-safe queue for incoming message packets
    tsqueue<owned_message> qMessagesIn;

    // Container of active validated connections
    std::deque<std::shared_ptr<connection>> deqConnections;

    asio::io_context asioContext;
    std::thread threadContext;

    // to get sockets of the connected clients
    asio::ip::tcp::acceptor asioAcceptor;

    uint32_t nIDCounter = 10000;
};