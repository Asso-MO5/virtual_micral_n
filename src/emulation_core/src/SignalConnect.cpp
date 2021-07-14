#include "SignalConnect.h"

#include "OwnedSignal.h"

SignalConnector::SignalConnector(OwnedSignal& source_signal, void* owner)
    : source_signal{source_signal}, owner_id{owner}
{}

SignalConnector SignalConnector::to(OwnedSignal& destination_signal)
{
    const auto local_owner_id = owner_id;
    destination_signal.request(local_owner_id);
    source_signal.subscribe([&destination_signal, local_owner_id](Edge edge) {
      destination_signal.apply(edge, local_owner_id);
    });

    return *this;
}

SignalConnector connect(OwnedSignal& source_signal, void* owner)
{
    return SignalConnector{source_signal, owner};
}
