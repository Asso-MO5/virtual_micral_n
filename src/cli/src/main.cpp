#include "TextInput.h"
#include <devices/src/AutomaticStart.h>
#include <devices/src/CPU8008.h>
#include <devices/src/Pluribus.h>
#include <devices/src/ProcessorCard.h>
#include <emulator/src/Simulator.h>
#include <emulator/src/VirtualTTY.h>
#include <iostream>
#include <loguru.hpp>
#include <stdexcept>

namespace
{
    // Somehow duplicated from PanelTTY
    std::string representation(char c, bool raw_output)
    {
        if (!raw_output)
        {
            c &= (c == -1) ? -1 : 0x7f;
        }

        if (c == 13)
        {
            return {"\n"};
        }
        if (c == 10)
        {
            return {};
        }

        if (c == 0x11)
        {
            return {"\n==(DC1)==\n"};
        }
        if (c == 0x12 && !raw_output)
        {
            return {"\n==(DC2)==\n"};
        }
        if (c == 0x14 && raw_output)
        {
            return {"\n==(DC4)==\n"};
        }

        if (raw_output)
        {
            return "¬";
        }

        return {c};
    }
}

int main(int argc, char** argv)
{
#ifdef NDEBUG
    loguru::g_stderr_verbosity = loguru::Verbosity_WARNING;
#else
    loguru::g_stderr_verbosity = loguru::Verbosity_INFO;
#endif
    loguru::set_fatal_handler([](const loguru::Message& message) {
        throw std::runtime_error(std::string(message.prefix) + message.message);
    });

    loguru::init(argc, argv);

    LOG_F(INFO, "Creates the simulator");
    Simulator simulator{BANNER_MO5};

    LOG_F(INFO, "Running ...");

    // Some duplication in the TTY
    std::size_t previous_content_size = 0;
    uint8_t ff_count = 0;
    std::string chars_to_send;
    const uint32_t INITIAL_COUNTER = 100000;
    uint32_t counter = INITIAL_COUNTER;

    TextInput input_task;
    input_task.run();

    volatile bool running = true; // Volatile just to avoid the warning at the moment.
    while (running)
    {
        simulator.step(16.f, STEP_ONE_CLOCK);

        {
            auto tty = simulator.get_virtual_tty();

            const auto& tty_content = tty.content();

            if (tty_content.size() > previous_content_size)
            {
                const auto new_content = tty_content.substr(previous_content_size);
                previous_content_size = tty_content.size();

                for (auto c : new_content)
                {
                    std::cout << representation(c, ff_count > 0);
                }

                std::cout.flush();
            }

            if (input_task.line_received())
            {
                auto local_str = input_task.line();
                chars_to_send.append(local_str);
                chars_to_send.push_back(0x0d);
                input_task.flush();
            }

            counter -= 1;

            if (counter == 0)
            {
                counter = INITIAL_COUNTER;
            }

            if (!chars_to_send.empty() && counter == INITIAL_COUNTER)
            {
                const auto next_char = chars_to_send.front();
                chars_to_send.erase(std::begin(chars_to_send));
                tty.emit_char(static_cast<char>(next_char & 0xff));
            }
        }
    }

    LOG_F(INFO, "Finished");
    LOG_F(INFO, "Clock ran for %lu nanoseconds", simulator.get_scheduler().get_counter().get());

    return 0;
}
