#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <boost/predef.h> // Tools to identify the OS.

// We need this to enable cancelling of I/O operations on
// Windows XP, Windows Server 2003 and earlier.
// Refer to "http://www.boost.org/doc/libs/1_58_0/
// doc/html/boost_asio/reference/basic_stream_socket/
// cancel/overload1.html" for details.
#ifdef BOOST_OS_WINDOWS
#define _WIN32_WINNT 0x0501

#if _WIN32_WINNT <= 0x0502 // Windows Server 2003 or earlier.
#define BOOST_ASIO_DISABLE_IOCP
#define BOOST_ASIO_ENABLE_CANCELIO
#endif
#endif

#include <boost/asio.hpp>

#include <QObject>

#include <mutex>
#include <iostream>
#include <fstream>
#include <boost/bind.hpp>

using namespace boost;

class HTTPClient;
class HTTPRequest;
class HTTPResponse;

/*!
 * \brief Класс HTTPResponse.
 * Порождается только классом HTTPRequest.
 * \details Содержит в себе код ответа на отправленный
 *          запрос, его заголовки и сообщение
 */
class HTTPResponse
{
    friend class HTTPRequest;
    HTTPResponse() :
        m_response_stream(&m_response_buf)
    {}
public:

    unsigned int get_status_code() const;

    const std::string& get_status_message() const;

    const std::map<std::string, std::string>& get_headers();

    const std::istream& get_response() const;

private:
    asio::streambuf& get_response_buf();

    void set_status_code(unsigned long status_code);

    void set_status_message(const std::string& status_message);

    void add_header(const std::string& name, const std::string& value);
private:
    unsigned long m_status_code; // HTTP status code.
    std::string m_status_message; // HTTP status message.

    // Response headers.
    std::map<std::string, std::string> m_headers;
    asio::streambuf m_response_buf;
    std::istream m_response_stream;
};
/*!
 * \brief Базовый класс HTTPRequest
 * \details Используется для установления HTTPS соединения и
 *          асинхронной передачи запроса
 */
class HTTPRequest : public QObject
{
    Q_OBJECT

protected:

    static const unsigned int DEFAULT_PORT = 80;

    HTTPRequest(asio::io_service& ios, unsigned int id) :
        m_port(DEFAULT_PORT),
        m_id(id),
        m_sock(ios),
        m_resolver(ios),
        m_was_cancelled(false),
        m_ios(ios)
    {}

public:
    void set_host(const std::string& host);
    void set_port(unsigned int port);
    void set_uri(const std::string& uri);

    std::string get_host() const;
    unsigned int get_port() const;
    const std::string& get_uri() const;
    unsigned int get_id() const;

    void execute();
    void cancel();

signals:
    void got_response(const HTTPRequest& request,
                      const HTTPResponse& response,
                      const system::error_code& ec);

protected:

    void on_host_name_resolved(const boost::system::error_code& ec,
                               asio::ip::tcp::resolver::iterator iterator);

    /// Виртуальная функция, отвечающая за формирование запроса
    /// реализуется в дочерних классах
    virtual void compose_request() = 0;

    void on_connection_established(const boost::system::error_code& ec,
                                   asio::ip::tcp::resolver::iterator iterator);

    void on_request_sent(const boost::system::error_code& ec,
                         std::size_t bytes_transferred);

    void on_status_line_received(const boost::system::error_code& ec,
                                 std::size_t bytes_transferred);

    void on_headers_received(const boost::system::error_code& ec,
        std::size_t bytes_transferred);

    void on_response_body_received(const boost::system::error_code& ec,
                                   std::size_t bytes_transferred);

    void on_finish(const boost::system::error_code& ec);

    // Request paramters.
    std::string m_host;
    unsigned int m_port;
    std::string m_uri;
    /*
    std::string m_user_agent;
    std::string m_accept_rules;
    std::string m_content_type;
    */
    // Object unique identifier.
    unsigned int m_id;
    // Buffer containing the request line.

    std::string m_request_buf;

    asio::ip::tcp::socket m_sock;
    asio::ip::tcp::resolver m_resolver;


    HTTPResponse m_response;

    bool m_was_cancelled;
    std::mutex m_cancel_mux;

    asio::io_service& m_ios;
};

#endif // HTTPREQUEST_H
