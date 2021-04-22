
#ifndef MICRALN_AUTOMATICSTART_H
#define MICRALN_AUTOMATICSTART_H

#include <cstdint>
#include <memory>

class CPU8008;
class Edge;

class AutomaticStart
{
public:
    explicit AutomaticStart(std::shared_ptr<CPU8008> cpu);

    void on_phase_1(const Edge& edge);
    void on_vdd(const Edge& edge);
    [[nodiscard]] bool is_ready() const;

private:
    std::shared_ptr<CPU8008> cpu;
    uint64_t counter{};
    bool ready{};
};

#endif //MICRALN_AUTOMATICSTART_H
