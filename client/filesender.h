#ifndef FILESENDER_H
#define FILESENDER_H

#include <QObject>

#include "fileparser.h"
#include "httpclient.h"

class FileSender : public QObject
{
    Q_OBJECT

    const int DEFAULT_CHUNK_COUNTER = 4;
    const int DEFAULT_PORT = 3333;
    const std::string DEFAULT_HOSTNAME = "home.eee";
public:
    FileSender(std::string filepath, std::string id);
    void set_chunk_count(int value);
    void set_id(std::string id);
    void set_hostname(std::string hostname);
    int start_session();

private slots:
    void parse_get(const HTTPRequest& request,
                   const HTTPResponse& response,
                   const system::error_code& ec);
    void parse_post(const HTTPRequest& request,
                    const HTTPResponse& response,
                    const system::error_code& ec);

private:
    int send_file(std::string filepath);

    std::vector<std::unique_ptr<HTTPRequest>> m_requests;


    int m_chunk_counter;
    std::string m_filepath;
    FileParser m_file_parser;
    std::string m_hostname;
    int m_port;
    std::string m_id;

};

#endif // FILESENDER_H
