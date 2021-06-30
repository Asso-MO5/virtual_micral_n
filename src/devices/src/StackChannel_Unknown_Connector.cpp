#include "StackChannel_Unknown_Connector.h"

#include "StackChannelCard.h"
#include "UnknownCard.h"

StackChannel_Unknown_Connector::StackChannel_Unknown_Connector(StackChannelCard& stack_channel_card,
                                                               UnknownCard& unknown_card)
{
    // From UnknownCard to StackChannel
    stack_channel_card.data_transfer.request(this);
    stack_channel_card.input_data.request(this, Scheduling::counter_type{0});

    unknown_card.output_data.subscribe(
            [&stack_channel_card, this](uint8_t, uint8_t new_value, Scheduling::counter_type time) {
                stack_channel_card.input_data.set(new_value, time, this);
            });

    unknown_card.available_data.subscribe([&stack_channel_card, this](Edge edge) {
        stack_channel_card.data_transfer.apply(edge, this);
    });
}
