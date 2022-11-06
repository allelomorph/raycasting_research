#include "KeyHandler.hh"
#include "safeCExec.hh"
#include "App.hh"          // stop

#include <linux/input.h>   // input_event EV_* KEY_*
#include <sys/ioctl.h>     // ioctl EVIOCGRAB
#include <unistd.h>        // getpid ttyname read close
#include <utmp.h>          // utmp setutent getutent endutent USER_PROCESS
#include <sys/select.h>    // FD_ZERO FD_SET
#include <sys/types.h>     // open pid_t
#include <sys/stat.h>      // open
#include <fcntl.h>         // open

#include <cstdlib>        // atoi exit getenv
#include <cctype>         // isdigit tolower

#include <vector>
#include <string>         // getline
#include <algorithm>      // transform max_element
#include <fstream>
#include <iostream>       // cerr

// used by determineInputDevice
#define INPUT_EVENT_PATH   "/dev/input/"


KeyHandler::KeyHandler() {
    key_states = std::unordered_map<LinuxKeyCode, InputKey> {
        { KEY_LEFT,       InputKey (KeyType::Live,        KEY_LEFT, "\u2190" ) },
        { KEY_UP,         InputKey (KeyType::Live,        KEY_UP, "\u2191" ) },
        { KEY_RIGHT,      InputKey (KeyType::Live,        KEY_RIGHT, "\u2192" ) },
        { KEY_DOWN,       InputKey (KeyType::Live,        KEY_DOWN, "\u2193" ) },
        { KEY_A,          InputKey (KeyType::Live,        KEY_A, "a" ) },
        { KEY_D,          InputKey (KeyType::Live,        KEY_D, "d" ) },
        { KEY_F,          InputKey (KeyType::ToggleOnOff, KEY_F, "f" ) },
        { KEY_M,          InputKey (KeyType::ToggleOnOff, KEY_M, "m" ) },
        { KEY_P,          InputKey (KeyType::ToggleOnOff, KEY_P, "p" ) },
        { KEY_Q,          InputKey (KeyType::Live,        KEY_Q, "q" ) },
        { KEY_C,          InputKey (KeyType::Live,        KEY_C, "c" ) },
        { KEY_LEFTCTRL,   InputKey (KeyType::Live,        KEY_LEFTCTRL, "<Lctrl>" ) },
        { KEY_RIGHTCTRL,  InputKey (KeyType::Live,        KEY_RIGHTCTRL, "<Rctrl>" ) },
        { KEY_ESC,        InputKey (KeyType::Live,        KEY_ESC, "<esc>" ) },
    };
}

static std:: string generateUngrabMessage() {
    std::ostringstream msg;
    static constexpr uint16_t msg_width { 100 };
    static std::string msg_border ( msg_width, '*' );
    static constexpr char msg_line_1_prefix[] { "Process " };
    static constexpr char msg_line_1_suffix[] { " is no longer grabbing keyboard events."};
    msg << '\n';
    msg << msg_border << '\n';
    msg << '\t' << msg_line_1_prefix << static_cast<int>(getpid()) <<
        msg_line_1_suffix << '\n';
    msg << msg_border << '\n';
    return msg.str();
}

void KeyHandler::ungrabDevice() {
    if (kbd_device_fd != UNINITIALIZED_FD) {
        safeCExec(ioctl, "ioctl", (int)-1, kbd_device_fd, EVIOCGRAB, (void*)0);
        safeCExec(close, "close", (int)-1, kbd_device_fd);
        std::ofstream ofs;
        ofs.open(input_tty_name);
        if (ofs.is_open()) {
            ofs << generateUngrabMessage();
            ofs.close();
        }
    }
}

KeyHandler::~KeyHandler() {
    ungrabDevice();
}

bool KeyHandler::isPressed(LinuxKeyCode keysym) {
    // remember that operator[] inserts a new key if not found
    auto ks_it { key_states.find(keysym) };
    if (ks_it != key_states.end())
        return (ks_it->second.isPressed());
    return false;
}

bool KeyHandler::isReleased(LinuxKeyCode keysym) {
    // remember that operator[] inserts a new key if not found
    auto ks_it { key_states.find(keysym) };
    if (ks_it != key_states.end())
        return (ks_it->second.isReleased());
    return true;
}

/*
void KeyHandler::handleKeyEvent(const struct input_event& ev) {
    // f (toggle FPS counter,) m (toggle map overlay,) and p (pause music)
    //   toggle states currently stored as key states instead of State members
    auto ks_it { key_states.find(ev.code) };
    if (ks_it == key_states.end())  // unsupported key
        return;
    ks_it->second.updateState(ev.value);
}
*/

