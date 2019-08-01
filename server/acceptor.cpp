#include "acceptor.h"
#include <service.h>

void Acceptor::Start()
{
    m_acceptor.listen();
    InitAccept();
}

void Acceptor::Stop()
{
    m_isStopped.store(true);
}

void Acceptor::InitAccept()
{
    std::shared_ptr<asio::ip::tcp::socket>
            sock(new asio::ip::tcp::socket(m_ios));

    m_acceptor.async_accept(*sock.get(),
                            [this, sock](
                            const boost::system::error_code& error)
    {
       onAccept(error, sock);
    });
}

void Acceptor::onAccept(const::boost::system::error_code&ec,
                        std::shared_ptr<asio::ip::tcp::socket> sock)
{
    if (ec == nullptr) {
        (new Service(sock))->start_handling();
    }
    else {
        std::cout << "Error occured! Error code = "
                  << ec.value()
                  << ". Message: " << ec.message();
    }

    if (!m_isStopped.load())
        InitAccept();
    else
        m_acceptor.close();
}
