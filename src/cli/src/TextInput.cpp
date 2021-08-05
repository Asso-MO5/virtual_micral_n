#include "TextInput.h"

#include <iostream>
#include <mutex>
#include <string>

namespace
{
    volatile std::string str;
    volatile bool acquisition_running = true;

    std::mutex str_lock;

    void acquire_keyboard()
    {
        while (acquisition_running)
        {
            while (!const_cast<std::string&>(str).empty())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }

            std::string local_str;
            std::getline(std::cin, local_str);

            if (!local_str.empty())
            {
                std::scoped_lock m(str_lock);
                const_cast<std::string&>(str) = local_str;
            }
        }
    }

}

TextInput::~TextInput() { stop(); }

void TextInput::run() { input_thread = std::thread{acquire_keyboard}; }

void TextInput::stop()
{
    acquisition_running = false;
    input_thread.join();
}

bool TextInput::line_received()
{
    std::scoped_lock m(str_lock);
    return !const_cast<std::string&>(str).empty();
}

std::string TextInput::line()
{
    std::scoped_lock m(str_lock);
    return const_cast<std::string&>(str);
}

void TextInput::flush()
{
    std::scoped_lock m(str_lock);
    auto& local_str = const_cast<std::string&>(str);
    local_str.erase(begin(local_str), end(local_str));
}
