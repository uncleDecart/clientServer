#include "service.h"
#include <iostream>
#include <thread>
#include <iterator>
#include <sstream>

void Service::StartHandling() {
    asio::async_read_until(*m_sock.get(),
                           m_request,
                           '\n',
                           [this](const boost::system::error_code&ec,
                                        std::size_t bytes_transfered)
                            {
                                onRequestReceived(ec, bytes_transfered);
                            });
}

void Service::onRequestReceived(const boost::system::error_code&ec,
                                std::size_t bytes_transfered)
{
    if (ec != 0) {
        std::cout << "Error occured! Error code = "
                  << ec.value()
                  << ". Message: " << ec.message();
        onFinish();
        return;
    }

    m_response = ProcessRequest(m_request);
    std::string s((std::istreambuf_iterator<char>(&m_request)),
                   std::istreambuf_iterator<char>());

    std::cout << "received request with : " << s << std::endl;

    asio::async_write(*m_sock.get(),
                      asio::buffer(m_response),
                      [this](const boost::system::error_code& ec,
                             std::size_t bytes_transfered)
    {
       onResponseSent(ec, bytes_transfered);
    });
}

void Service::onResponseSent(const boost::system::error_code& ec,
                             std::size_t bytes_transfered)
{
    if (ec != 0) {
        std::cout << "Error occured! Error code = "
                  << ec.value()
                  << ". Message: " << ec.message();
    }
    onFinish();
}

void Service::onFinish()
{
    delete this;
}

std::string Service::ProcessRequest(asio::streambuf& request)
{
    int i = 0;
    while (i != 1000000)
        i++;

    std::this_thread::sleep_for(
                std::chrono::seconds(3));

    std::string response = "Response!!\n";

    return response;
}
