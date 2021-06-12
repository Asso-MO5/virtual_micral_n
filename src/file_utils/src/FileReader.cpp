#include "FileReader.h"

#include <fstream>

FileReader::FileReader(const char* file_path)
{
    std::fstream file;
    file.open(file_path, std::ios::in | std::ios::binary);
    if (file)
    {
        file.seekg(0, std::ios::end);
        auto file_size = file.tellg();
        file.seekg(0, std::ios::beg);

        data.resize(file_size);
        file.read(reinterpret_cast<char*>(&data[0]), file_size);
    }
    else
    {
        throw std::runtime_error("Cannot open file");
    }
}
