#ifndef MICRALN_IO_DISKCONTROLLER_CONNECTOR_H
#define MICRALN_IO_DISKCONTROLLER_CONNECTOR_H

class IOCard;
class DiskControllerCard;

class IO_DiskController_Connector
{
public:
    IO_DiskController_Connector(IOCard& io_card, DiskControllerCard& unknown_card);
};

#endif //MICRALN_IO_DISKCONTROLLER_CONNECTOR_H
