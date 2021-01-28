#ifndef MICRALN_FREQUENCY_H
#define MICRALN_FREQUENCY_H

#include <cstdint>

class Frequency
{
public:
    explicit Frequency(uint64_t frequency) : frequency(frequency) {}
    [[nodiscard]] uint64_t get_period_as_ns() const { return 1000 * 1000 * 1000 / frequency; };

private:
    uint64_t frequency;
};

inline Frequency operator"" _hz(unsigned long long int freq) { return Frequency{freq}; }

#endif //MICRALN_FREQUENCY_H
