//
// Created by 张荣晋 on 2021/2/19.
//

#ifndef NET_CLIENT_SIMPLECLIENT_H
#define NET_CLIENT_SIMPLECLIENT_H

#include <iostream>
#include <olc_net.h>

enum class CustomMsgTypes : uint32_t {
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage,
};

class CustomClient : public olc::net::ClientInterface<CustomMsgTypes> {
public:
    void pingServer()
    {
        olc::net::Message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::ServerPing;

        std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

        msg << timeNow;
        send(msg);
    }

    void messageAll()
    {
        olc::net::Message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::MessageAll;
        send(msg);
    }
};

#endif //NET_CLIENT_SIMPLECLIENT_H
