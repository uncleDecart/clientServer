#include "service.h"

#include <boost/range/iterator_range.hpp>

const std::map<unsigned int, std::string> Service::http_status_table =
{
    { 200, "200 OK" },
    { 404, "404 Not Found" },
    { 413, "413 Request Entity Too Large" },
    { 500, "500 Server Error" },
    { 501, "501 Not Implemented" },
    { 505, "505 HTTP Version Not Supported"}
};

void Service::start_handling()
{
    asio::async_read_until(*m_sock.get(),
                           m_request,
                           "\r\n",
                           [this](
                           const boost::system::error_code& ec,
                           std::size_t bytes_transferred)
    {
        on_request_line_received(ec, bytes_transferred);
    });
}

void Service::on_request_line_received(
        const boost::system::error_code& ec,
        std::size_t bytes_transferred)
{
    if (ec != nullptr) {
        std::cout << "Error occured! Error code = "
                  << ec.value()
                  << ". Message: " << ec.message();

        if (ec == asio::error::not_found) {
            m_response_status_code = 413;
            send_response();

            return;
        }
        else {
            on_finish();
            return;
        }
    }

    std::string request_line;
    std::istream request_stream(&m_request);
    std::getline(request_stream, request_line, '\r');
    request_stream.get(); // remove \n
    std::cout << "Getting request : " << request_line << std::endl;

    std::string request_method;
    std::istringstream request_line_stream(request_line);
    request_line_stream >> request_method;

    if (request_method.compare("GET")  != 0 &&
        request_method.compare("POST") != 0) {
        m_response_status_code = 501;
        send_response();
        return;
    }

    request_line_stream >> m_requested_resource;

    std::string request_http_version;
    request_line_stream >> request_http_version;
    std::cout << "HTTP version:" << request_http_version << std::endl;

    if (request_http_version.compare("HTTP/1.1") != 0) {
        m_response_status_code = 505;
        send_response();

        return;
    }

    asio::async_read_until(*m_sock.get(),
                           m_request,
                           "\r\n\r\n",
                           [this](
                           const boost::system::error_code& ec,
                           std::size_t bytes_transferred)
    {
        on_headers_received(ec, bytes_transferred);
    });

    return;
}

void Service::on_headers_received(const boost::system::error_code& ec,
                                  std::size_t bytes_transferred)
{
    if (ec != nullptr) {
        std::cout << "Error occured! Error code = "
                  << ec.value()
                  << ". Message: " << ec.message();

        if (ec == asio::error::not_found) {
            m_response_status_code = 413;
            send_response();

            return;
        }
        else {
            on_finish();
            return;
        }
    }

    std::istream request_stream(&m_request);
    std::string header_name, header_value;

    while (!request_stream.eof()) {
        std::getline(request_stream, header_name, ':');
        if (!request_stream.eof()) {
            std::getline(request_stream,
                         header_value, '\r');
            request_stream.get(); // \n
            m_request_headers[header_name] = header_value;
        }
    }

    process_request();
    send_response();

    return;
}

void Service::process_request()
{
    //std::cout << "incoming resource : " << resource_file_path << std::endl;

    auto idx = m_requested_resource.find_first_of("0123456789");
    std::string resource_file_path = "/home/pabramov" + m_requested_resource.substr(idx);


    if (!boost::filesystem::exists(resource_file_path)) {
        m_response_status_code = 404;
        return;
    }

    //std::string ans;
    namespace fs = boost::filesystem;
    for (auto& entry : boost::make_iterator_range(
             fs::directory_iterator(fs::path(resource_file_path))))
        if (fs::extension(entry.path()) == "part")
            m_response_headers += entry.path().string() + " ";

    std::ifstream resource_fstream(resource_file_path,
                                   std::ifstream::binary);

    if (!resource_fstream.is_open()) {
        m_response_status_code = 500;
        return;
    }

    resource_fstream.seekg(0, std::ifstream::end);
    m_resource_size_bytes = static_cast<std::size_t>(
                resource_fstream.tellg());

    m_resource_buffer.reset(new char[m_resource_size_bytes]);

    resource_fstream.seekg(std::ifstream::beg);
    resource_fstream.read(m_resource_buffer.get(),
                          m_resource_size_bytes);

    m_response_headers += std::string("content-length: ") +
                          std::to_string(m_resource_size_bytes) +
                          "\r\n";
}

void Service::send_response()
{
    m_sock->shutdown(asio::ip::tcp::socket::shutdown_receive);

    auto status_line = http_status_table.at(m_response_status_code);

    m_response_status_line = std::string("HTTP/1.1 ") +
                             status_line +
                             "\r\n";

    m_response_headers += "\r\n";

    std::vector<asio::const_buffer> response_buffers;
    response_buffers.push_back(asio::buffer(m_response_status_line));

    if (m_resource_size_bytes > 0) {
        response_buffers.push_back(
                    asio::buffer(m_resource_buffer.get(),
                                 m_resource_size_bytes));
    }

    asio::async_write(*m_sock.get(),
                      response_buffers,
                      [this](const boost::system::error_code& ec,
                             std::size_t bytes_trasferred)
    {
        on_response_sent(ec, bytes_trasferred);
    });
}

void Service::on_response_sent(const boost::system::error_code& ec,
                               std::size_t bytes_tranfered)
{
    if (ec != nullptr) {
        std::cout << "Error occured! Error code = "
                  << ec.value()
                  << ". Message: " << ec.message();
    }

    m_sock->shutdown(asio::ip::tcp::socket::shutdown_both);

    on_finish();
}

void Service::on_finish()
{
    delete this;
}
