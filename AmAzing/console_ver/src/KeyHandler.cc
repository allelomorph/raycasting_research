#include "KeyHandler.hh"
#include "safeCExec.hh"
#include "App.hh"         // stop

#include <linux/input.h>   // input_event EV_* KEY_*
#include <sys/ioctl.h>     // ioctl EVIOCGRAB
#include <unistd.h>        // uid_t gid_t get(set)egid get(set)euid getpid
#include <pwd.h>           // passwd getpwnam
#include <utmp.h>          // utmp setutent getutent endutent USER_PROCESS
#include <sys/select.h>    // FD_ZERO FD_SET
#include <sys/types.h>     // open gid_t uid_t pid_t
#include <sys/stat.h>      // open
#include <fcntl.h>         // open

#include <cstdio>         // popen FILE perror feof fgets pclose
#include <cstdlib>        // atoi exit getenv
#include <cctype>         // isdigit tolower

#include <vector>
#include <string>         // getline
#include <algorithm>      // transform max_element
#include <array>
#include <fstream>
#include <iostream>       // cerr


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
    // select blocks until there is something to read in fd, but does not prevent
    //   other reads like a blocking read() would
    select(kbd_fd + 1, &rdfds, NULL, NULL, NULL);
    // TBD: this needed? `if (stop) break;`
    ssize_t rd { safeCExec(read, "read", (ssize_t)-1, kbd_fd, ev, sizeof(ev)) };
    if (rd < (int) sizeof(struct input_event)) {
        std::cerr << "expected " << sizeof(struct input_event) <<
            " bytes, got " << rd << ", read: " << strerror(errno);
        // TBD: fail out condition?
        return;
    }
    std::size_t input_event_ct { rd / sizeof(struct input_event) };
    for (std::size_t i {0}; i < input_event_ct; ++i) {
        // TBD: other input events ignored for now
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

/**
 * Test ability to grab keyboard device; ungrab on failure.
 *
 * @param fd The file descriptor to the keyboard device
 * @return 0 if the grab was successful, or 1 otherwise.
 */
static int testGrabDevice(int fd) {
    int rc { ioctl(fd, EVIOCGRAB, (void*)1) };
    if (rc != 0)
        ioctl(fd, EVIOCGRAB, (void*)0);
    return rc;
}


// forks into child, executes cmd and returns string output
// adapted from https://github.com/kernc/logkeys/blob/master/src/logkeys.cc execute()
static std::string getShellCmdOutput(const char* cmd) {
    // Likely running main either as root or as member of `input` group
    // man getegid(2), geteuid(2): "These functions are always successful."
    gid_t gid { getegid() };
    uid_t uid { geteuid() };

    std::string result;
    std::string error_msg;

    try {
        // For safety, while running other programs, switch user to nobody
        // Note: only priveledged users can set gid and uid, expecting root
        struct passwd* nobody_pwd {
            safeCExec(getpwnam, "getpwnam", (struct passwd*)nullptr, "nobody") };
        safeCExec(setegid, "setegid", (int)-1, nobody_pwd->pw_gid);
        safeCExec(seteuid, "seteuid", (int)-1, nobody_pwd->pw_uid);

        // forks into child and returns cmd output on "r"
        FILE* pipe { safeCExec(popen, "popen", (FILE*)nullptr, cmd, "r") };
        char buffer[128];
        while (!std::feof(pipe)) {
            if (std::fgets(buffer, 128, pipe) != nullptr)
                result += buffer;
        }
        safeCExec(pclose, "pclose", (int)-1, pipe);
    } catch (std::exception &e) {
        error_msg.append(e.what());
    }

    // restore original user and group
    safeCExec(setegid, "setegid", (int)-1, gid);
    safeCExec(seteuid, "seteuid", (int)-1, uid);

    // __FUNCTION__ should work with clang, gcc, and MSVC
    if (error_msg != "") {
        error_msg.insert(0, ": ");
        error_msg.insert(0, __FUNCTION__);
        throw std::runtime_error(error_msg);
    }
    return result;
}

#define EXE_GREP           "/bin/grep"
#define INPUT_EVENT_PATH   "/dev/input/"
// adapted from https://github.com/kernc/logkeys/blob/master/src/logkeys.cc determine_input_device()
static std::string determineInputDevice(void) {
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

// check if on pty or tty
static void determineCaptureTty(std::string exec_filename) {
    // Use of ttyname taken from coreutils tty, see:
    //  - https://github.com/coreutils/coreutils/blob/master/src/tty.c
    std::string curr_tty_name {
        safeCExec(ttyname, "ttyname", (char*)nullptr, STDIN_FILENO) };
    char *SSH_TTY { getenv("SSH_TTY") };

    // Current tty is true tty with hardware access
    if (curr_tty_name.find("tty") != std::string::npos && SSH_TTY == nullptr) {
        std::cout << "Terminal focus for input: " << curr_tty_name << " (current)\n";
        std::cout << "Terminal output sent to: " << curr_tty_name << " (current)\n";
        return;
    }

    // Current tty is pty and/or ssh session
    // Need to find and nominate a true tty for input capture
    std::ofstream ofs;
    uint16_t msg_width { 100 };
    std::string msg_border ( msg_width, '*' );
    constexpr char msg_line_1_prefix[] { "WARNING! Process " };
    constexpr char msg_line_1_suffix[] { " is grabbing keyboard events."};
    constexpr char msg_line_2[] {
        "Please maintain keyboard focus on this tty until ungrab message appears." };

    std::string candidate_tty_name;
    bool valid_input_tty_found;
    // Use of utmp taken from coreutils who, see:
    //  - https://github.com/coreutils/coreutils/blob/master/src/who.c
    //  - https://github.com/coreutils/gnulib/blob/master/lib/readutmp.h
    struct utmp *ut;
    // Open _PATH_UTMP (eg /var/run/utmp)
    // Note: in testing setutent will set ENOENT even on success, so we do not use safeCExecVoidRet
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
                    ofs << '\n';
                    ofs << msg_border << '\n';
                    ofs << '\t' << msg_line_1_prefix << static_cast<int>(getpid()) <<
                        ": " << (exec_filename.c_str() + 2) << msg_line_1_suffix << '\n';
                    ofs << '\t' << msg_line_2 << '\n';
                    ofs << msg_border << std::endl;
                    ofs.close();
                    break;
                }
            }
        }
    }
    // close _PATH_UTMP
    safeCExecVoidRet(endutent, "endutent");

    if (!valid_input_tty_found) {
        std::cout << "No non-pty tty found, cannot capture any keyboard events!\n";
        exit(EXIT_FAILURE);
    }
    std::cout << "Terminal focus for input: " << candidate_tty_name << "\n";
    std::cout << "Terminal output sent to: " << curr_tty_name << " (current)\n";
}

void KeyHandler::initialize(std::string exec_filename) {
    // determine likely keboard device file path via /proc/bus/input/devices
    kbd_device_path = determineInputDevice();
    std::cout << "Selected keyboard device path: " << kbd_device_path << '\n';
    kbd_fd = safeCExec(open, "open", (int)-1, kbd_device_path.c_str(), O_RDONLY);
    if (testGrabDevice(kbd_fd) != 0) {
        std::cerr << "Failure to grab device \"" << kbd_device_path <<
            "\": check that it is not already grabbed by X." <<
            std::endl;
        exit(EXIT_FAILURE);
    }

    determineCaptureTty(exec_filename);

    FD_ZERO(&rdfds);     // init fd_set
    FD_SET(kbd_fd, &rdfds);  // add kbd_fd to read fd set
}