void KeyHandler::getKeyEvents() {
    // select() blocks until there is something to read in fd, but does not
    //   prevent other reads like a blocking read() would.
    // TBD: currently without a time limit select will block the current frame
    //   from completing, even if the process was signaled. Maybe setting a
    //   time limit, and early return from here if time limit exceeded, will
    //   help this? Per man 2 select, return is 0 on timeout, so maybe set a
    //   timeout of the maximum expected frame duration, like .1 sec?
    std::ostringstream error_oss;
    safeCExec(select, "select", (int)-1,
              kbd_device_fd + 1, &rdfds,
              (fd_set*)nullptr, (fd_set*)nullptr, (timeval*)nullptr);
    ssize_t rd { safeCExec(read, "read", (ssize_t)-1,
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
        // TBD: is there a way to standardize handling of ctrl + key?
        if (ev[i].code == KEY_C && ev[i].value == VAL_PRESS) {
            stop = (isPressed(KEY_LEFTCTRL) || isPressed(KEY_RIGHTCTRL));
            continue;
        }
        auto ks_it { key_states.find(ev[i].code) };
        if (ks_it == key_states.end())  // unsupported key
            continue;
        ks_it->second.updateState(ev[i].value);
    }
}

// inspired by https://github.com/kernc/logkeys/blob/master/src/logkeys.cc
//    determine_input_device(), but parses file directly rather than popen(grep...)
// determine likely keyboard device file path via /proc/bus/input/devices
static std::string determineInputDevice(void) {
    // TBD: make filename a macro/constexpr?
    std::ifstream ifs( "/proc/bus/input/devices" );
    std::ostringstream error_msg;
    if (!ifs.is_open()) {
        error_msg << __FUNCTION__ << ": couldn't open /proc/bus/input/devices";
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
            // Further, it looks like cctype or ctype.h has been included by another header
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
                    input_device_path << INPUT_EVENT_PATH << "event" << index;
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

static std:: string generateGrabMessage(std::string exec_filename) {
    std::ostringstream msg;
    static constexpr uint16_t msg_width { 100 };
    static std::string msg_border ( msg_width, '*' );
    static constexpr char msg_line_1_prefix[] { "WARNING! Process " };
    static constexpr char msg_line_1_suffix[] { " is grabbing keyboard events." };
    static constexpr char msg_line_2[] {
        "Please maintain keyboard focus on this tty until ungrab message appears." };
    msg << '\n';
    msg << msg_border << '\n';
    msg << '\t' << msg_line_1_prefix << static_cast<int>(getpid()) <<
        ": " << exec_filename << msg_line_1_suffix << '\n';
    msg << '\t' << msg_line_2 << '\n';
    msg << msg_border << std::endl;
    return msg.str();
}

// check if on pty or tty
static void determineTtys(std::string& exec_filename,
                          std::string& input_tty_name, std::string& display_tty_name) {
    // Use of ttyname taken from coreutils tty, see:
    //  - https://github.com/coreutils/coreutils/blob/master/src/tty.c
    std::string curr_tty_name {
        safeCExec(ttyname, "ttyname", (char*)nullptr, STDIN_FILENO) };
    char *SSH_TTY { getenv("SSH_TTY") };
    display_tty_name = curr_tty_name;

    // Current tty is true tty with hardware access
    if (curr_tty_name.find("tty") != std::string::npos && SSH_TTY == nullptr) {
        input_tty_name = curr_tty_name;
        return;
    }
    // Current tty is pty and/or ssh session
    // Need to find and nominate a true tty for input capture
    std::ofstream ofs;
    std::string candidate_tty_name;
    bool valid_input_tty_found;
    // Use of utmp taken from coreutils who, see:
    //  - https://github.com/coreutils/coreutils/blob/master/src/who.c
    //  - https://github.com/coreutils/gnulib/blob/master/lib/readutmp.h
    struct utmp *ut;
    // Note: man 3 getutent prescribes calling setutent first as a best practice,
    //   but in testing it fails here with ENOENT, so we don't use safeCExecVoidRet
    // Open _PATH_UTMP (eg /var/run/utmp)
    setutent();
    while ((ut = getutent()) != nullptr) {
        // USER_PROCESS: normal process with attached username that is not LOGIN
        if (ut->ut_type == USER_PROCESS) {
            candidate_tty_name.clear();
            candidate_tty_name.append("/dev/");
            candidate_tty_name.append(ut->ut_line);
            if (candidate_tty_name.find("tty") != std::string::npos) {
                ofs.open(candidate_tty_name);
                if (ofs.is_open()) {
                    valid_input_tty_found = true;
                    ofs << generateGrabMessage(exec_filename);
                    ofs.close();
                    break;
                }
            }
        }
    }
    // close _PATH_UTMP
    safeCExecVoidRet(endutent, "endutent");

    if (!valid_input_tty_found) {
        std::ostringstream error_msg;
        error_msg << __FUNCTION__ << ": No non-pty tty found, cannot capture " <<
            " any keyboard events" << std::endl;
        throw std::runtime_error(error_msg.str());
    }
    input_tty_name = candidate_tty_name;
}

void KeyHandler::grabDevice(std::string& exec_filename) {
    std::ostringstream error_msg;

    kbd_device_fd = safeCExec(open, "open", (int)-1,
                              kbd_device_path.c_str(), O_RDONLY);
    try {
        safeCExec(ioctl, "ioctl", (int)-1, kbd_device_fd, EVIOCGRAB, (void*)1);
    } catch (std::runtime_error& re) {
        safeCExec(ioctl, "ioctl", (int)-1, kbd_device_fd, EVIOCGRAB, (void*)0);
        throw re;
    }
    std::ofstream ofs;
    ofs.open(input_tty_name);
    if (ofs.is_open()) {
        ofs << generateGrabMessage(exec_filename);
        ofs.close();
    }
}

void KeyHandler::initialize(std::string& exec_filename) {
    kbd_device_path = determineInputDevice();
    std::cout << "Selected keyboard device path: " << kbd_device_path << '\n';

    grabDevice(exec_filename);

    // fd_set used by select() in getKeyEvents
    FD_ZERO(&rdfds);
    FD_SET(kbd_device_fd, &rdfds);

    determineTtys(exec_filename, input_tty_name, display_tty_name);
    std::cout << "Terminal focus for input: " << input_tty_name << "\n";
    std::cout << "Terminal output sent to: " << display_tty_name << "\n";
}
