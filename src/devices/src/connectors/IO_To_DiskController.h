#ifndef MICRALN_IO_TO_DISKCONTROLLER_H
#define MICRALN_IO_TO_DISKCONTROLLER_H

class IOCard;
class DiskControllerCard;

namespace Connectors
{
    class IO_To_DiskController
    {
    public:
        IO_To_DiskController(IOCard& io_card, DiskControllerCard& unknown_card);
    };
}

#endif //MICRALN_IO_TO_DISKCONTROLLER_H
