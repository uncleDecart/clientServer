#include "httpclient.h"

std::unique_ptr<HTTPPOSTRequest>
    HTTPClient::create_post_request(unsigned int id)
{
    return std::unique_ptr<HTTPPOSTRequest>(new HTTPPOSTRequest(m_ios, id));
}
std::unique_ptr<HTTPGETRequest>
    HTTPClient::create_get_request(unsigned int id)
{
    return std::unique_ptr<HTTPGETRequest>(new HTTPGETRequest(m_ios, id));
}

void HTTPClient::close() {
    // Destroy the work object.
    m_work.reset(NULL);

    // Waiting for the I/O thread to exit.
    m_thread->join();
}
