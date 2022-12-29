#include "LinuxKbdInputMgr.hh"
#include "safeCExec.hh"

#include <sys/types.h>     // open pid_t
#include <sys/stat.h>      // open
#include <fcntl.h>         // open
#include <sys/ioctl.h>     // ioctl EVIOCGRAB
#include <linux/input.h>   // input_event EV_* KEY_*
#include <unistd.h>        // getpid ttyname read close
#include <utmp.h>          // utmp setutent getutent endutent USER_PROCESS
#include <sys/select.h>    // FD_ZERO FD_SET

#include <cstdlib>         // atoi exit getenv
#include <cctype>          // isdigit tolower

#include <vector>
#include <string>          // getline
#include <algorithm>       // transform max_element
#include <fstream>
#include <iostream>        // cerr


// pre-C++17, static constexpr class members also need defintion (without init)
//   in namespace scope
constexpr char LinuxKbdInputMgr::INPUT_EVENT_PATH_PREFIX[];
constexpr char LinuxKbdInputMgr::INPUT_DEVICES_PATH[];

void LinuxKbdInputMgr::grabDevice(const std::string& exec_filename) {
    kbd_device_fd = safeCExec(open, "open", C_RETURN_TEST(int, (ret == -1)),
                              kbd_device_path.c_str(), O_RDONLY);
    try {
        safeCExec(ioctl, "ioctl", C_RETURN_TEST(int, (ret == -1)),
                  kbd_device_fd, EVIOCGRAB, (void*)1);
    } catch (std::runtime_error& re) {
        safeCExec(ioctl, "ioctl", C_RETURN_TEST(int, (ret == -1)),
                  kbd_device_fd, EVIOCGRAB, (void*)0);
        throw re;
    }
    std::ofstream ofs;
    ofs.open(input_tty_name);
    if (ofs.is_open()) {
        // generate grab message
        static constexpr uint16_t msg_width { 100 };
        static const std::string msg_border ( msg_width, '*' );
        static constexpr char msg_line_1_prefix[] { "WARNING! Process " };
        static constexpr char msg_line_1_suffix[] { " is grabbing keyboard events." };
        static constexpr char msg_line_2[] { "Please maintain keyboard focus "
                "on this tty until ungrab message appears." };
        ofs << '\n';
        ofs << msg_border << '\n';
        ofs << '\t' << msg_line_1_prefix << static_cast<int>(getpid()) <<
            ": " << exec_filename << msg_line_1_suffix << '\n';
        ofs << '\t' << msg_line_2 << '\n';
        ofs << msg_border << std::endl;
        ofs.close();
    }
}

void LinuxKbdInputMgr::ungrabDevice() {
    if (kbd_device_fd != UNINITIALIZED_FD) {
        safeCExec(ioctl, "ioctl", C_RETURN_TEST(int, (ret == -1)),
                  kbd_device_fd, EVIOCGRAB, (void*)0);
        safeCExec(close, "close", C_RETURN_TEST(int, (ret == -1)),
                  kbd_device_fd);
        std::ofstream ofs;
        ofs.open(input_tty_name);
        if (ofs.is_open()) {
            // generate ungrab message
            static constexpr uint16_t msg_width { 100 };
            static const std::string msg_border ( msg_width, '*' );
            static constexpr char msg_line_1_prefix[] { "Process " };
            static constexpr char msg_line_1_suffix[] { " is no longer grabbing keyboard events." };
            ofs << '\n';
            ofs << msg_border << '\n';
            ofs << '\t' << msg_line_1_prefix << static_cast<int>(getpid()) <<
                msg_line_1_suffix << '\n';
            ofs << msg_border << '\n';
            ofs.close();
        }
    }
}

