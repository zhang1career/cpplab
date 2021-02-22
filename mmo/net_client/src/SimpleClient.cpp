//
// Created by 张荣晋 on 2021/2/19.
//

#include <iostream>
#include <olc_net.h>

enum class CustomMsgTypes : uint32_t {
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage,
};

class CustomClient : public cpplab::net::ClientInterface<CustomMsgTypes> {
public:
    void pingServer()
    {
        cpplab::net::Message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::ServerPing;

        std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

        msg << timeNow;
        send(msg);
    }

    void messageAll()
    {
        cpplab::net::Message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::MessageAll;
        send(msg);
    }
};

int main() {
    CustomClient c;
    c.connect("127.0.0.1", 60000);

    bool key[3] = { false, false, false };
    bool old_key[3] = { false, false, false };

    bool bQuit = false;
    while (!bQuit)
    {
//        if (getForegroundWindow() == getConsoleWindow())
//        {
//            key[0] = getAsyncKeyState('1') & 0x8000;
//            key[1] = getAsyncKeyState('2') & 0x8000;
//            key[2] = getAsyncKeyState('3') & 0x8000;
//        }

        if (key[0] && !old_key[0]) {
            c.pingServer();
        }
        if (key[1] && !old_key[1]) {
            c.messageAll();
        }
        if (key[2] && !old_key[2]) {
            bQuit = true;
        }

        for (int i = 0; i < 3; i++) {
            old_key[i] = key[i];
        }

        if (c.isConnected()) {
            if (!c.incoming().empty()) {
                auto msg = c.incoming().popFront().msg;

                switch (msg.header.id) {
                    case CustomMsgTypes::ServerAccept: {
                        std::cout << "Server Accepted Connection\n";
                    }
                        break;

                    case CustomMsgTypes::ServerPing: {
                        std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
                        std::chrono::system_clock::time_point timeThen;
                        msg >> timeThen;
                        std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n";
                    }
                        break;

                    case CustomMsgTypes::ServerMessage: {
                        uint32_t clientID;
                        msg >> clientID;
                        std::cout << "Hello from [" << clientID << "]\n";
                    }
                        break;
                }
            }
        } else {
            std::cout << "Server Down\n";
            bQuit = true;
        }
    }

    return 0;
}