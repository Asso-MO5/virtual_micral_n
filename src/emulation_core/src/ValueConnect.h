#ifndef MICRALN_VALUECONNECT_H
#define MICRALN_VALUECONNECT_H

#include "OwnedValue.h"

template<typename ValueType, typename DestValueType>
class MaskedValueConnector
{
public:
    explicit MaskedValueConnector(OwnedValue<ValueType>& source_value, uint64_t mask,
                                  int8_t right_shift, void* owner)
        : source_value{source_value}, mask{mask}, right_shift{right_shift}, owner_id{owner}
    {}

    MaskedValueConnector<ValueType, DestValueType> to(OwnedValue<DestValueType>& destination_value)
    {
        const auto local_owner_id = owner_id;
        const auto local_mask = mask;
        const auto local_shift = right_shift;
        destination_value.request(local_owner_id, Scheduling::counter_type{0});
        source_value.subscribe(
                [&destination_value, local_owner_id, local_mask,
                 local_shift](ValueType, ValueType new_value, Scheduling::counter_type time) {
                    const DestValueType masked_value = (new_value & local_mask) >> local_shift;
                    destination_value.set(masked_value, time, local_owner_id);
                });

        return *this;
    }

private:
    OwnedValue<ValueType>& source_value;
    uint64_t mask;
    int8_t right_shift;
    void* owner_id;
};

template<typename ValueType>
class ValueConnector
{
public:
    explicit ValueConnector(OwnedValue<ValueType>& source_value, void* owner)
        : source_value{source_value}, owner_id{owner}
    {}

    ValueConnector<ValueType> to(OwnedValue<ValueType>& destination_value)
    {
        const auto local_owner_id = owner_id;
        destination_value.request(local_owner_id, Scheduling::counter_type{0});
        source_value.subscribe([&destination_value, local_owner_id](ValueType, ValueType new_value,
                                                                    Scheduling::counter_type time) {
            destination_value.set(new_value, time, local_owner_id);
        });

        return *this;
    }

    template<typename DestValueType>
    MaskedValueConnector<ValueType, DestValueType> mask(uint64_t mask, int8_t right_shift)
    {
        return MaskedValueConnector<ValueType, DestValueType>{source_value, mask, right_shift,
                                                              owner_id};
    }

private:
    OwnedValue<ValueType>& source_value;
    void* owner_id;
};

template<typename ValueType>
ValueConnector<ValueType> connect(OwnedValue<ValueType>& source_value, void* owner)
{
    return ValueConnector{source_value, owner};
}

#endif //MICRALN_VALUECONNECT_H
