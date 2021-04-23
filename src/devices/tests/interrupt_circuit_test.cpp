#include "InterruptCircuit.h"

#include <emulation_core/src/OwnedSignal.h>

#include <gmock/gmock.h>

using namespace testing;

TEST(InterruptCircuit, can_trigger_the_request_signal)
{
    OwnedSignal request_signal;
    OwnedSignal ack_signal;

    auto circuit = InterruptCircuit{request_signal, ack_signal};

    circuit.trigger(Scheduling::counter_type{100});

    ASSERT_THAT(request_signal.get_state(), Eq(State::HIGH));
    ASSERT_THAT(request_signal.get_latest_change_time(), Eq(Scheduling::counter_type{100}));
}

TEST(InterruptCircuit, triggering_does_not_touch_the_ack_signal)
{
    OwnedSignal request_signal;
    OwnedSignal ack_signal;

    auto circuit = InterruptCircuit{request_signal, ack_signal};

    circuit.trigger(Scheduling::counter_type{100});

    ASSERT_THAT(ack_signal.get_state(), Eq(State::LOW));
    ASSERT_THAT(ack_signal.get_latest_change_time(), Eq(Scheduling::counter_type{0}));
}

TEST(InterruptCircuit, resets_the_request_when_ack_is_asserted)
{
    OwnedSignal request_signal;
    OwnedSignal ack_signal;

    auto circuit = InterruptCircuit{request_signal, ack_signal};

    circuit.trigger(Scheduling::counter_type{100});

    ack_signal.request(this);
    ack_signal.set(State::HIGH, Scheduling::counter_type{120}, this);
    ack_signal.release(this);

    ASSERT_THAT(request_signal.get_state(), Eq(State::LOW));
    ASSERT_THAT(request_signal.get_latest_change_time(), Ge(Scheduling::counter_type{120}));
}

TEST(InterruptCircuit, the_reset_is_insensible_to_ack_variations)
{
    OwnedSignal request_signal;
    OwnedSignal ack_signal;

    auto circuit = InterruptCircuit{request_signal, ack_signal};

    circuit.trigger(Scheduling::counter_type{100});

    ack_signal.request(this);
    ack_signal.set(State::HIGH, Scheduling::counter_type{120}, this);
    ack_signal.set(State::LOW, Scheduling::counter_type{150}, this);
    ack_signal.set(State::HIGH, Scheduling::counter_type{180}, this);
    ack_signal.set(State::LOW, Scheduling::counter_type{200}, this);
    ack_signal.release(this);

    ASSERT_THAT(request_signal.get_state(), Eq(State::LOW));
    ASSERT_THAT(request_signal.get_latest_change_time(), Lt(Scheduling::counter_type{149}));
}
