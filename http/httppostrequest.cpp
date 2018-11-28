#include "httppostrequest.h"

const std::string HTTPPOSTRequest::DEFAULT_FILEPATH = "";

HTTPPOSTRequest::HTTPPOSTRequest(asio::io_service& ios, unsigned int id) :
    HTTPRequest(ios, id),
    m_filepath(DEFAULT_FILEPATH)
{
}

void HTTPPOSTRequest::set_filepath(std::string fp) {
    m_filepath = fp;
}
std::string HTTPPOSTRequest::get_filepath() {
    return m_filepath;
}

void HTTPPOSTRequest::compose_request()
{
    assert(m_filepath != DEFAULT_FILEPATH);

    std::ifstream ifs(m_filepath, std::ios::binary);

    ifs.seekg(0, std::ios_base::end);
    std::fstream::pos_type size = ifs.tellg();
    ifs.seekg(0, std::ios_base::beg);


    std::string content;
    content.resize(size);
    ifs.read(&content[0], content.size());
    ifs.close();

    m_request_buf += "POST " + m_uri + " HTTP/1.1\r\n";

    // Add mandatory header.
    m_request_buf += "Host: " + m_host + "\r\n";

    m_request_buf += "User-Agent: C/1.0\r\n";

    m_request_buf += "Accept: */*\r\n";

    m_request_buf += "Content-Length: ";
    m_request_buf += std::to_string(content.size());
    m_request_buf += "\r\n";

    m_request_buf += "Content-Type: application/x-www-form-urlencoded\r\n";

    m_request_buf += "\r\n";

    m_request_buf += content;

}
