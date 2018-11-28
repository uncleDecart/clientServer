#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include "acceptor.h"
#include <thread>

using namespace boost;

class Server
{
public:
    Server() {
        m_work.reset(new asio::io_service::work(m_ios));
    }

    void Start(unsigned short port_num,
               unsigned int thread_pool_size);
    void Stop();

private:
    asio::io_service m_ios;
    std::unique_ptr<asio::io_service::work> m_work;
    std::unique_ptr<Acceptor> m_acc;
    std::vector<std::unique_ptr<std::thread>> m_thread_pool;
};

#endif // SERVER_H
