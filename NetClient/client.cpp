#include "../NetCommon/net_common.h"
#include "../NetCommon/net_message.h"
#include "../NetCommon/net_tsque.h"

int main()
{
    message msg;
    msg.header.type = MsgType::MovePlayer;
    msg.body.push_back(1);
    msg.body.push_back(2);

    tsqueue<int> tsq;
    tsq.push_front(1);

    size_t uint = 3000000000;

    std::cout << sizeof (size_t )<< std::endl;
}