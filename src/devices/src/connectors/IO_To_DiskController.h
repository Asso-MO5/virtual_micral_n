#ifndef MICRALN_IO_TO_DISKCONTROLLER_H
#define MICRALN_IO_TO_DISKCONTROLLER_H

class IOCard;
class DiskControllerCard;

namespace Connectors
{
    class IO_To_DiskController
    {
    public:
        IO_To_DiskController(IOCard& io_card, DiskControllerCard& disk_controller);
        
    private:
        void from_io_to_controller(IOCard& io_card, DiskControllerCard& disk_controller);
        void from_controller_to_io(IOCard& io_card, DiskControllerCard& disk_controller);
    };
}

#endif //MICRALN_IO_TO_DISKCONTROLLER_H
