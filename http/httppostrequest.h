#ifndef HTTPPOSTREQUEST_H
#define HTTPPOSTREQUEST_H

#include "httprequest.h"

/*!
 * \brief Дочерний класс HTTPPOSTRequest
 * \details Наследуется от класса HTTPRequest,
 *          формирует POST запрос, который
 *          отправляет файл
 *
 */
class HTTPPOSTRequest : public HTTPRequest
{
    friend class HTTPClient;

    HTTPPOSTRequest(asio::io_service& ios, unsigned int id); //! потому что только клиент может создать запрос
public:
    void set_filepath(std::string fp);
    std::string get_filepath();
private:
    std::string m_filepath;
    static const std::string DEFAULT_FILEPATH;
protected:
    void compose_request();
};


#endif // HTTPPOSTREQUEST_H
