#include "server.h"

const unsigned int DEFAULT_THREAD_POOL_SIZE = 2;

int main()
{
    unsigned short port_num = 3333;

    try {
        Server srv;

        unsigned int thread_pool_size = std::thread::hardware_concurrency() * 2;

        if (thread_pool_size == 0)
            thread_pool_size = DEFAULT_THREAD_POOL_SIZE;

        srv.Start(port_num, thread_pool_size);

        std::this_thread::sleep_for(std::chrono::seconds(180));

        srv.Stop();
    }
    catch (system::system_error& e) {
        std::cout << "Error occured! Error code = "
                  << e.code() << ". Message: "
                  << e.what();
    }

    return 0;
}

