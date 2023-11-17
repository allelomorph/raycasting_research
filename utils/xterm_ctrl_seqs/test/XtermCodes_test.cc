#include "Xterm.hh"

#include <iostream>

int main() {
    using namespace Xterm;

    std::cout << "256 color mode\n\n";

    std::cout << "System colors:\n";
    std::cout << CtrlSeqs::CharFgColor(Color::Codes::System::Black)  << "::";
    std::cout << CtrlSeqs::CharFgColor(Color::Codes::System::Maroon) << "::";
    std::cout << CtrlSeqs::CharFgColor(Color::Codes::System::Green)  << "::";
    std::cout << CtrlSeqs::CharFgColor(Color::Codes::System::Olive)  << "::";
    std::cout << CtrlSeqs::CharFgColor(Color::Codes::System::Navy)   << "::";
    std::cout << CtrlSeqs::CharFgColor(Color::Codes::System::Purple) << "::";
    std::cout << CtrlSeqs::CharFgColor(Color::Codes::System::Teal)   << "::";
    std::cout << CtrlSeqs::CharFgColor(Color::Codes::System::Silver) << "::";
    std::cout << CtrlSeqs::CharDefaults() << "\n";

    std::cout << CtrlSeqs::CharFgColor(Color::Codes::System::Grey)    << "::";
    std::cout << CtrlSeqs::CharFgColor(Color::Codes::System::Red)     << "::";
    std::cout << CtrlSeqs::CharFgColor(Color::Codes::System::Lime)    << "::";
    std::cout << CtrlSeqs::CharFgColor(Color::Codes::System::Yellow)  << "::";
    std::cout << CtrlSeqs::CharFgColor(Color::Codes::System::Blue)    << "::";
    std::cout << CtrlSeqs::CharFgColor(Color::Codes::System::Fuschia) << "::";
    std::cout << CtrlSeqs::CharFgColor(Color::Codes::System::Aqua)    << "::";
    std::cout << CtrlSeqs::CharFgColor(Color::Codes::System::White)   << "::";
    std::cout << CtrlSeqs::CharDefaults() << "\n\n";

    std::cout << "Color cube, 6x6x6:\n";
    for (uint8_t green { 0 }; green < 6; ++green) {
        for (uint8_t red { 0 }; red < 6; ++red) {
            for (uint8_t blue { 0 }; blue < 6; ++blue) {
                uint8_t color ( 16 + (red * 36) + (green * 6) + blue );
                std::cout << CtrlSeqs::CharFgColor(color) << "::";
            }
            std::cout << CtrlSeqs::CharDefaults() << " ";
        }
        std::cout << "\n";
    }

    std::cout << "Grayscale ramp:\n";
    for (uint32_t color { 232 }; color < 256; ++color) {
        std::cout << CtrlSeqs::CharFgColor(color) << "::";
    }
    std::cout << CtrlSeqs::CharDefaults() << "\n\n";

    std::cout << "System colors:\n";
    std::cout << CtrlSeqs::CharBgColor(Color::Codes::System::Black)  << "::";
    std::cout << CtrlSeqs::CharBgColor(Color::Codes::System::Maroon) << "::";
    std::cout << CtrlSeqs::CharBgColor(Color::Codes::System::Green)  << "::";
    std::cout << CtrlSeqs::CharBgColor(Color::Codes::System::Olive)  << "::";
    std::cout << CtrlSeqs::CharBgColor(Color::Codes::System::Navy)   << "::";
    std::cout << CtrlSeqs::CharBgColor(Color::Codes::System::Purple) << "::";
    std::cout << CtrlSeqs::CharBgColor(Color::Codes::System::Teal)   << "::";
    std::cout << CtrlSeqs::CharBgColor(Color::Codes::System::Silver) << "::";
    std::cout << CtrlSeqs::CharDefaults() << "\n";

    std::cout << CtrlSeqs::CharBgColor(Color::Codes::System::Grey)    << "::";
    std::cout << CtrlSeqs::CharBgColor(Color::Codes::System::Red)     << "::";
    std::cout << CtrlSeqs::CharBgColor(Color::Codes::System::Lime)    << "::";
    std::cout << CtrlSeqs::CharBgColor(Color::Codes::System::Yellow)  << "::";
    std::cout << CtrlSeqs::CharBgColor(Color::Codes::System::Blue)    << "::";
    std::cout << CtrlSeqs::CharBgColor(Color::Codes::System::Fuschia) << "::";
    std::cout << CtrlSeqs::CharBgColor(Color::Codes::System::Aqua)    << "::";
    std::cout << CtrlSeqs::CharBgColor(Color::Codes::System::White)   << "::";
    std::cout << CtrlSeqs::CharDefaults() << "\n\n";

    std::cout << "Color cube, 6x6x6:\n";
    for (uint8_t green { 0 }; green < 6; ++green) {
        for (uint8_t red { 0 }; red < 6; ++red) {
            for (uint8_t blue { 0 }; blue < 6; ++blue) {
                uint8_t color ( 16 + (red * 36) + (green * 6) + blue );
                std::cout << CtrlSeqs::CharBgColor(color) << "::";
            }
            std::cout << CtrlSeqs::CharDefaults() << " ";
        }
        std::cout << "\n";
    }

    std::cout << "Grayscale ramp:\n";
    for (uint32_t color { 232 }; color < 256; ++color) {
        std::cout << CtrlSeqs::CharBgColor(color) << "::";
    }
    std::cout << CtrlSeqs::CharDefaults() << "\n\n";


    std::cout << "Examples for the 3-byte color mode\n\n";

    std::cout << "Color cube\n";
    for (uint32_t green { 0 }; green < 256; green += 51) {
        for (uint32_t red { 0 }; red < 256; red += 51) {
            for (uint32_t blue { 0 }; blue < 256; blue += 51) {
                std::cout << CtrlSeqs::CharFgColor(red, green, blue)   << "::";
            }
            std::cout << CtrlSeqs::CharDefaults() << " ";
        }
        std::cout << "\n";
    }

    std::cout << "Grayscale ramp:\n";
    for (uint32_t gray { 8 }; gray < 256; gray += 10) {
        std::cout << CtrlSeqs::CharFgColor(gray, gray, gray)   << "::";
    }
    std::cout << CtrlSeqs::CharDefaults() << "\n\n";

    std::cout << "Color cube\n";
    for (uint32_t green { 0 }; green < 256; green += 51) {
        for (uint32_t red { 0 }; red < 256; red += 51) {
            for (uint32_t blue { 0 }; blue < 256; blue += 51) {
                std::cout << CtrlSeqs::CharBgColor(red, green, blue)   << "::";
            }
            std::cout << CtrlSeqs::CharDefaults() << " ";
        }
        std::cout << "\n";
    }

    std::cout << "Grayscale ramp:\n";
    for (uint32_t gray { 8 }; gray < 256; gray += 10) {
        std::cout << CtrlSeqs::CharBgColor(gray, gray, gray)   << "::";
    }
    std::cout << CtrlSeqs::CharDefaults() << "\n\n";
}
