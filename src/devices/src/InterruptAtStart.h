
#ifndef MICRALN_INTERRUPTATSTART_H
#define MICRALN_INTERRUPTATSTART_H

#include <cstdint>
#include <memory>

class CPU8008;
class Edge;

class InterruptAtStart
{
public:
    explicit InterruptAtStart(std::shared_ptr<CPU8008> cpu);

    void signal_phase_1(const Edge& edge);
    void signal_vdd(const Edge& edge);

private:
    std::shared_ptr<CPU8008> cpu;
    uint64_t counter{};
};

#endif //MICRALN_INTERRUPTATSTART_H
