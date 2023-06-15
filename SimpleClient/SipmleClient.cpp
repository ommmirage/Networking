#include <iostream>
#include "../NetCommon/net.h"

class CustomClient : public client_interface
{

};

int main()
{
    CustomClient client;
    client.Connect("127.0.0.1", 60000);
    return 0;
}