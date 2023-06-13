#include <iostream>

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

#include <chrono>
#include <thread>

std::vector<char> vBuffer(1024);

void GrabSomeData(asio::ip::tcp::socket& socket)
{
    // It will prime the context with some work to do when data is available on this
    // socket to read
    socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()),
        [&](std::error_code ec, std::size_t length)
        {
            if (!ec)
            {
                std::cout << "\n\nRead " << length << " bytes\n\n";

                for (int i = 0; i < length; i++)
                    std::cout << vBuffer[i];

                GrabSomeData(socket);
            }
        }
    );
}

int main()
{
    asio::error_code ec;

    // Create a context - essentially the platform specific interface
    asio::io_context context;

    // We are loading the context with instructions with witch the context has to wait
    // for certain conditions. And then it will execute code associated with those instructions.

    // Give some fake tasks to asio so the context doesn't finish before it got real job.
    asio::io_context::work idleWork(context);

    // We run the context in its own thread. This gives the context some temporal space
    // within which it can execute this instructions without blocking main program.
    // Context.run() will return as soon as context is run out of things to do, a.i.
    // there are no instructions registered with context to do in the future
    std::thread thrContext = std::thread([&]() { context.run(); });

    // Get the address of somewhere we wish to connect to
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address("93.184.216.34", ec), 80);

    // The context will deliver the implementation
    asio::ip::tcp::socket socket(context);

    // Tell socket to try to connect
    socket.connect(endpoint, ec);

    if (!ec)
    {
        std::cout << "Connected!" << std::endl;
    }
    else
    {
        std::cout << "Failed to connect to address:\n" << ec.message() << std::endl;
    }

    if (socket.is_open())
    {
        GrabSomeData(socket);

        std::string sRequest =
            "GET /index.html HTTP/1.1\r\n"
            "Host: example.com\r\n"
            "Connection: close\r\n\r\n";

        socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);

        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }

    return 0;
}