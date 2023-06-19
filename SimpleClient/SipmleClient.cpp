#include <iostream>
#include "../NetCommon/net.h"
#include "../NetCommon/raylib.h"

class CustomClient : public client_interface
{
public:
    void PingServer()
    {
        message msg;
        msg.header.type = MsgType::ServerPing;

        std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

        msg.add(msg, timeNow);
        Send(msg);
    }
};

int main()
{
    CustomClient client;
    client.Connect("127.0.0.1", 60003);

    client.PingServer();

    bool bQuit = false;
    while (!bQuit)
    {
        if (client.IsConnected())
        {
            if (!client.Incoming().empty())
            {
                message msg = client.Incoming().pop_front().msg;

                switch (msg.header.type)
                {
                    case MsgType::ServerPing:
                    {
                        std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
                        std::chrono::system_clock::time_point timeThen;
                        timeThen = msg.get<std::chrono::system_clock::time_point>(msg);
                        std::cout << "Ping:  " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n";
                    }
                    break;

                    case MsgType::ServerAccept:
                    {
                        std::cout << "Server accepted connection\n";
                    }
                    break;
                }
            }
        }
        else
        {
            std::cout << "Server Down\n";
            bQuit = true;
        }
    }

    return 0;
}