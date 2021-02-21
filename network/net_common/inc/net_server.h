//
// Created by 张荣晋 on 2021/2/20.
//

#ifndef NET_COMMON_NET_SERVER_H
#define NET_COMMON_NET_SERVER_H

#include "net_common.h"
#include "net_connection.h"
#include "net_message.h"
#include "net_tsqueue.h"

namespace olc {
    namespace net {
        template<typename T>
        class ServerInterface;
    }
}

#endif //NET_COMMON_NET_SERVER_H
