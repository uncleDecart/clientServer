#ifndef HTTPGETREQUEST_H
#define HTTPGETREQUEST_H

#include "httprequest.h"

/*!
 * \brief Дочерний класс HTTPGETRequest
 * \details Наследуется от класса HTTPRequest
 * формирует GET запрос
 */
class HTTPGETRequest : public HTTPRequest
{
    friend class HTTPClient;

    HTTPGETRequest(asio::io_service& ios, unsigned int id); //! потому что только клиент может создать запрос
public:
    void set_message(std::string message);

private:

    static const std::string DEFAULT_MESSAGE;

    void compose_request();

    std::string m_message;
};

#endif // HTTPGETREQUEST_H
