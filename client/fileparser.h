#ifndef FILEPARSER_H
#define FILEPARSER_H

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <string>

/*!
 * \brief Класс FileParser, разбивает файл на чанки
 *        и сжимает их.
 */
class FileParser
{
public:
    FileParser(std::string filepath, uint chunk_count);
    std::string get_compressed_chunk(uint chunk_num);

    void set_chunk_count(uint chunk_count);
    static void remove_chunk(uint chunk_num);
    static std::vector<boost::filesystem::path> existing_files(const boost::filesystem::path& root);

    std::vector<std::string> get_missing_files(uint chunk_count, std::vector<std::string> file_sended);

    uint get_chunk_count();

private:

    std::string compress(const std::string& data);

    static std::string get_filepath(uint chunk_num);
    static int get_chunk_num(std::string filepath);

    uint64_t m_size;
    std::string m_filepath;
    uint m_chunk_count;
    std::fstream::pos_type m_filesize;
    std::fstream::pos_type m_chunk_size;
    std::fstream::pos_type m_last_chunk_size;
};

#endif // FILEPARSER_H
