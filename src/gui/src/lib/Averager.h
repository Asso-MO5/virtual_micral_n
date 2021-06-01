#ifndef MICRALN_AVERAGER_H
#define MICRALN_AVERAGER_H

#include <array>
#include <cstdint>
#include <numeric>

template<typename ValueType, std::size_t size>
class Averager
{
public:
    void push(ValueType new_value)
    {
        values[next_index] = new_value;
        next_index = (next_index + 1) % size;
        actual_count = std::min(size, actual_count + 1);
    }
    ValueType average() {
        auto sum = std::accumulate(values.data(), values.data() + actual_count, ValueType{});
        return sum / actual_count; }

private:
    std::array<ValueType, size> values{};
    std::size_t next_index{};
    std::size_t actual_count{};
};

#endif //MICRALN_AVERAGER_H
