#define NOECHO
//#define TIMED
//#define DEBUG

#include <sys/types.h>    // open(2)
#include <sys/stat.h>     // open(2)
#include <fcntl.h>        // open(2)
#include <errno.h>
#include <string.h>       // strerror
#include <unistd.h>       // STDIN_FILENO read
#include <linux/input.h>  // input_event
#include <sys/ioctl.h>    // EVIOCGRAB
#include <sys/select.h>   // fd_set FD_ZERO FD_SET

#ifdef NOECHO
#include <termios.h>      // termios tcgetattr tcsetattr
#endif  // no echo

#include <string>
#include <map>

#include <iostream>
#include <sstream>

#ifdef TIMED
#include <chrono>
#else   // signal quit
#include <csignal>        // signal sig_atomic_t SIGINT SIGTERM
#endif  // signal quit

//#include "typeName.hh"

static const char* terminal_esc_up(const uint32_t lines) {
    static constexpr char ESC { '\x1b' };
    static constexpr char CSI[2] { ESC, '[' };

    std::ostringstream oss;
    oss << CSI << lines << 'A';
    return oss.str().c_str();
}

#ifndef TIMED
static volatile std::sig_atomic_t stop { 0 };

static void interrupt_handler(int /*sig*/) {
    stop = 1;
}
#endif  // signal quit

/**
 * Grab and immediately ungrab the keyboard device.
 *
 * @param fd The file descriptor to the keyboard device
 * @return 0 if the grab was successful, or 1 otherwise.
 */
static int test_grab(int fd)
{
    int rc { ioctl(fd, EVIOCGRAB, (void*)1) };
    if (rc != 0)
        ioctl(fd, EVIOCGRAB, (void*)0);
    return rc;
}


struct HUDGlyph {
    bool pressed;
    const char* repr;
};


int main() {
    std::string kbd_filename { "/dev/input/by-path/platform-i8042-serio-0-event-kbd" };
    int fd { open(kbd_filename.c_str(), O_RDONLY) };
    if (fd < 0) {
        std::cerr << "Failure to open \"" << kbd_filename << "\": " <<
            strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    std::map<int, HUDGlyph> display_map {
        { KEY_A,     { false, "a" } },
        { KEY_S,     { false, "s" } },
        { KEY_D,     { false, "d" } },
        { KEY_F,     { false, "f" } },
        { KEY_LEFT,  { false, "\u2190" } },
        { KEY_UP,    { false, "\u2191" } },
        { KEY_RIGHT, { false, "\u2192" } },
        { KEY_DOWN,  { false, "\u2193" } }
    };
    // beware: sizeof and .size() both consider unicode in char* or string as
    //   1 char and 1 byte(!), regardless of actual encoding size in bytes (3 each for arrows)
#ifdef TIMED
    using namespace std::literals; // s, ms (std::chrono::literals) requires C++14+
    auto beg_t { std::chrono::system_clock::now() };
    auto curr_t { std::chrono::system_clock::now() };
#else   // signal quit
    signal(SIGINT, interrupt_handler);
    signal(SIGTERM, interrupt_handler);
#endif  // signal quit

    if (test_grab(fd) != 0) {
        std::cerr << "Failure to grab device \"" << kbd_filename <<
            "\": check permissions and that it is not grabbed by X." <<
            std::endl;
        return EXIT_FAILURE;
    }

#ifdef NOECHO
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);

    term.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, 0, &term);
#endif  // no echo

    struct input_event ev[64];
    fd_set rdfds;
    FD_ZERO(&rdfds);     // init fd_set
    FD_SET(fd, &rdfds);  // add fd to read fd set
    ssize_t rd;

#ifdef TIMED
    while (curr_t - beg_t < 10s) {
        curr_t = std::chrono::system_clock::now();
#else   // signal quit
    while (!stop) {
#endif  // signal quit
        // blocks until there is something to read in fd, but does not prevent
        //   other reads like a blocking read() would
        select(fd + 1, &rdfds, NULL, NULL, NULL);
#ifndef TIMED  // signal quit
        if (stop)
            break;
#endif         // signal quit
        rd = read(fd, ev, sizeof(ev));
        if (rd < (int) sizeof(struct input_event)) {
            std::cerr << "expected " << sizeof(struct input_event) <<
                " bytes, got " << rd << ", read: " << strerror(errno);
            return EXIT_FAILURE;
        }
#ifdef DEBUG
        std::cout << "read " << (rd / sizeof(struct input_event)) <<
            " input_event structs in this frame\n";
#endif  // DEBUG
        for (size_t i {0}; i < rd / sizeof(struct input_event); ++i) {
            if (ev[i].type == EV_KEY) {
#ifdef DEBUG
                std::cout << "\tev[" << i <<
                    "]: code: " << ev[i].code <<
                    " value: " << ev[i].value;
                auto HUD_key_it { display_map.find(ev[i].code) };
                if (ev[i].value > 1 ||                   // 2: autorepeat
                    HUD_key_it == display_map.end() ) {  // unsupported key
                    std::cout << '\n';
                    continue;
                }
                HUDGlyph HUD_key { (*HUD_key_it).second };
                std::cout << " HUD index: " << HUD_key.kbd_HUD_i <<
                    " HUD char: " << HUD_key.c << '\n';
#else  // not debugging
                auto HUD_key_it { display_map.find(ev[i].code) };
                int value { ev[i].value };
                if (HUD_key_it != display_map.end() &&
                    (value == 0 || value == 1)) {
                    // 0: release 1: press 2: autorepeat
                    HUD_key_it->second.pressed = value;
                }
#endif  // not debugging
            }
        }

        for (const auto& pair : display_map) {
            std::cout << (pair.second.pressed ?
                pair.second.repr : " ") << ' ';
        }
        std::cout << '\n' << terminal_esc_up(1);
    }

#ifdef NOECHO
    term.c_lflag |= ECHO;
    tcsetattr(fileno(stdin), 0, &term);
#endif  // no echo

    // ungrab (test_grab leaves grabbed if successful)
    ioctl(fd, EVIOCGRAB, (void*)0);
}
