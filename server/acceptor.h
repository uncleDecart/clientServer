#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <boost/asio.hpp>
#include <iostream>

using namespace boost;

class Acceptor
{
public:
    Acceptor(asio::io_service&ios, unsigned short port_num) :
        m_ios(ios),
        m_acceptor(m_ios,
            asio::ip::tcp::endpoint(
                       asio::ip::address_v4::any(),
                       port_num)),
        m_isStopped(false)
    { }

    void Start();
    void Stop();

private:
    void InitAccept();
    void onAccept(const boost::system::error_code& ec,
                  std::shared_ptr<asio::ip::tcp::socket> sock);

private:
    asio::io_service&m_ios;
    asio::ip::tcp::acceptor m_acceptor;
    std::atomic<bool> m_isStopped;
};

#endif // ACCEPTOR_H
