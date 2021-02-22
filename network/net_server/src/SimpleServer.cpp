//
// Created by 张荣晋 on 2021/2/20.
//

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

class CustomServer : public cpplab::net::ServerInterface<CustomMsgTypes> {
public:
    CustomServer(uint16_t nPort) : ServerInterface<CustomMsgTypes>(nPort) {}

public:
    void onClientValidated(std::shared_ptr<cpplab::net::Connection<CustomMsgTypes>> client) override {
        cpplab::net::Message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::ServerAccept;
        client->send(msg);
    }

protected:
    bool onClientConnect(std::shared_ptr<cpplab::net::Connection<CustomMsgTypes>> client) override {
        return true;
    }

    void onClientDisconnect(std::shared_ptr<cpplab::net::Connection<CustomMsgTypes>> client) override {
        std::cout << "Removing client [" << client->getId() << "]\n";
    }

    void onMessage(std::shared_ptr<cpplab::net::Connection<CustomMsgTypes>> client, cpplab::net::Message<CustomMsgTypes>& msg) override {
        switch (msg.header.id) {
            case CustomMsgTypes::ServerPing: {
                std::cout << "[" << client->getId() << "]: Server Ping\n";
                client->send(msg);
            }
                break;

            case CustomMsgTypes::MessageAll: {
                std::cout << "[" << client->getId() << "]: Message All\n";
                cpplab::net::Message<CustomMsgTypes> msg;
                msg.header.id = CustomMsgTypes::ServerMessage;
                msg << client->getId();
                messageAllClients(msg, client);
            }
                break;
            case CustomMsgTypes::ServerAccept:
                break;
            case CustomMsgTypes::ServerDeny:
                break;
            case CustomMsgTypes::ServerMessage:
                break;
        }
    }
};

int main() {
    CustomServer server(60000);
    server.start();

    while(1) {
        server.update(-1, true);
    }
    return 0;
}
