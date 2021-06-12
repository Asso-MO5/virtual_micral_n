#ifndef MICRALN_TOHEX_H
#define MICRALN_TOHEX_H

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

namespace utils
{
    template<typename IntType, std::size_t padding>
    std::string to_hex(IntType value)
    {
        std::stringstream hex_value;
        hex_value << "$" << std::setfill('0') << std::setw(padding) << std::hex
                  << static_cast<int>(value);

        return hex_value.str();
    }
}

#endif //MICRALN_TOHEX_H
