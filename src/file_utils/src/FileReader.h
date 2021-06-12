#ifndef MICRALN_FILEREADER_H
#define MICRALN_FILEREADER_H

#include <vector>
#include <cstdint>

class FileReader
{
public:
    explicit FileReader(const char* file_path);

    std::vector<uint8_t> data;
};

#endif //MICRALN_FILEREADER_H
