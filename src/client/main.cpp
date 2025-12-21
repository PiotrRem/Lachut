#include "network.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <unistd.h>

int main() {
    NetworkClient netClient;

    netClient.connectToServer("localhost", "4444");

    while (true) {
        netClient.queueMessage("JOIN 123432\n");

        netClient.pollEvents();

        while (netClient.hasMessage()) {
            std::string msg = netClient.popMessage();
            std::cout << "otrzymano: " << msg << '\n';
        }

        sleep(1);
    }

    return 0;
}
