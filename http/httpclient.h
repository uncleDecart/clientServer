#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include "httppostrequest.h"
#include "httpgetrequest.h"
#include <thread>
#include <memory>

/*!
 * \brief Класс HTTPClient.
 * Определяет определяет потоковую политику
 * Является фабрикой для HTTPPOSTRequest и HTTPGETRequest
 * \details отслеживает выполнение
 */
class HTTPClient {
public:
    HTTPClient(){
        m_work.reset(new boost::asio::io_service::work(m_ios));

        m_thread.reset(new std::thread([this](){
            m_ios.run();
        }));
    }

    std::unique_ptr<HTTPPOSTRequest> create_post_request(unsigned int id);
    std::unique_ptr<HTTPGETRequest> create_get_request(unsigned int id);

    void close();

private:
    asio::io_service m_ios;
    std::unique_ptr<boost::asio::io_service::work> m_work;
    std::unique_ptr<std::thread> m_thread;
};

#endif // HTTPCLIENT_H
