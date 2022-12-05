#include "XtermCodes.hh"

#include <iostream>

int main() {
    using XtermCodes::SysColor;

    std::cout << "256 color mode\n\n";

    std::cout << "System colors:\n";
    std::cout << XtermCodes::charFgColor(SysColor::Black)   << "::";
    std::cout << XtermCodes::charFgColor(SysColor::Red)     << "::";
    std::cout << XtermCodes::charFgColor(SysColor::Green)   << "::";
    std::cout << XtermCodes::charFgColor(SysColor::Yellow)  << "::";
    std::cout << XtermCodes::charFgColor(SysColor::Blue)    << "::";
    std::cout << XtermCodes::charFgColor(SysColor::Magenta) << "::";
    std::cout << XtermCodes::charFgColor(SysColor::Cyan)    << "::";
    std::cout << XtermCodes::charFgColor(SysColor::White)   << "::";
    std::cout << XtermCodes::charDefaults() << "\n";

    std::cout << XtermCodes::charFgColor(SysColor::LightBlack)   << "::";
    std::cout << XtermCodes::charFgColor(SysColor::LightRed)     << "::";
    std::cout << XtermCodes::charFgColor(SysColor::LightGreen)   << "::";
    std::cout << XtermCodes::charFgColor(SysColor::LightYellow)  << "::";
    std::cout << XtermCodes::charFgColor(SysColor::LightBlue)    << "::";
    std::cout << XtermCodes::charFgColor(SysColor::LightMagenta) << "::";
    std::cout << XtermCodes::charFgColor(SysColor::LightCyan)    << "::";
    std::cout << XtermCodes::charFgColor(SysColor::LightWhite)   << "::";
    std::cout << XtermCodes::charDefaults() << "\n\n";

    std::cout << "Color cube, 6x6x6:\n";
    for (uint8_t green { 0 }; green < 6; ++green) {
        for (uint8_t red { 0 }; red < 6; ++red) {
            for (uint8_t blue { 0 }; blue < 6; ++blue) {
                uint8_t color ( 16 + (red * 36) + (green * 6) + blue );
                std::cout << XtermCodes::charFgColor(color) << "::";
            }
            std::cout << XtermCodes::charDefaults() << " ";
        }
        std::cout << "\n";
    }

    std::cout << "Grayscale ramp:\n";
    for (uint32_t color { 232 }; color < 256; ++color) {
        std::cout << XtermCodes::charFgColor(color) << "::";
    }
    std::cout << XtermCodes::charDefaults() << "\n\n";

    std::cout << "System colors:\n";
    std::cout << XtermCodes::charBgColor(SysColor::Black)   << "::";
    std::cout << XtermCodes::charBgColor(SysColor::Red)     << "::";
    std::cout << XtermCodes::charBgColor(SysColor::Green)   << "::";
    std::cout << XtermCodes::charBgColor(SysColor::Yellow)  << "::";
    std::cout << XtermCodes::charBgColor(SysColor::Blue)    << "::";
    std::cout << XtermCodes::charBgColor(SysColor::Magenta) << "::";
    std::cout << XtermCodes::charBgColor(SysColor::Cyan)    << "::";
    std::cout << XtermCodes::charBgColor(SysColor::White)   << "::";
    std::cout << XtermCodes::charDefaults() << "\n";

    std::cout << XtermCodes::charBgColor(SysColor::LightBlack)   << "::";
    std::cout << XtermCodes::charBgColor(SysColor::LightRed)     << "::";
    std::cout << XtermCodes::charBgColor(SysColor::LightGreen)   << "::";
    std::cout << XtermCodes::charBgColor(SysColor::LightYellow)  << "::";
    std::cout << XtermCodes::charBgColor(SysColor::LightBlue)    << "::";
    std::cout << XtermCodes::charBgColor(SysColor::LightMagenta) << "::";
    std::cout << XtermCodes::charBgColor(SysColor::LightCyan)    << "::";
    std::cout << XtermCodes::charBgColor(SysColor::LightWhite)   << "::";
    std::cout << XtermCodes::charDefaults() << "\n\n";

    std::cout << "Color cube, 6x6x6:\n";
    for (uint8_t green { 0 }; green < 6; ++green) {
        for (uint8_t red { 0 }; red < 6; ++red) {
            for (uint8_t blue { 0 }; blue < 6; ++blue) {
                uint8_t color ( 16 + (red * 36) + (green * 6) + blue );
                std::cout << XtermCodes::charBgColor(color) << "::";
            }
            std::cout << XtermCodes::charDefaults() << " ";
        }
        std::cout << "\n";
    }

    std::cout << "Grayscale ramp:\n";
    for (uint32_t color { 232 }; color < 256; ++color) {
        std::cout << XtermCodes::charBgColor(color) << "::";
    }
    std::cout << XtermCodes::charDefaults() << "\n\n";


    std::cout << "Examples for the 3-byte color mode\n\n";

    std::cout << "Color cube\n";
    for (uint32_t green { 0 }; green < 256; green += 51) {
        for (uint32_t red { 0 }; red < 256; red += 51) {
            for (uint32_t blue { 0 }; blue < 256; blue += 51) {
                std::cout << XtermCodes::charFgColor(red, green, blue)   << "::";
            }
            std::cout << XtermCodes::charDefaults() << " ";
        }
        std::cout << "\n";
    }

    std::cout << "Grayscale ramp:\n";
    for (uint32_t gray { 8 }; gray < 256; gray += 10) {
        std::cout << XtermCodes::charFgColor(gray, gray, gray)   << "::";
    }
    std::cout << XtermCodes::charDefaults() << "\n\n";

    std::cout << "Color cube\n";
    for (uint32_t green { 0 }; green < 256; green += 51) {
        for (uint32_t red { 0 }; red < 256; red += 51) {
            for (uint32_t blue { 0 }; blue < 256; blue += 51) {
                std::cout << XtermCodes::charBgColor(red, green, blue)   << "::";
            }
            std::cout << XtermCodes::charDefaults() << " ";
        }
        std::cout << "\n";
    }

    std::cout << "Grayscale ramp:\n";
    for (uint32_t gray { 8 }; gray < 256; gray += 10) {
        std::cout << XtermCodes::charBgColor(gray, gray, gray)   << "::";
    }
    std::cout << XtermCodes::charDefaults() << "\n\n";
}
