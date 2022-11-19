#include "XtermCodes.hh"

#include <iostream>

// adapted from https://github.com/robertknight/konsole/blob/master/tests/color-spaces.pl
// xterm CSI reference: https://invisible-island.net/xterm/ctlseqs/ctlseqs.html

namespace XtermCodes {

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


}  // namespace XtermCodes
