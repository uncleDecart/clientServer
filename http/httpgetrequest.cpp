#include "httpgetrequest.h"

const std::string HTTPGETRequest::DEFAULT_MESSAGE = "";

HTTPGETRequest::HTTPGETRequest(asio::io_service& ios, unsigned int id) :
    HTTPRequest(ios, id),
    m_message(DEFAULT_MESSAGE)
{
}

void HTTPGETRequest::set_message(std::__cxx11::string message) {
    m_message = message;
}

/*
GET /docs/index.html HTTP/1.1
Host: www.nowhere123.com
Accept: text/html, 8/8
Accept-Language: en-us,en;q=0.5
Accept-Encoding: gzip, deflate
User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1)
(blank line)
*/

void HTTPGETRequest::compose_request()
{
    m_request_buf += "GET " + m_uri + " HTTP/1.1\r\n";

    // Add mandatory header.
    m_request_buf += "Host: " + m_host + "\r\n";

    m_request_buf += "User-Agent: C/1.0\r\n";

    m_request_buf += "Accept: text/html, */*\r\n";

    m_request_buf += "Accept-Language: en-us,en;q=0.5\r\n";

    m_request_buf += "Accept-Encoding: gzip, deflate\r\n";

    m_request_buf += "\r\n";

}
