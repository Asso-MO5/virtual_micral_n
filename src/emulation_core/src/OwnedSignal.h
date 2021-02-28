#ifndef MICRALN_OWNEDSIGNAL_H
#define MICRALN_OWNEDSIGNAL_H

#include "Edge.h"
#include "State.h"

#include <exception>
#include <functional>
#include <string>

struct signal_error : public std::exception
{
    explicit signal_error(const char* message) : msg(message) {}
    [[nodiscard]] const char* what() const noexcept override { return msg.c_str(); }

protected:
    std::string msg;
};

/* An OwnedSignal is a signal that devices can own by request.
 *
 * Each modification of the signal must prove ownership.
 * OwnedSignal can be subscribed to.
 */
class OwnedSignal
{
public:
    using counter_type = Scheduling::counter_type;
    using callback_type = std::function<void(Edge)>;

    [[nodiscard]] State get_state() const;
    State operator*() const;
    [[nodiscard]] counter_type get_latest_change_time() const;
    void request(void* requested_id);
    void release(void* release_id);
    void set(State new_state, counter_type time, void* set_id);
    void apply(Edge edge, void* set_id);

    void subscribe(const callback_type& callback);

private:
    void* owner_id{};
    State current_state{};
    counter_type latest_change_time{Scheduling::unscheduled()};
    std::vector<callback_type> callbacks;

    void set_and_broadcast(State new_state, counter_type time);
};

#endif //MICRALN_OWNEDSIGNAL_H