// inspired by https://github.com/kernc/logkeys/blob/master/src/logkeys.cc
//    determine_input_device(), but parses file directly rather than popen(grep...)
// determine likely keyboard device file path via /proc/bus/input/devices
std::string LinuxKbdInputMgr::determineInputDevice() {
    std::ifstream ifs( INPUT_DEVICES_PATH );
    std::ostringstream error_msg;
    if (!ifs.is_open()) {
        error_msg << __FUNCTION__ << ": couldn't open " << INPUT_DEVICES_PATH;
        throw std::runtime_error(error_msg.str());
    }
    // Interpretation of /proc/bus/input/devices as linux/input.h `struct
    //   input_dev`s, see: https://unix.stackexchange.com/questions/74903/
    // """
    // I @id: id of the device (struct input_id)
    //     Bus     => id.bustype
    //     Vendor  => id.vendor
    //     Product => id.product
    //     Version => id.version
    // N => name of the device
    // P => physical path to the device in the system hierarchy
    // S => sysfs path
    // U => unique identification code for the device (if device has it)
    // H => list of input handles associated with the device
    // B => bitmaps
    //     PROP => device properties and quirks
    //     EV   => types of events supported by the device
    //     KEY  => keys/buttons this device has
    //     MSC  => miscellaneous events supported by the device
    //     LED  => leds present on the device
    // """
    // In local testing, there was no leading whitespace, and every entry
    //   including the last ends with a blank line, for example:
    // ```
    // I: Bus=0011 Vendor=0001 Product=0001 Version=ab41
    // N: Name="AT Translated Set 2 keyboard"
    // P: Phys=isa0060/serio0/input0
    // S: Sysfs=/devices/platform/i8042/serio0/input/input2
    // U: Uniq=
    // H: Handlers=sysrq kbd event2 leds
    // B: PROP=0
    // B: EV=120013
    // B: KEY=402000000 3803078f800d001 feffffdfffefffff fffffffffffffffe
    // B: MSC=10
    // B: LED=7
    //
    // ```
    std::string name;
    std::string handlers;
    std::string key;
    std::vector<std::string> device_paths;
    std::vector<unsigned short> device_scores;

    for (unsigned short score { 0 }; ifs.good();
         name.clear(), handlers.clear(), key.clear()) {
        // Get relevant lines from device entry
        for (std::string line; std::getline(ifs, line) && line.size() > 0;) {
            if (line.find("N: Name=") != std::string::npos)
                name = line.substr(8);       // std::string("N: Name=").size()
            else if (line.find("H: Handlers=") != std::string::npos)
                handlers = line.substr(12);  // std::string("H: Handlers=").size()
            else if (line.find("B: KEY=") != std::string::npos)
                key = line.substr(7);        // std::string("B: KEY=").size()
        }
        // Look for devices with keybit bitmask conataining standard keyboard keys
        // If a bitmask ends with 'e', it supports KEY_2, KEY_1, KEY_ESC, and
        //    KEY_RESERVED is set to 0, so it's probably a keyboard
        // keybit:   https://github.com/torvalds/linux/blob/02de58b24d2e1b2cf947d57205bd2221d897193c/include/linux/input.h#L45
        // keycodes: https://github.com/torvalds/linux/blob/139711f033f636cc78b6aaf7363252241b9698ef/include/uapi/linux/input-event-codes.h#L75
        if (key[key.size() - 1] == 'e') {
            // Generate score based on device name
            // TBD: passsing std::tolower causes problem with unresolved overload,
            //   but global ::tolower works
            // TBD: also it looks like cctype or ctype.h has been included by another header
            std::transform(name.begin(), name.end(),
                           name.begin(), ::tolower);
            if (name.find("keyboard") != std::string::npos)
                score += 100;

            // Add the event handler
            std::string::size_type i { handlers.find("event") };
            if (i != std::string::npos) {
                i += 5;  // std::string("event").size()
                if (i < handlers.size() && std::isdigit(handlers[i])) {
                    int index { std::atoi(handlers.c_str() + i) };
                    std::stringstream input_device_path;
                    input_device_path << INPUT_EVENT_PATH_PREFIX << index;
                    device_paths.emplace_back(input_device_path.str());
                }
            }

            // Generate score based on size of key bitmask
            i = key.find("=");
            if (i != std::string::npos)
                score += key.substr(i + 1).size();

            device_scores.emplace_back(score);
            score = 0;
        }
    }
    ifs.close();

    if (device_paths.size() == 0) {
        error_msg << __FUNCTION__ << ": couldn't determine keyboard device file";
        throw std::runtime_error(error_msg.str());
    }
    auto max_device_i {
            std::max_element(device_scores.begin(),
                             device_scores.end()) - device_scores.begin() };
    return device_paths[max_device_i];
}

