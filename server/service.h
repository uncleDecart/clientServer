#ifndef SERVICE_H
#define SERVICE_H

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include <fstream>
#include <atomic>
#include <thread>
#include <iostream>

using namespace boost;

class Service
{
    static const std::map<unsigned int, std::string>
        http_status_table;

public:
    Service(std::shared_ptr<asio::ip::tcp::socket> sock) :
        m_sock(sock),
        m_request(4096),
        m_response_status_code(200),
        m_resource_size_bytes(0)
    {}

    void start_handling();

private:
    void on_request_line_received(const boost::system::error_code& ec,
                                  std::size_t bytes_transferred);

    void on_headers_received(const boost::system::error_code& ec,
                             std::size_t bytes_transfered);

    void process_request();

    void send_response();

    void on_response_sent(const boost::system::error_code& ec,
                          std::size_t bytes_transfered);
    void on_finish();

    std::string ProcessRequest(asio::streambuf& request);
    std::string handlePOSTRequest(asio::streambuf& request);
    std::string handleGETRequest(asio::streambuf& request);

private:
    std::shared_ptr<asio::ip::tcp::socket> m_sock;
    asio::streambuf m_request;
    std::map<std::string, std::string> m_request_headers;
    std::string m_requested_resource;

    std::unique_ptr<char[]> m_resource_buffer;
    unsigned int m_response_status_code;
    std::size_t m_resource_size_bytes;
    std::string m_response_headers;
    std::string m_response_status_line;
};

#endif // SERVICE_H
