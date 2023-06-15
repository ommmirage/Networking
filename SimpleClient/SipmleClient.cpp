#include <iostream>
#include <iomanip>
#include <ctime>
#include "../NetCommon/net.h"

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

        // timeNow = msg.get<std::chrono::system_clock::time_point>(msg);

        // std::time_t timeNow_t = std::chrono::system_clock::to_time_t(timeNow);
        // std::tm* timeNow_tm = std::localtime(&timeNow_t);
        // char buffer[80];
        // std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeNow_tm);

        // std::cout << buffer << "\n";
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