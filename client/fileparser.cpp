#include "fileparser.h"
#include <fstream>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <sstream>

/*!
 * \brief FileParser::FileParser
 * Устанавливает размер частей файла
 *
 * \param filepath путь к файлу
 * \param chunk_count количество частей, на которые разбивается файл
 */
FileParser::FileParser(std::string filepath, uint chunk_count) :
    m_filepath(filepath)
{
    m_chunk_count = chunk_count;
    std::ifstream ifs(filepath, std::ifstream::ate | std::ifstream::binary);
    m_size = ifs.tellg();
    set_chunk_count(chunk_count);
}

/*!
 * \brief FileParser::set_chunk_count
 * Устанавливает, на сколько частей должен быть разбит исходный файл
 * \param chunk_count количество частей
 */
void FileParser::set_chunk_count(uint chunk_count)
{
    m_chunk_count = chunk_count;
    m_chunk_size = m_size/m_chunk_count + ( (m_size % m_chunk_count)?1U:0U );
    m_last_chunk_size = m_size - m_chunk_size*(m_chunk_count - 1);
}

/*!
 * \brief FileParser::existing_files
 * возвращает имена файлов с расширением '.part',
 * которые содержаться в директории.
 * \param root директория
 * \return вектор файлов с расширением '.part'
 */
std::vector<boost::filesystem::path> FileParser::existing_files(const boost::filesystem::path& root)
{
    std::string ext = ".part";

    namespace fs = ::boost::filesystem;
    std::vector<fs::path> ret;

    if(!fs::exists(root) || !fs::is_directory(root))
        return ret;

    fs::recursive_directory_iterator it(root);
    fs::recursive_directory_iterator endit;

    while(it != endit)
    {
        if(fs::is_regular_file(*it) && it->path().extension() == ext)
            ret.push_back(it->path().filename());
        ++it;
    }
    return ret;
}
/*!
 * \brief FileParser::get_missing_files
 * Создаёт (при необходимости) и возвращает список файлов,
 * сформированных исходя из количества chunk_count,
 * которых не достаёт в векторе file_sended.
 * \details Вызывается для обработки ситуации внезапного обрыва и
 * выключения системы.
 * \param chunk_count изначальное количество передаваемых файлов
 * \param file_sended список файлов, которые были успешно переданы на сервер.
 * \return список созданных (при необходимости) файлов
 */
std::vector<std::string> FileParser::get_missing_files(uint chunk_count,
                                                       std::vector<std::string> file_sended)
{
    std::vector<std::string> whole;
    for (uint i = 0; i < chunk_count; i++)
    {
        std::cout << get_filepath(i) << std::endl;
        whole.push_back(get_filepath(i));
    }
    for (auto it = file_sended.begin(); it < file_sended.end(); it++)
        std::cout << *it << std::endl;

    std::vector<std::string> ans;
    std::set_difference(whole.begin(), whole.end(),
                        file_sended.begin(), file_sended.end(),
                        std::back_inserter(ans));
    for (auto it = ans.begin(); it < ans.end(); it++)
        get_compressed_chunk(get_chunk_num(*it));
    return ans;
}
/*!
 * \brief FileParser::get_compressed_chunk
 * Создаёт (при необходимости) сжатую часть исходного файла
 * \param chunk_num номер части исходного файла
 * \return путь к сжатой части файла
 */
std::string FileParser::get_compressed_chunk(uint chunk_num)
{

    std::string filepath = get_filepath(chunk_num);
    if(!boost::filesystem::exists(filepath))
    {
        boost::iostreams::mapped_file_source file(m_filepath);
        auto cz = (chunk_num != m_chunk_count - 1)? m_chunk_size : m_last_chunk_size;
        std::string fileContent(file.data() + m_chunk_size*chunk_num, cz);
        std::ofstream ofs(filepath);
        ofs << compress(fileContent);
        ofs.close();
        file.close();
    }
    return filepath;
}

void FileParser::remove_chunk(uint chunk_num)
{
    std::remove(get_filepath(chunk_num).c_str());
}

std::string FileParser::get_filepath(uint chunk_num)
{
    return "a" + std::to_string(chunk_num) + ".part";
}
int FileParser::get_chunk_num(std::string filepath)
{
    static const boost::regex re("\\d+");

    boost::sregex_iterator it(filepath.begin(), filepath.end(), re);
    boost::sregex_iterator end;

    if (it != end)
    {
        return boost::lexical_cast<int>(it->str());
    }
    return -1;
}

uint FileParser::get_chunk_count()
{
    return m_chunk_count;
}

std::string FileParser::compress(const std::string& data)
{

    std::stringstream compressed;
    std::stringstream origin(data);

    boost::iostreams::filtering_streambuf<boost::iostreams::input> out;
    out.push(boost::iostreams::zlib_compressor());
    out.push(origin);
    boost::iostreams::copy(out, compressed);

    return compressed.str();
}
