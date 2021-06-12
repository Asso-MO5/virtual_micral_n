#ifndef MICRALN_FILEREADER_H
#define MICRALN_FILEREADER_H

#include <vector>
#include <cstdint>
#include <filesystem>

class FileReader
{
public:
    explicit FileReader(const std::filesystem::path& file_path);

    std::vector<uint8_t> data;
};

#endif //MICRALN_FILEREADER_H
