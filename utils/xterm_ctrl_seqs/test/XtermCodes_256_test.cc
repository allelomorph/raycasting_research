#include "Xterm.hh"
#include "xterm256colors.hh"

#include <iostream>
#include <iomanip>


int main() {
    using namespace Xterm;

    std::cout << "256 color mode\n\n";

    std::cout << "System colors:\n";
    for (uint8_t code { 0 }; code < 16; ++code) {
        std::cout <<
            CtrlSeqs::CharBgColor(code) << std::setw(3) << (int)code <<
            CtrlSeqs::CharDefaults() << ' ' <<
            xterm_256_colors[code] << '\n';
    }
    std::cout << "\n";

    std::cout << "Color cubes, 6x6x6:\n";
    for (uint8_t green { 0 }; green < 6; ++green) {
        for (uint8_t red { 0 }; red < 6; ++red) {
            for (uint8_t blue { 0 }; blue < 6; ++blue) {
                uint8_t code ( 16 + (red * 36) + (green * 6) + blue );
                std::cout << CtrlSeqs::CharBgColor(code) <<
                    std::setw(4) << (int)code;
            }
            std::cout << CtrlSeqs::CharDefaults() << " ";
        }
        std::cout << "\n";
    }

    std::cout << "Contiguous greyscale\n";
    for (uint16_t code { 232 }; code < 256; ++code) {
        std::cout << CtrlSeqs::CharBgColor(code) <<
            std::setw(4) << (int)code;
    }
    std::cout << CtrlSeqs::CharDefaults() << "\n";

    std::cout << "Color::Codes::fromRGB(43, 181, 205): " << (int)Color::Codes::fromRGB(43, 181, 205) << '\n';
    std::cout << "\tor, " <<
        CtrlSeqs::CharBgColor(43, 181, 205) << "::" <<
        CtrlSeqs::CharDefaults() << " vs " <<
        CtrlSeqs::CharBgColor(Color::Codes::fromRGB(43, 181, 205)) << "::" <<
        CtrlSeqs::CharDefaults() << " (TrueColor vs converted to 256-color)\n";

    std::cout << "Color::Codes::fromRGB(128, 128, 128): " << (int)Color::Codes::fromRGB(128, 128, 128) << '\n';
    std::cout << "\tor, " <<
        CtrlSeqs::CharBgColor(128, 128, 128) << "::" <<
        CtrlSeqs::CharDefaults() << " vs " <<
        CtrlSeqs::CharBgColor(Color::Codes::fromRGB(128, 128, 128)) << "::" <<
        CtrlSeqs::CharDefaults() << " (TrueColor vs converted to 256-color)\n";

    std::cout << "Color::Codes::fromRGB(255, 255, 255): " << (int)Color::Codes::fromRGB(255, 255, 255) << '\n';
    std::cout << "\tor, " <<
        CtrlSeqs::CharBgColor(255, 255, 255) << "::" <<
        CtrlSeqs::CharDefaults() << " vs " <<
        CtrlSeqs::CharBgColor(Color::Codes::fromRGB(255, 255, 255)) << "::" <<
        CtrlSeqs::CharDefaults() << " (TrueColor vs converted to 256-color)\n";
}
