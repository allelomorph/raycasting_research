// TBD: how many of these headers can be converted to <c...> to move to std::?

#include <sys/types.h>    // open(2) gid_t uid_t passwd
#include <sys/stat.h>     // open(2)
#include <fcntl.h>        // open(2)
#include <errno.h>
//#include <string.h>       // strerror
#include <unistd.h>       // STDIN_FILENO read uid_t gid_t get(set)egid get(set)euid
#include <linux/input.h>  // input_event
#include <sys/ioctl.h>    // EVIOCGRAB
#include <sys/select.h>   // fd_set FD_ZERO FD_SET

#include <termios.h>      // termios tcgetattr tcsetattr
#include <pwd.h>          // getpwnam


#include <string>
#include <map>

#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>      // transform

#include <cstring>        // strerror
#include <csignal>        // signal sig_atomic_t SIGINT SIGTERM
#include <cstdio>         // popen FILE perror feof fgets pclose
#include <cstdlib>        // atoi exit
//#include <cctype>         // isdigit tolower

/*
TBD: what is including ctype.h?
#ifdef _CTYPE_H
#error "_CTYPE_H defined"
#endif
*/

//#define DEBUG
//#include "typeName.hh"

// TBD: why did terminal_cursor_up stop working?
// CSI_CURSOR_UP expects an unsigned integer
#define CSI_CURSOR_UP( x ) "\x1b[" #x "A"

/*
#ifndef DEBUG
static const char* terminal_cursor_up(const uint32_t *lines*) {
    static constexpr char ESC { '\x1b' };
    static constexpr char CSI[2] { ESC, '[' };

    std::ostringstream oss;
    oss << CSI; // << lines << 'A';
    return oss.str().c_str();
}
#endif  // not debugging
*/
static volatile std::sig_atomic_t stop { 0 };

static void interrupt_handler(int /*signal*/) {
    stop = 1;
}

/**
 * Test ability to grab keyboard device; ungrab on failure.
 *
 * @param fd The file descriptor to the keyboard device
 * @return 0 if the grab was successful, or 1 otherwise.
 */
static int testGrabDevice(int fd)
{
    int rc { ioctl(fd, EVIOCGRAB, (void*)1) };
    if (rc != 0)
        ioctl(fd, EVIOCGRAB, (void*)0);
    return rc;
}


// forks into child, executes cmd and returns string output
// adapted from https://github.com/kernc/logkeys/blob/master/src/logkeys.cc execute()
static std::string getShellCmdOutput(const char* cmd)
{
    // Likely running main either as root or as member of `input` group
    gid_t gid { getegid() };
    uid_t uid { geteuid() };

    // For safety, while running other programs, switch user to nobody
    struct passwd* nobody_pwd { getpwnam("nobody") };
    if (nobody_pwd == nullptr) {
        // TBD: standardize error returns
        errno = 0;
        std::ostringstream msg;
        msg << "getpwnam: " << std::strerror(errno);
        throw std::runtime_error(msg.str());
    }
    setegid(nobody_pwd->pw_gid);
    seteuid(nobody_pwd->pw_uid);

    FILE* pipe { popen(cmd, "r") };
    if (pipe == nullptr) {
        // TBD: standardize error returns
        errno = 0;
        std::ostringstream msg;
        msg << "popen: " << std::strerror(errno);
        throw std::runtime_error(msg.str());
    }
    char buffer[128];
    std::string result;
    while (!std::feof(pipe)) {
        if (std::fgets(buffer, 128, pipe) != nullptr)
            result += buffer;
    }
    if (pclose(pipe) == -1) {
        // TBD: standardize error returns
        errno = 0;
        std::ostringstream msg;
        msg << "pclose: " << std::strerror(errno);
        throw std::runtime_error(msg.str());
    }
    // restore original user and group
    setegid(gid);
    seteuid(uid);

    return result;
}

