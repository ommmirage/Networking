#pragma once
#include "net_common.h"
#include "net_message.h"
#include "net_tsque.h"
#include "net_connection.h"

class client_interface
{
public:
    client_interface() : sock(context)
    {
        // Initialise the socket with the io context
    }

    ~client_interface()
    {
        Disconnect();
    }

    // Connect to server with hostmane/ip address and port
    bool Connect(const std::string &host, const uint16_t port)
    {
        try
        {
            // Resolve hostname/ip-address в ощутимый физический адрес
            asio::ip::tcp::resolver resolver(context);
            asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

            // Create coonection
            conn = std::make_unique<connection>(
                connection::owner::client,
                context,
                asio::ip::tcp::socket(context),
                qMessagesIn);

            // Tell the connection object to connect to server
            conn->ConnectToServer(endpoints);

            thrContext = std::thread([this](){ context.run(); });
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            return false;
        }

        return true;
    }

    void Disconnect()
    {
    }

    bool IsConnected()
    {
        if (conn)
            return true;
        else
            return false;
    }

    // Send message to server
    void Send(const message &msg)
    {
        if (IsConnected())
        {
            conn->Send(msg);
        }
    }

    // Retrieve queue of messages from server
    tsqueue<owned_message> &Incoming()
    {
        return qMessagesIn;
    }

    // asio context handles the data transfer
    asio::io_context context;
    // it need a thread to execute its commands
    std::thread thrContext;
    // This is the hardware socket that is connected to the server
    asio::ip::tcp::socket sock;
    // The client has a single instance of a "connection" object,
    // which handles data transfer
    std::unique_ptr<connection> conn;

    tsqueue<owned_message> qMessagesIn;
};