#include "App.hh"
#include "WindowMgr.hh"
#include "SdlWindowMgr.hh"
#include "Settings.hh"     // TtyDisplayMode
#include "Xterm.hh"        // CtrlSeqs

#include <getopt.h>        // option getopt_long optind
#include <ctype.h>         // tolower

#include <iostream>
#include <string>


// static contexpr class members in C++11 require declaration outside of the
//   class, doing so once here follows ODR
constexpr std::array<const char*, 10> WindowMgr::wall_tex_paths;

constexpr char SdlWindowMgr::SKY_TEX_PATH[];
constexpr char SdlWindowMgr::FONT_PATH[];


enum class IoMode  { Uninitialized, Tty, Sdl };

/**
 * @brief generate formatted usage message
 *
 * @param exec_filename - executable file name
 */
static void printUsage(const char* exec_filename) {
    std::cerr << "\nUsage: " << exec_filename << "...\n\n" <<
        "\t-m=mapfile\n" <<
        "\t--map=mapfile\t Opens mapfile path to initialize maze map (required)\n" <<
        "\n" <<
        "\t-X\n" <<
        "\t--SDL\t\t Display and keyboard capture via SDL2/X11\n" <<
        "\t\t\t (game X11 window will open)\n" <<
        "\n" <<
        "\t-t(=ttymode)\n" <<
        "\t--tty(=ttymode)\t Display via tty, keyboard capture through direct device grab\n" <<
        "\t\t\t (requires running as root user or as member of input group)\n" <<
        "\t\t\t ttymode determines pixel rendering as:\n" <<
        "\t\t\t   ascii: monochrome characters (default)\n" <<
        "\t\t\t   code/256color/1byte: terminal background colors (256 color mode)\n" <<
        "\t\t\t   rgb/truecolor/3byte: terminal background colors (true (RGB) color mode)\n" <<
        std::endl;
}

/**
 * @brief parse command line options
 *
 * @param argc             - command token count
 * @param argv             - array of command tokens (char* const* expected by
 *                             getopt_long)
 * @param map_filename     - set by reference to layout map file name
 * @param io_mode          - enum set by reference to tty or sdl mode
 * @param tty_display_mode - enum set by reference to tty pixel mode
 *
 * @return 0 on success, 1 on failure
 */
static int getOptions(const int argc, char* const argv[],
                      std::string& map_filename, IoMode& io_mode,
                      TtyDisplayMode& tty_display_mode) {
    constexpr struct option long_options[] {
        {"SDL", no_argument,       nullptr, 'X' },
        {"tty", optional_argument, nullptr, 't' },
        {"map", required_argument, nullptr, 'm' },
        {nullptr, 0, 0, 0 }   // required sentinel with null name field
    };
    constexpr char optstring[] { "Xt::m:" };

    int c;
    int option_i { 0 };
    while ( (c = getopt_long(argc, argv, optstring,
                             long_options, &option_i)) != -1 ) {
        switch (c) {
        case 'X':
            if (io_mode == IoMode::Tty) {
                std::cerr << argv[0] << ": Please choose either tty or SDL mode.\n";
                return 1;
            }
            io_mode = IoMode::Sdl;
            break;

        case 't':
        {
            if (io_mode == IoMode::Sdl) {
                std::cerr << argv[0] << ": Please choose either tty or SDL mode.\n";
                return 1;
            }
            io_mode = IoMode::Tty;
            // if `-t=`(arg), need to trim leading '='
            char* trimmed_optarg {
                (optarg && optarg[0] == '=') ? optarg + 1 : optarg };
            // string from nullptr is undefined behavior
            std::string optarg_s { trimmed_optarg ? trimmed_optarg : "" };
            for (auto& c : optarg_s)
                c = std::tolower(c);
            if (optarg_s == "ascii") {
                tty_display_mode = TtyDisplayMode::Ascii;
            } else if (optarg_s == "code" || optarg_s == "256color" ||
                optarg_s == "1byte") {
                tty_display_mode = TtyDisplayMode::ColorCode;
            } else if (optarg_s == "rgb" || optarg_s == "truecolor" ||
                       optarg_s == "3byte") {
                tty_display_mode = TtyDisplayMode::TrueColor;
            } else if (optarg_s != "") {
                std::cerr << argv[0] << ": Unrecognized tty display mode: \"" <<
                    optarg_s << "\".\n";
                return 1;
            }
        }
            break;
        case 'm':
            map_filename = optarg;
            break;
        case 0:    // long option parsed
            // should only return if longopts member without option.val matched
        case '?':  // unknown option
        default:
            std::cerr << argv[0] << ": Unable to parse options.\n";
            return 1;
        }
    }

    if (map_filename.size() == 0) {
        std::cerr << argv[0] << ": No map file path given.\n";
        return 1;
    }

    // no non-option argv members expected
    if (optind < argc) {
        std::cerr << argv[0] << ": Unrecognized parameter: " << argv[optind] << "\n";
        return 1;
    }

    return 0;
}

/**
 * @brief entry point
 *
 * @param argc - command token count
 * @param argv - array of command tokens (char* const* expected by getOptions)
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int main(const int argc, char* const argv[]) {
    std::string map_filename;
    IoMode io_mode  { IoMode::Uninitialized };
    TtyDisplayMode tty_display_mode { TtyDisplayMode::Uninitialized };

    if (getOptions(argc, argv, map_filename, io_mode, tty_display_mode) != 0) {
        printUsage(argv[0]);
        return (EXIT_FAILURE);
    }

    if (io_mode == IoMode::Uninitialized)
        io_mode = IoMode::Sdl;
    if (io_mode == IoMode::Tty && tty_display_mode == TtyDisplayMode::Uninitialized)
        tty_display_mode = TtyDisplayMode::Ascii;

    try {
        App app(argv[0], map_filename, (io_mode == IoMode::Tty), tty_display_mode);
        app.run();
    } catch (const std::exception& e) {
        // sudden failure of app in tty mode can leave cursor hidden or terminal
        //   bg color set
        if (io_mode == IoMode::Tty) {
            std::cout << Xterm::CtrlSeqs::CharDefaults() <<
                Xterm::CtrlSeqs::ShowCursor();
        }
        throw e;
    }
}
