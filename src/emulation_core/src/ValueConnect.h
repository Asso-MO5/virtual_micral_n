#ifndef MICRALN_VALUECONNECT_H
#define MICRALN_VALUECONNECT_H

#include "OwnedValue.h"

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
        source_value.subscribe([&destination_value, local_owner_id](uint8_t, uint8_t new_value,
                                                                    Scheduling::counter_type time) {
            destination_value.set(new_value, time, local_owner_id);
        });

        return *this;
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
