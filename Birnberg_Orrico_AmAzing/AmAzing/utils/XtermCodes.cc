#include "XtermCodes.hh"

#include <cstdint>

#include <iostream>
#include <array>


// adapted from https://github.com/robertknight/konsole/blob/master/tests/color-spaces.pl
// xterm CSI codes tested in Win11 cmd, over ssh into Vagrant/VirtualBox
//   Ubuntu 20.04 VM, see references:
//   - https://invisible-island.net/xterm/ctlseqs/ctlseqs.html
//   - https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences

namespace XtermCodes {

// converts 3-byte color to 1-byte color code
// xterm color data from:
//   - https://www.ditig.com/256-colors-cheat-sheet
uint8_t colorCodeFromRGB(uint8_t red, uint8_t green, uint8_t blue) {
    constexpr std::array<uint8_t, 6> cube_steps { 0, 95, 135, 175, 215, 255 };
    uint8_t code { 16 };
    uint8_t i;
    for (i = 0; i < 5 && red >= cube_steps[i]; ++i) {};
    code += (cube_steps[i] - red < red - cube_steps[i - 1]) ?
        36 * i : 36 * (i - 1);
    for (i = 0; i < 5 && green >= cube_steps[i]; ++i) {};
    code += (cube_steps[i] - green < green - cube_steps[i - 1]) ?
        6 * i : 6 * (i - 1);
    for (i = 0; i < 5 && blue >= cube_steps[i]; ++i) {};
    code += (cube_steps[i] - blue < blue - cube_steps[i - 1]) ?
        i : (i - 1);
    return code;
}

// ESC [ = control sequence initiator
static constexpr char CSI[] { "\x1b[" };

std::ostream& operator<<(std::ostream& os, const CharFgColor& cfgc) {
    if (cfgc.true_color) {
        return os << CSI << "38;2;" <<
            static_cast<int>(cfgc.red) << ';' <<
            static_cast<int>(cfgc.green) << ';' <<
            static_cast<int>(cfgc.blue) << 'm';
    }
    return os << CSI << "38;5;" << static_cast<int>(cfgc.color) << 'm';
}

std::ostream& operator<<(std::ostream& os, const CharBgColor& cbgc) {
    if (cbgc.true_color) {
        return os << CSI << "48;2;" <<
            static_cast<int>(cbgc.red) << ';' <<
            static_cast<int>(cbgc.green) << ';' <<
            static_cast<int>(cbgc.blue) << 'm';
    }
    return os << CSI << "48;5;" << static_cast<int>(cbgc.color) << 'm';
}

std::ostream& operator<<(std::ostream& os, const CharDefaults& /*cdflts*/) {
    return os << CSI << "0m";
}

std::ostream& operator<<(std::ostream& os, const CursorUp& cu) {
    return os << CSI << cu.rows << 'A';
}

std::ostream& operator<<(std::ostream& os, const CursorHome& ch) {
    return os << CSI << ch.row << ';' << ch.column << 'H';
}

std::ostream& operator<<(std::ostream& os, const EraseLinesBelow& /*elb*/) {
    // erase lines: below (J/0J), above (1J), above and below (2J),
    //   scrollback (3J)
    return os << CSI << 'J';
}

std::ostream& operator<<(std::ostream& os, const HideCursor& /*hc*/) {
    return os << CSI << "?25l";
}

std::ostream& operator<<(std::ostream& os, const ShowCursor& /*sc*/) {
    return os << CSI << "?25h";
}

}  // namespace XtermCodes
