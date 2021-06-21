#ifndef MICRALN_IO_UNKNOWN_CONNECTOR_H
#define MICRALN_IO_UNKNOWN_CONNECTOR_H

class IOCard;
class UnknownCard;

class IO_Unknown_Connector
{
public:
    IO_Unknown_Connector(IOCard& io_card, UnknownCard& unknown_card);
};

#endif //MICRALN_IO_UNKNOWN_CONNECTOR_H
