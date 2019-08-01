#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "httprequest.h"
#include <thread>
#include <memory>

class HTTPServer
{
public:
    HTTPServer() {
        m_work.reset(new boost::asio::io_service::work(m_ios));

        m_thread.reset(new std::thread([this]() {
            m_ios.run();
        }));
    }

    std::unique_ptr<HTTPRequest> handle_request(asio::streambuf& request);

private:
    asio::io_service m_ios;
    std::unique_ptr<boost::asio::io_service::work> m_work;
    std::unique_ptr<std::thread> m_thread;
};

#endif // HTTPSERVER_H
