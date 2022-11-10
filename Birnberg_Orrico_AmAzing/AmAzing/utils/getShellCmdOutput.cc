#include "safeCExec.hh"

#include <unistd.h>        // get(set)egid get(set)euid
#include <pwd.h>           // passwd getpwnam
#include <sys/types.h>     // gid_t uid_t pid_t

#include <cstdio>          // popen FILE feof fgets pclose

#include <string>
#include <sstream>


// forks into child, executes cmd and returns string output
// adapted from https://github.com/kernc/logkeys/blob/master/src/logkeys.cc execute()
// TBD: sete(gid|uid) requires root
std::string getShellCmdOutput(const char* cmd) {
    // man getegid(2), geteuid(2): "These functions are always successful."
    gid_t gid { getegid() };
    uid_t uid { geteuid() };

    std::string result;
    std::ostringstream error_msg;
    error_msg << __FUNCTION__ << ": ";
    bool error { false };

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
        error = true;
        error_msg << e.what();
    }

    // restore original user and group
    safeCExec(setegid, "setegid", (int)-1, gid);
    safeCExec(seteuid, "seteuid", (int)-1, uid);

    if (error)
        throw std::runtime_error(error_msg.str());
    return result;
}
