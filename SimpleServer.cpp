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
        message msg;
        msg.header.type = MsgType::ServerAccept;
        client->Send(msg);

        return true;
    }

    virtual void OnClientDisconnect(std::shared_ptr<connection> client)
    {
    }

    // Called when a message arrived
    virtual void OnMessage(std::shared_ptr<connection> client, message& msg)
    {
        switch (msg.header.type)
        {
            case MsgType::ServerPing:
            {
                std::cout << "[" << client->id << "] Server Ping\n";

                // Simply bounce message back to client
                client->Send(msg);
            }
            break;

            case MsgType::MessageAll:
            {
                std::cout << "[" << client->id << "] Message all\n";
                message msg;
                msg.header.type = MsgType::ServerMessage;
                msg.add(msg, client->id);
                MessageAllClients(msg, client);
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

