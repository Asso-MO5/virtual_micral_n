#include "OwnedSignal.h"

OwnedSignal::OwnedSignal(State initial_state) : current_state{initial_state} {}

State OwnedSignal::get_state() const { return current_state; }
State OwnedSignal::operator*() const { return current_state; }
OwnedSignal::counter_type OwnedSignal::get_latest_change_time() const { return latest_change_time; }

void OwnedSignal::request(void* requested_id)
{
    if (owner_id != nullptr && requested_id != owner_id)
    {
        throw signal_error{"Cannot request, the signal is already owned."};
    }
    owner_id = requested_id;
}

void OwnedSignal::release(void* release_id)
{
    if (owner_id != nullptr && release_id != owner_id)
    {
        throw signal_error{"Cannot release, not owned."};
    }
    owner_id = nullptr;
}

void OwnedSignal::set(State new_state, OwnedSignal::counter_type time, void* set_id)
{
    if (owner_id != set_id)
    {
        throw signal_error{"Cannot set signal when not owned."};
    }

    set_and_broadcast(new_state, time);
}

void OwnedSignal::apply(Edge edge, void* set_id) { set(edge.apply(), edge.time(), set_id); }

void OwnedSignal::subscribe(const OwnedSignal::callback_type& callback)
{
    callbacks.push_back(callback);
}

void OwnedSignal::set_and_broadcast(State new_state, OwnedSignal::counter_type time)
{
    auto previous_state = current_state;

    if (previous_state != new_state)
    {
        current_state = new_state;
        latest_change_time = time;

        for (auto& callback : callbacks)
        {
            callback(Edge(previous_state, new_state, time));
        }
    }
}
