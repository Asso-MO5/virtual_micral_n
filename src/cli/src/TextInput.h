#ifndef MICRALN_TEXTINPUT_H
#define MICRALN_TEXTINPUT_H

#include <thread>

class TextInput
{
public:
    ~TextInput();
    void run();

    bool line_received();
    std::string line();
    void flush();

private:
    void stop();

    std::thread input_thread;
};

#endif //MICRALN_TEXTINPUT_H
