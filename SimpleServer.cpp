#include <iostream>
#include "NetCommon/net.h"

class CustomServer : public server_interface
{
public:
    CustomServer(uint16_t port) : server_interface(port)
    {

    }

    virtual bool OnClientConnect(std::shared_ptr<connection> client)
    {
        return true;
    }

    virtual void OnClientDisconnect(std::shared_ptr<connection> client)
    {
    }

    // Called when a message arrived
    virtual void OnMessage(std::shared_ptr<connection> client, message& msg)
    {
        // std::cout << "Message arrived!\n";
        switch (msg.header.type)
        {
            case MsgType::ServerPing:
            {
                std::cout << "[" << client->id << "]: Server Ping\n";

                // Simply bounce message back to client
                client->Send(msg);
            }
            break;
        }
    }
};

int main()
{
    CustomServer server(60003);
    server.Start();

    while(1)
    {
        server.Update();
    }

    return 0;
}

