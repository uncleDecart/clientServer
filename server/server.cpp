#include "server.h"

void Server::Start(unsigned short port_num,
                     unsigned int thread_pool_size)
{
    assert(thread_pool_size > 0);

    m_acc.reset(new Acceptor(m_ios, port_num));
    m_acc->Start();

    for (unsigned int i = 0; i < thread_pool_size; i++) {
        std::unique_ptr<std::thread> th(
                    new std::thread([this] ()
        {
            m_ios.run();
        }));

        m_thread_pool.push_back(std::move(th));
    }
}

void Server::Stop()
{
    m_acc->Stop();
    m_ios.stop();

    for (auto& th : m_thread_pool)
        th->join();
}
