#include "../NetCommon/net_common.h"
#include "../NetCommon/net_message.h"
#include "../NetCommon/net_tsque.h"
#include "../NetCommon/net_client.h"

int main()
{
    message msg;
    // msg.header.type = MsgType::MovePlayer;
    msg.body.push_back(1);
    msg.body.push_back(2);

    tsqueue<int> tsq;
    tsq.push_front(1);

    std::cout << sizeof (size_t )<< std::endl;
}