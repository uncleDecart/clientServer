#include "filesender.h"
#include <boost/algorithm/string.hpp>


FileSender::FileSender(std::__cxx11::string filepath, std::string id) :
    m_chunk_counter(DEFAULT_CHUNK_COUNTER),
    m_filepath(filepath),
    m_file_parser(filepath, m_chunk_counter),
    m_hostname(DEFAULT_HOSTNAME),
    m_port(DEFAULT_PORT),
    m_id(id)
{
}
void FileSender::set_chunk_count(int value)
{
    m_chunk_counter = value;
    m_file_parser.set_chunk_count(value);
}
void FileSender::set_id(std::__cxx11::string id)
{
    m_id = id;
}
void FileSender::set_hostname(std::__cxx11::string hostname)
{
    m_hostname = hostname;
}

void FileSender::parse_get(const HTTPRequest &request,
                           const HTTPResponse &response,
                           const system::error_code &ec)
{
    if (ec == 0 || ec.category() == asio::error::get_ssl_category())
    {
        // TODO: обработать ошибки SSL
        std::cout << "RECEIVE : " << response.get_status_message() << std::endl;
        if (response.get_status_code() == 200)
        {
            if (response.get_status_message() == " NONE")
                for (int i = 0; i < m_chunk_counter; i++)
                    send_file(m_file_parser.get_compressed_chunk(i));
            else
            {
                std::vector<std::string> resp;
                boost::split(resp, response.get_status_message(), boost::is_any_of(" "));
                int chunk_count = std::stoi(resp.back());
                resp.pop_back();

                std::vector<std::string> missing_files = m_file_parser.get_missing_files(chunk_count, resp);

                for (auto it = missing_files.begin(); it < missing_files.end(); it++)
                    send_file(*it);
            }
        }
        if (response.get_status_code() == 403)
        {
            std::cout << "Request Forbiden! " << response.get_status_message() << std::endl;
        }
    }
    else if (ec == asio::error::operation_aborted)
    {
        std::cout << "Request #" << request.get_id()
            << " has been cancelled by the user."
            << std::endl;
    }
    else
    {
        std::cout << "Request #" << request.get_id()
            << " failed! Error code = " << ec.value()
            << ". Error message = " << ec.message()
            << std::endl;
    }
    return;


}

void FileSender::parse_post(const HTTPRequest &request,
                           const HTTPResponse &response,
                           const system::error_code &ec)
{
    if (ec == 0)
    {
        std::cout << "Request #" << request.get_id()
            << " has completed. Response: "
            << response.get_status_message();

    }
    else if (ec == asio::error::operation_aborted)
    {
        std::cout << "Request #" << request.get_id()
            << " has been cancelled by the user."
            << std::endl;
    }
    else
    {
        std::cout << "Request #" << request.get_id()
            << " failed! Error code = " << ec.value()
            << ". Error message = " << ec.message()
            << std::endl;

    }

    return;
}

int FileSender::send_file(std::string filepath)
{
    try {
        using namespace std::placeholders;
        HTTPClient client;

        std::unique_ptr<HTTPPOSTRequest> request_one =
                client.create_post_request(1);

        request_one->set_host(m_hostname);
        request_one->set_filepath(filepath);
        request_one->set_uri(m_id + "/" + filepath);
        request_one->set_port(m_port);

        QObject::connect(request_one.get(), SIGNAL(got_response(HTTPRequest,HTTPResponse,system::error_code)),
                         this, SLOT(parse_post(HTTPRequest,HTTPResponse,system::error_code)), Qt::DirectConnection);

        request_one->execute();

        client.close();
    }
    catch (system::system_error &e)
    {
        std::cout << "Error occured! Error code = " << e.code()
            << ". Message: " << e.what() << std::endl;

        return e.code().value();
    }

    return 0;
}
int FileSender::start_session()
{
    try {
        HTTPClient client;

        std::unique_ptr<HTTPGETRequest> request_one =
                client.create_get_request(1);

        request_one->set_host(m_hostname);
        request_one->set_uri(m_id + "/any?" + std::to_string(m_chunk_counter));
        request_one->set_port(m_port);
        QObject::connect(request_one.get(), SIGNAL(got_response(HTTPRequest,HTTPResponse,system::error_code)),
                         this, SLOT(parse_get(HTTPRequest,HTTPResponse,system::error_code)), Qt::DirectConnection);

        request_one->execute();

        client.close();
    }
    catch (system::system_error &e)
    {
        std::cout << "Error occured! Error code = " << e.code()
            << ". Message: " << e.what() << std::endl;

        return e.code().value();
    }

}
