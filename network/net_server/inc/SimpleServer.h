//
// Created by 张荣晋 on 2021/2/20.
//

#ifndef NET_SERVER_SIMPLESERVER_H
#define NET_SERVER_SIMPLESERVER_H

#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"
#include "net_client.h"
#include "net_server.h"
#include "net_connection.h"

enum class CustomMsgTypes : uint32_t {
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage,
};

class CustomServer : public olc::net::ServerInterface<CustomMsgTypes> {
public:
    CustomServer(uint16_t nPort) : olc::net::ServerInterface<CustomMsgTypes>(nPort) {}

protected:
    virtual bool onClientConnect(std::shared_ptr<olc::net::Connection<CustomMsgTypes>> client) {
        olc::net::Message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::ServerAccept;
        client->send(msg);
        return true;
    }

    virtual void onClientDisconnect(std::shared_ptr<olc::net::Connection<CustomMsgTypes>> client)
    {
        std::cout << "Removing client [" << client->getId() << "]\n";
    }

    virtual void onMessage(std::shared_ptr<olc::net::Connection<CustomMsgTypes>> client, olc::net::Message<CustomMsgTypes> msg) {
        switch (msg.header.id) {
            case CustomMsgTypes::ServerPing: {
                std::cout << "[" << client->getId() << "]: Server Ping\n";
                client->send(msg);
            }
            break;

            case CustomMsgTypes::MessageAll: {
                std::cout << "[" << client->getId() << "]: Message All\n";
                olc::net::Message<CustomMsgTypes> msg;
                msg.header.id = CustomMsgTypes::ServerMessage;
                msg << client->getId();
                messageAllClients(msg, client);
            }
            break;
        }
    }
};

#endif //NET_SERVER_SIMPLESERVER_H
