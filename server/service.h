#ifndef SERVICE_H
#define SERVICE_H

#include <boost/asio.hpp>

using namespace boost;

class Service
{
public:
    Service(std::shared_ptr<asio::ip::tcp::socket> sock) :
        m_sock(sock)
    {}

    void StartHandling();

private:
    void onRequestReceived(const boost::system::error_code& ec,
                           std::size_t bytes_transfered);
    void onResponseSent(const boost::system::error_code& ec,
                        std::size_t bytes_transfered);
    void onFinish();

    std::string ProcessRequest(asio::streambuf& request);

private:
    std::shared_ptr<asio::ip::tcp::socket> m_sock;
    std::string m_response;
    asio::streambuf m_request;
};

#endif // SERVICE_H
