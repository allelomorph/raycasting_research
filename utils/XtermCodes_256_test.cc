#include "XtermCodes.hh"
#include "xterm256colors.hh"

#include <iostream>
#include <iomanip>





int main() {
    std::cout << "256 color mode\n\n";

    std::cout << "System colors:\n";
    for (uint8_t code { 0 }; code < 16; ++code) {
        std::cout <<
            XtermCodes::CharBgColor(code) << std::setw(3) << (int)code <<
            XtermCodes::CharDefaults() << ' ' <<
            xterm_256_colors[code] << '\n';
    }
    std::cout << "\n";

    std::cout << "Color cubes, 6x6x6:\n";
    for (uint8_t green { 0 }; green < 6; ++green) {
        for (uint8_t red { 0 }; red < 6; ++red) {
            for (uint8_t blue { 0 }; blue < 6; ++blue) {
                uint8_t code ( 16 + (red * 36) + (green * 6) + blue );
                std::cout << XtermCodes::CharBgColor(code) <<
                    std::setw(4) << (int)code;
            }
            std::cout << XtermCodes::CharDefaults() << " ";
        }
        std::cout << "\n";
    }

    std::cout << "Contiguous greyscale\n";
    for (uint16_t code { 232 }; code < 256; ++code) {
        std::cout << XtermCodes::CharBgColor(code) <<
            std::setw(4) << (int)code;
    }
    std::cout << XtermCodes::CharDefaults() << "\n";

    std::cout << "colorCodeFromRGB(43, 181, 205): " << (int)XtermCodes::colorCodeFromRGB(43, 181, 205) << '\n';
    std::cout << "colorCodeFromRGB(128, 128, 128): " << (int)XtermCodes::colorCodeFromRGB(128, 128, 128) << '\n';
    std::cout << "colorCodeFromRGB(255, 255, 255): " << (int)XtermCodes::colorCodeFromRGB(255, 255, 255) << '\n';
}
