#ifndef MICRALN_PLOTSTRUCTURES_H
#define MICRALN_PLOTSTRUCTURES_H

template<typename ValueType>
struct PlotValues
{
    const double* x_series = nullptr;
    const ValueType* y_series = nullptr;
    int count{};
    int offset = 0;
};

struct PlotScale
{
    double x_min{};
    double x_max{};
    double y_min{};
    double y_max{};
    bool x_scaled = false;
};

#endif //MICRALN_PLOTSTRUCTURES_H
