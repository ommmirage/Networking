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

    void MessageAll()
    {
        message msg;
        msg.header.type = MsgType::MessageAll;
        Send(msg);
    }
};

int main()
{
    InitWindow(32, 32, "raylib [core] example - basic window");

    SetTargetFPS(30);

    CustomClient client;
    client.Connect("127.0.0.1", 60003);

    bool bQuit = false;

    while (!WindowShouldClose() && !bQuit)
    {
        BeginDrawing();
            // ClearBackground(RAYWHITE);
        EndDrawing();

        if (client.IsConnected())
        {
            if (IsKeyPressed(KEY_A)) client.PingServer();
            if (IsKeyPressed(KEY_S)) client.MessageAll();

            if (!client.Incoming().empty())
            {
                message msg = client.Incoming().pop_front().msg;

                switch (msg.header.type)
                {
                    case MsgType::ServerAccept:
                    {
                        std::cout << "Server accepted connection\n";
                    }
                    break;

                    case MsgType::ServerPing:
                    {
                        std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
                        std::chrono::system_clock::time_point timeThen;
                        timeThen = msg.get<std::chrono::system_clock::time_point>(msg);
                        std::cout << "Ping:  " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n";
                    }
                    break;

                    case MsgType::ServerMessage:
                    {
                        uint32_t client_id = msg.get<uint32_t>(msg);
                        std::cout << "Message from [" << client_id << "]\n";
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

    CloseWindow();

    return 0;
}