#ifndef MICRALN_SIGNALCONNECT_H
#define MICRALN_SIGNALCONNECT_H

class OwnedSignal;

class SignalConnector
{
public:
    explicit SignalConnector(OwnedSignal& source_signal, void* owner);

    SignalConnector to(OwnedSignal& destination_signal);

private:
    OwnedSignal& source_signal;
    void* owner_id;
};

SignalConnector connect(OwnedSignal& source_signal, void* owner);

#endif //MICRALN_SIGNALCONNECT_H