std::string LinuxKbdInputMgr::determineInputTty() {
    // Use of ttyname taken from coreutils tty, see:
    //  - https://github.com/coreutils/coreutils/blob/master/src/tty.c
    std::string curr_tty_name { safeCExec(ttyname, "ttyname",
                                          C_RETURN_TEST(char *, (ret == nullptr)),
                                          STDIN_FILENO) };
    char *SSH_TTY { getenv("SSH_TTY") };

    // Current tty is true tty with hardware access
    if (curr_tty_name.find("tty") != std::string::npos && SSH_TTY == nullptr)
        return curr_tty_name;

    // Current tty is pty and/or ssh session
    // Need to find and nominate a true tty for input capture
    std::string candidate_tty_name;
    bool valid_input_tty_found;
    // Use of utmp taken from coreutils who, see:
    //  - https://github.com/coreutils/coreutils/blob/master/src/who.c
    //  - https://github.com/coreutils/gnulib/blob/master/lib/readutmp.h
    struct utmp *ut;
    // Note: man 3 getutent prescribes calling setutent first as a best practice,
    //   but in testing it fails here with ENOENT, so we ignore that case
    // opens _PATH_UTMP (eg /var/run/utmp)
    safeCExec(setutent, "setutent", C_ERRNO_TEST( (err != ENOENT) ));
    while ((ut = getutent()) != nullptr) {
        // USER_PROCESS: normal process with attached username that is not LOGIN
        if (ut->ut_type == USER_PROCESS) {
            candidate_tty_name.clear();
            candidate_tty_name.append("/dev/");
            candidate_tty_name.append(ut->ut_line);
            if (candidate_tty_name.find("tty") != std::string::npos) {
                std::ofstream ofs;
                ofs.open(candidate_tty_name);
                if (ofs.is_open()) {
                    valid_input_tty_found = true;
                    ofs.close();
                    break;
                }
            }
        }
    }
    // closes _PATH_UTMP
    safeCExec(endutent, "endutent");

    if (!valid_input_tty_found) {
        std::ostringstream error_msg;
        error_msg << __FUNCTION__ << ": No non-pty tty found, cannot capture " <<
            " any keyboard events" << std::endl;
        throw std::runtime_error(error_msg.str());
    }

    return candidate_tty_name;
}

LinuxKbdInputMgr::LinuxKbdInputMgr(const std::string& exec_filename) {
    key_states = std::unordered_map<int32_t, KeyState> {
        { KEY_LEFT,       KeyState (KEY_LEFT       ) },
        { KEY_UP,         KeyState (KEY_UP         ) },
        { KEY_RIGHT,      KeyState (KEY_RIGHT      ) },
        { KEY_DOWN,       KeyState (KEY_DOWN       ) },
        { KEY_C,          KeyState (KEY_C          ) },
        { KEY_F1,         KeyState (KEY_F1         ) },
        { KEY_F2,         KeyState (KEY_F2         ) },
        { KEY_F3,         KeyState (KEY_F3         ) },
        { KEY_F4,         KeyState (KEY_F4         ) },
        { KEY_F5,         KeyState (KEY_F5         ) },
        { KEY_F10,        KeyState (KEY_F10        ) },
        { KEY_F11,        KeyState (KEY_F11        ) },
        { KEY_F12,        KeyState (KEY_F12        ) },
        { KEY_LEFTSHIFT,  KeyState (KEY_LEFTSHIFT  ) },
        { KEY_RIGHTSHIFT, KeyState (KEY_RIGHTSHIFT ) },
        { KEY_LEFTCTRL,   KeyState (KEY_LEFTCTRL   ) },
        { KEY_RIGHTCTRL,  KeyState (KEY_RIGHTCTRL  ) },
        { KEY_LEFTALT,    KeyState (KEY_LEFTALT    ) },
        { KEY_RIGHTALT,   KeyState (KEY_RIGHTALT   ) },
        { KEY_ESC,        KeyState (KEY_ESC        ) }
    };

    kbd_device_path = determineInputDevice();
    std::cout << "Selected keyboard device path: " << kbd_device_path << '\n';

    input_tty_name = determineInputTty();
    std::cout << "Terminal focus for input: " << input_tty_name << "\n";

    grabDevice(exec_filename);

    // init fd_set used by select(2) in consumeKeyEvents
    FD_ZERO(&rdfds);
}

