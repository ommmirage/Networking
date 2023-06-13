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

    }
};

int main()
{
    CustomServer server(6000);
    server.Start();

    while(1)
    {
        server.Update();
    }

    return 0;
}

