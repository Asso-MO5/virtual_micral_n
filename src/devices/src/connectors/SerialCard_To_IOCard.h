#ifndef MICRALN_SERIALCARD_TO_IOCARD_H
#define MICRALN_SERIALCARD_TO_IOCARD_H

class IOCard;
class SerialCard;

namespace Connectors
{
    class SerialCard_To_IOCard
    {
    public:
        SerialCard_To_IOCard(SerialCard& serial_card, IOCard& io_card);
    };

}

#endif //MICRALN_SERIALCARD_TO_IOCARD_H