LinuxKbdInputMgr::~LinuxKbdInputMgr() {
    ungrabDevice();
}

void LinuxKbdInputMgr::consumeKeyEvents() {
    std::ostringstream error_oss;
    // select() blocks until there is something to read in fd, but does not
    //   prevent other reads like a blocking read() would.
    // using timeout prevents app shutdown hanging on frame with no keyboard input
    FD_SET(kbd_device_fd, &rdfds);
    // using copy of select_timeout per man select(2): "On Linux, select()
    //   modifies timeout to reflect the amount of time not slept"
    struct timeval tv { select_timeout };
    // select could be interrupted by a signal and return failure (likely cases
    //   in this application are SIGTERM, SIGINT, or SIGWINCH)
    safeCExec(select, "select", C_RETURN_ERRNO_TEST(int, (ret == -1 && err != EINTR)),
              kbd_device_fd + 1, &rdfds, nullptr, nullptr, &tv);
    // select() interrupted by a signal, or did it time out?
    if (errno == EINTR || !FD_ISSET(kbd_device_fd, &rdfds))
        return;
    ssize_t rd { safeCExec(read, "read", C_RETURN_TEST(ssize_t, (ret == -1)),
                           kbd_device_fd, ev, sizeof(ev)) };
    if (rd < (ssize_t)sizeof(struct input_event)) {
        error_oss << __FUNCTION__ << ": expected to read at least " <<
            sizeof(struct input_event) << " bytes, got " << rd;
        throw std::runtime_error(error_oss.str());
    }
    std::size_t input_event_ct { rd / sizeof(struct input_event) };
    for (std::size_t i {0}; i < input_event_ct; ++i) {
        // Reading from the keyboard device, so all events should be EV_KEY
        if (ev[i].type != EV_KEY)
            continue;
        auto ks_it { key_states.find(ev[i].code) };
        if (ks_it != key_states.end())  // supported key
            ks_it->second.update(ev[i].value);  // should be in KeyValue range
    }
}

bool LinuxKbdInputMgr::keyDownThisFrame(const int32_t keysym) {
    // remember that operator[] inserts a new key if not found
    auto ks_it { key_states.find(keysym) };
    if (ks_it != key_states.end())
        return (ks_it->second.keyDownThisFrame());
    return false;
}

bool LinuxKbdInputMgr::isPressed(const int32_t keysym) {
    // remember that operator[] inserts a new key if not found
    auto ks_it { key_states.find(keysym) };
    if (ks_it != key_states.end())
        return (ks_it->second.isPressed());
    return false;
}

bool LinuxKbdInputMgr::isReleased(const int32_t keysym) {
    // remember that operator[] inserts a new key if not found
    auto ks_it { key_states.find(keysym) };
    if (ks_it != key_states.end())
        return (ks_it->second.isReleased());
    return true;
}

void LinuxKbdInputMgr::decayToAutorepeat() {
    for (auto& pair : key_states) {
        pair.second.decayToAutorepeat();
    }
}
