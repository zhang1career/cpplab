//
// Created by 张荣晋 on 2021/2/20.
//

#include "SimpleServer.h"

int main() {
    CustomServer server(60000);
    server.start();

    while(1) {
        server.update(-1, true);
    }
    return 0;
}