#define EXE_GREP           "/bin/grep"
#define INPUT_EVENT_PATH   "/dev/input/"
// adapted from https://github.com/kernc/logkeys/blob/master/src/logkeys.cc determine_input_device()
static std::string determineInputDevice(void)
{
    // Look for devices with keybit bitmask that has keys a keyboard doeas
    // If a bitmask ends with 'e', it supports KEY_2, KEY_1, KEY_ESC, and KEY_RESERVED is set to 0, so it's probably a keyboard
    // keybit:   https://github.com/torvalds/linux/blob/02de58b24d2e1b2cf947d57205bd2221d897193c/include/linux/input.h#L45
    // keycodes: https://github.com/torvalds/linux/blob/139711f033f636cc78b6aaf7363252241b9698ef/include/uapi/linux/input-event-codes.h#L75
    // Take the Name, Handlers, and KEY values
    const char* cmd = EXE_GREP " -B8 -E 'KEY=.*e$' /proc/bus/input/devices | "
        EXE_GREP " -E 'Name|Handlers|KEY' ";
    std::stringstream output(getShellCmdOutput(cmd));

    std::vector<std::string> devices;
    std::vector<unsigned short> scores;
    std::string line;

    // devices after grep come in 3 lines each, for example:
    // ```
    // N: Name="AT Translated Set 2 keyboard"
    // H: Handlers=sysrq kbd event2 leds
    // B: KEY=402000000 3803078f800d001 feffffdfffefffff fffffffffffffffe
    // ```
    enum { Name, Handlers, KEY };
    for (unsigned short line_type { 0 }, score { 0 }; std::getline(output, line);) {
        // TBD: passsing std::tolower causes problem with unresolved overload,
        //   but global ::tolower works
        // Further, it looks like cctype or ctype.h has been included by another header
        std::transform(line.begin(), line.end(), line.begin(), ::tolower);
        switch (line_type) {
        case Name:
            // Generate score based on device name
            if (line.find("keyboard") != std::string::npos)
                score += 100;
            break;
        case Handlers: {
            // Add the event handler
            std::string::size_type i { line.find("event") };
            if (i != std::string::npos) {
                i += 5; // "event".size() == 5
                if (i < line.size() && std::isdigit(line[i])) {
                    int index { std::atoi(line.c_str() + i) };
                    std::stringstream input_dev_path;
                    input_dev_path << INPUT_EVENT_PATH << "event" << index;
                    devices.emplace_back(input_dev_path.str());
                }
            }
            break;
        }
        case KEY: {
            // Generate score based on size of key bitmask
            std::string::size_type i { line.find("=") };
            std::string full_key_map { line.substr(i + 1) };
            score += full_key_map.length();
            scores.emplace_back(score);
            score = 0;
            break;
        }
        default: break;
        }
        line_type = (line_type + 1) % 3;
    }

    if (devices.size() == 0) {
        // TBD: standardize error returns
        std::cerr << "Couldn't determine keyboard device.\n";
        exit(EXIT_FAILURE);
    }

    // Choose device with the best score
    int max_device_i { static_cast<int>(
            std::max_element(scores.begin(), scores.end()) - scores.begin()) };
    return devices[max_device_i];
}

struct HUDGlyph {
    bool pressed;
    const char* repr;
};

int main() {
    // determine likely keboard device file path via /proc/bus/input/devices
    std::string kbd_path { determineInputDevice() };
    std::cout << "Selected keyboard device path: " << kbd_path << '\n';
    int fd { open(kbd_path.c_str(), O_RDONLY) };
    if (fd < 0) {
        std::cerr << "Failure to open \"" << kbd_path << "\": " <<
            std::strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    if (testGrabDevice(fd) != 0) {
        // TBD: standardize error failure
        std::cerr << "Failure to grab device \"" << kbd_path <<
            "\": check permissions, and that it is not already grabbed by X." <<
            std::endl;
        return EXIT_FAILURE;
    }

    signal(SIGINT, interrupt_handler);
    signal(SIGTERM, interrupt_handler);

    // beware: sizeof and .size() both consider unicode in char* or string as
    //   1 char and 1 byte(!), regardless of actual encoding size in bytes (3 each for arrows)
    std::map<int, HUDGlyph> display_map {
        { KEY_A,     { false, "a" } },
        { KEY_S,     { false, "s" } },
        { KEY_D,     { false, "d" } },
        { KEY_F,     { false, "f" } },
        { KEY_LEFT,  { false, "\u2190" } },
        { KEY_UP,    { false, "\u2191" } },
        { KEY_RIGHT, { false, "\u2192" } },
        { KEY_DOWN,  { false, "\u2193" } },
        { KEY_C,          { false, "" } },
        { KEY_LEFTCTRL,   { false, "" } },
        { KEY_RIGHTCTRL,  { false, "" } }
    };

    struct input_event ev[64];
    fd_set rdfds;
    FD_ZERO(&rdfds);     // init fd_set
    FD_SET(fd, &rdfds);  // add fd to read fd set
    ssize_t rd;

    while (!stop) {
        // blocks until there is something to read in fd, but does not prevent
        //   other reads like a blocking read() would
        select(fd + 1, &rdfds, NULL, NULL, NULL);
        if (stop)
            break;
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
                if (ev[i].code == KEY_LEFTCTRL || ev[i].code == KEY_RIGHTCTRL) {
                    display_map[ev[i].code].pressed = ev[i].value;
                    continue;
                }
                if (ev[i].code == KEY_C && ev[i].value == 1) {
                    stop = (display_map[KEY_LEFTCTRL].pressed ||
                            display_map[KEY_RIGHTCTRL].pressed);
                    continue;
                }
#ifdef DEBUG
                std::cout << "\tev[" << i <<
                    "]: code: " << ev[i].code <<
                    " value: " << ev[i].value;
                auto HUD_key_it { display_map.find(ev[i].code) };
                if (ev[i].value > 1 ||                   // 2: autorepeat
                    HUD_key_it == display_map.end() ) {  // unsupported key
                    std::cout << '\n';
                } else {
                    HUDGlyph HUD_key { (*HUD_key_it).second };
                    std::cout << " pressed: " << HUD_key.pressed <<
                        " HUD char: " << HUD_key.repr << '\n';
                }
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
#ifndef DEBUG
        for (const auto& pair : display_map) {
            std::cout << (pair.second.pressed ?
                pair.second.repr : " ") << ' ';
        }
        // TBD: why did terminal_cursor_up(1) stop working in testing?
        std::cout << '\n' << CSI_CURSOR_UP(1);
#endif  // not debugging
    }

    // ungrab (test_grab leaves grabbed if successful)
    ioctl(fd, EVIOCGRAB, (void*)0);
}
