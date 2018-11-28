#include "httprequest.h"
#include "fileparser.h"


namespace http_errors {
    enum http_error_codes
    {
        invalid_response = 1
    };

    class http_errors_category
        : public boost::system::error_category
    {
    public:
        const char* name() const BOOST_SYSTEM_NOEXCEPT { return "http_errors"; }

        std::string message(int e) const {
            switch (e) {
            case invalid_response:
                return "Server response cannot be parsed.";
                break;
            default:
                return "Unknown error.";
                break;
            }
        }
    };

    const boost::system::error_category&
        get_http_errors_category()
    {
            static http_errors_category cat;
            return cat;
        }

    boost::system::error_code
        make_error_code(http_error_codes e)
    {
            return boost::system::error_code(
                static_cast<int>(e), get_http_errors_category());
    }
} // namespace http_errors
namespace boost {
    namespace system {
        template<>
        struct is_error_code_enum
            <http_errors::http_error_codes>
        {
            BOOST_STATIC_CONSTANT(bool, value = true);
        };
    } // namespace system
} // namespace boost



using namespace boost;


    unsigned int HTTPResponse::get_status_code() const {
        return m_status_code;
    }

    const std::string& HTTPResponse::get_status_message() const {
        return m_status_message;
    }

    const std::map<std::string, std::string>& HTTPResponse::get_headers() {
        return m_headers;
    }

    const std::istream& HTTPResponse::get_response() const {
        return m_response_stream;
    }

    asio::streambuf& HTTPResponse::get_response_buf() {
        return m_response_buf;
    }

    void HTTPResponse::set_status_code(unsigned int status_code) {
        m_status_code = status_code;
    }

    void HTTPResponse::set_status_message(const std::string& status_message) {
        m_status_message = status_message;
    }

    void HTTPResponse::add_header(const std::string& name,
        const std::string& value)
    {
        m_headers[name] = value;
    }

    void HTTPRequest::set_host(const std::string& host) {
        m_host = host;
    }

    void HTTPRequest::set_port(unsigned int port) {
        m_port = port;
    }

    void HTTPRequest::set_uri(const std::string& uri) {
        m_uri = uri;
    }

    std::string HTTPRequest::get_host() const {
        return m_host;
    }

    unsigned int HTTPRequest::get_port() const {
        return m_port;
    }

    const std::string& HTTPRequest::get_uri() const {
        return m_uri;
    }

    unsigned int HTTPRequest::get_id() const {
        return m_id;
    }

    /*!
     * \brief HTTPRequest::on_peer_verify
     * Проверяет валидность сертификата
     * \details Вызывается во время установления TLS/SSL соединения
     * функцией async_handshake из функции on_connection_established
     * \param preverified см. стандарт проверки
     * \param ctx сертификат
     * \return
     */
    bool HTTPRequest::on_peer_verify(bool preverified, asio::ssl::verify_context& ctx)
    {
        int8_t subject_name[256];
        X509_STORE_CTX *cts = ctx.native_handle();

        X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
        std::cout << "CTX ERROR : " << cts->error << std::endl;

        int32_t depth = X509_STORE_CTX_get_error_depth(cts);
        std::cout << "CTX DEPTH : " << depth << std::endl;

        switch (cts->error)
        {
        case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
            std::cout << "X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT" << std::endl;
            break;
        case X509_V_ERR_CERT_NOT_YET_VALID:
        case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
            std::cout << "Certificate not yet valid!!" << std::endl;
            break;
        case X509_V_ERR_CERT_HAS_EXPIRED:
        case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
            std::cout << "Certificate expired.." << std::endl;
            break;
        case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
            std::cout <<  "Self signed certificate in chain!!!\n" << std::endl;
            preverified = true;
            break;
        default:
            break;
        }
        const int32_t name_length = 256;
        X509_NAME_oneline(X509_get_subject_name(cert), reinterpret_cast<char*>(subject_name), name_length);
        std::cout << "Verifying " << subject_name << std::endl;
        std::cout << "Verification status : " << preverified << std::endl;

        return preverified;
    }

    /*!
     * \brief HTTPRequest::execute
     * Проверяется правильность данных, используемых для соединения
     * разрешается имя хоста и управление передаётся функции
     * on_host_name_resolved
     */
    void HTTPRequest::execute()
    {
        // Ensure that precorditions hold.
        assert(m_port > 0);
        assert(m_host.length() > 0);
        assert(m_uri.length() > 0);

        // Prepare the resolving query.
        asio::ip::tcp::resolver::query resolver_query(m_host,
            std::to_string(m_port),
            asio::ip::tcp::resolver::query::numeric_service);

        std::unique_lock<std::mutex>
            cancel_lock(m_cancel_mux);

        if (m_was_cancelled)
        {
            cancel_lock.unlock();
            on_finish(boost::system::error_code(
                asio::error::operation_aborted));
            return;
        }

        // Resolve the host name.
        m_resolver.async_resolve(resolver_query,
            [this](const boost::system::error_code& ec,
            asio::ip::tcp::resolver::iterator iterator)
        {
            on_host_name_resolved(ec, iterator);
        });
    }

    void HTTPRequest::cancel() {
        std::unique_lock<std::mutex>
            cancel_lock(m_cancel_mux);

        m_was_cancelled = true;

        m_resolver.cancel();

        if (m_sock.lowest_layer().is_open()) {
            m_sock.lowest_layer().cancel();
        }
    }

    /*!
     * \brief HTTPRequest::on_host_name_resolved
     * Вызывается после разрешения имени хоста функцией
     * execute
     * \details
     * Осуществляет подключение к хосту.
     * Далее вызывается функция
     * on_connection_established.
     * \param ec код ошибки предыдущей операции
     * \param iterator список ip-адресов, соответсвующих
     * данному имени хоста
     */
    void HTTPRequest::on_host_name_resolved(
        const boost::system::error_code& ec,
        asio::ip::tcp::resolver::iterator iterator)
    {
        if (ec != 0) {
            on_finish(ec);
            return;
        }

        std::unique_lock<std::mutex>
            cancel_lock(m_cancel_mux);

        if (m_was_cancelled) {
            cancel_lock.unlock();
            on_finish(boost::system::error_code(
                asio::error::operation_aborted));
            return;
        }

        // Connect to the host.
        asio::async_connect(m_sock.lowest_layer(),
            iterator,
            [this](const boost::system::error_code& ec,
            asio::ip::tcp::resolver::iterator iterator)
        {
            on_connection_established(ec, iterator);
        });

    }
    /*!
     * \brief HTTPRequest::on_connection_established
     * Вызывается после установления соединения функцией
     * on_host_name_resolved
     * \details
     * Осуществляет рукопожатие SSL/TLS.
     * Во время рукопожатия вызывается функция
     * on_peer_verify
     * Далее управление передаётся функции
     * on_handshake_complete
     * \param ec
     * \param iterator
     */
    void HTTPRequest::on_connection_established(
        const boost::system::error_code& ec,
        asio::ip::tcp::resolver::iterator iterator)
    {
            if (ec != 0)
            {
                on_finish(ec);
                return;
            }
            m_sock.async_handshake(asio::ssl::stream_base::client,
                                   boost::bind(&HTTPRequest::on_handshake_complete, this,
                                               asio::placeholders::error));
    }

    /*!
     * \brief HTTPRequest::on_handshake_complete
     * Вызывается после успешного рукопожатия, выполненного функцией
     * on_connection_established
     * \details
     * Компанует искомый {GET, POST} запрос и отправляет его на хост
     * После этого управление передаётся функции
     * on_request_sent
     * \param ec
     */
    void HTTPRequest::on_handshake_complete(
        const boost::system::error_code& ec)
    {
        std::cout << "handshake complete" << std::endl;

            if (ec != 0)
            {
                on_finish(ec);
                return;
            }

            // Compose the request message.
            compose_request();

            std::unique_lock<std::mutex>
                cancel_lock(m_cancel_mux);

            if (m_was_cancelled) {
                cancel_lock.unlock();
                on_finish(boost::system::error_code(
                    asio::error::operation_aborted));
                return;
            }

            // Send the request message.
            asio::async_write(m_sock,
                asio::buffer(m_request_buf),
                [this](const boost::system::error_code& ec,
                std::size_t bytes_transferred)
            {
                on_request_sent(ec, bytes_transferred);
            });
    }

    /*!
     * \brief HTTPRequest::on_request_sent
     * Вызывается после отправки запроса функцией
     * on_handshake_complete
     * \details
     * закрывает соединение считывает строку состояние ответа на запрос
     * дальше управление передаётся функции
     * on_status_line_received
     * \param ec
     * \param bytes_transferred
     */
    void HTTPRequest::on_request_sent(const boost::system::error_code& ec,
        std::size_t bytes_transferred)
    {
        std::cout << "request sent" << std::endl;

            if (ec != 0)
            {
                on_finish(ec);
                return;
            }

            m_sock.lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_send);

            std::unique_lock<std::mutex>
                cancel_lock(m_cancel_mux);

            if (m_was_cancelled)
            {
                cancel_lock.unlock();
                on_finish(boost::system::error_code(
                    asio::error::operation_aborted));
                return;
            }


            // Read the status line.
            asio::async_read_until(m_sock,
                m_response.get_response_buf(),
                "\r\n",
                [this](const boost::system::error_code& ec,
                std::size_t bytes_transferred)
            {
                on_status_line_received(ec, bytes_transferred);
            });
        }
    /*!
     * \brief HTTPRequest::on_status_line_received
     * Вызывается после считывания строки состояния
     * ответа на запрос функцией on_request_sent
     * \details
     * Парсит строку состояния и считывает заголовки
     * ответа
     * Дальше управление передаётся функции
     * on_headers_received
     * \param ec
     * \param bytes_transferred
     */
    void HTTPRequest::on_status_line_received(
        const boost::system::error_code& ec,
        std::size_t bytes_transferred)
    {
        std::cout << "status line received" << std::endl;

            if (ec != 0)
            {
                on_finish(ec);
                return;
            }

            // Parse the status line.
            std::string http_version;
            std::string str_status_code;
            std::string status_message;

            std::istream response_stream(&m_response.get_response_buf());
            response_stream >> http_version;

            if (http_version != "HTTP/1.1" && http_version != "HTTP/1.0")
            {
                // Response is incorrect.
                std::cout << "Invalid HTTP version! " << http_version << std::endl;
                on_finish(http_errors::invalid_response);
                return;
            }

            response_stream >> str_status_code;

            // Convert status code to integer.
            unsigned int status_code = 200;

            try
            {
                status_code = std::stoul(str_status_code);
            }
            catch (std::logic_error&)
            {
                // Response is incorrect.
                on_finish(http_errors::invalid_response);
                return;
            }

            std::getline(response_stream, status_message, '\r');
            // Remove symbol '\n' from the buffer.
            response_stream.get();

            m_response.set_status_code(status_code);
            m_response.set_status_message(status_message);

            std::unique_lock<std::mutex> cancel_lock(m_cancel_mux);

            if (m_was_cancelled)
            {
                cancel_lock.unlock();
                on_finish(boost::system::error_code(asio::error::operation_aborted));
                return;
            }

            // At this point the status line is successfully
            // received and parsed.
            // Now read the response headers.
            asio::async_read_until(m_sock,
                m_response.get_response_buf(),
                "\r\n\r\n",
                [this](
                const boost::system::error_code& ec,
                std::size_t bytes_transferred)
            {
                on_headers_received(ec,
                    bytes_transferred);
            });
        }
    /*!
     * \brief HTTPRequest::on_headers_received
     * Вызывается после считывания заголовков ответа
     * на исходный запрос функцией
     * on_status_line_received
     * \details
     * Парсятся заголовки, считывается тело ответа
     * Далее управление передаётся функции
     * on_response_body_received
     * \param ec
     * \param bytes_transferred
     */
    void HTTPRequest::on_headers_received(const boost::system::error_code& ec,
                                          std::size_t bytes_transferred)
    {
        std::cout << "headers received" << std::endl;

        if (ec != 0)
        {
            on_finish(ec);
            return;
        }

        // Parse and store headers.
        std::string header, header_name, header_value;
        std::istream response_stream(&m_response.get_response_buf());

        while (true)
        {
            std::getline(response_stream, header, '\r');

            // Remove \n symbol from the stream.
            response_stream.get();

            if (header == "")
                break;

            size_t separator_pos = header.find(':');
            if (separator_pos != std::string::npos)
            {
                header_name = header.substr(0,
                    separator_pos);

                if (separator_pos < header.length() - 1)
                    header_value = header.substr(separator_pos + 1);
                else
                    header_value = "";

                m_response.add_header(header_name,
                    header_value);
            }
        }

        std::unique_lock<std::mutex>
            cancel_lock(m_cancel_mux);

        if (m_was_cancelled)
        {
            cancel_lock.unlock();
            on_finish(boost::system::error_code(
                asio::error::operation_aborted));
            return;
        }

        // Now we want to read the response body.
        asio::async_read(m_sock,
            m_response.get_response_buf(),
            [this](
            const boost::system::error_code& ec,
            std::size_t bytes_transferred)
        {
            on_response_body_received(ec,
                bytes_transferred);
        });

        return;
    }
    /*!
     * \brief HTTPRequest::on_response_body_received
     * Вызывается после считывания тела ответа функцией
     * on_headers_received
     * \details
     * Передаёт управление функции on_finish с кодом
     * ошибки ec, если не было конца считывания файла
     * \param ec
     * \param bytes_transferred
     */
    void HTTPRequest::on_response_body_received(
        const boost::system::error_code& ec,
        std::size_t bytes_transferred)
    {
        std::cout << "response body received" << std::endl;

        if (ec == asio::error::eof)
            on_finish(boost::system::error_code());
        else
            on_finish(ec);
    }
    /*!
     * \brief HTTPRequest::on_finish
     * Вызывается функцией on_response_body_received
     * \details
     * По завершении срабатывает сигнал got_response.
     * \param ec
     */
    void HTTPRequest::on_finish(const boost::system::error_code& ec)
    {
        if (ec != 0) {
            std::cout << "Error occured! Error code = "
                << ec.value()
                << ". Message: " << ec.message() << std::endl;
        }
        emit got_response(*this, m_response, ec);

        return;
    }

