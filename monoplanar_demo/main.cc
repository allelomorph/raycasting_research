#include "App.hh"
#include "Settings.hh"     // TtyDisplayMode

#include <getopt.h>        // option getopt_long optind
#include <ctype.h>         // tolower

#include <iostream>
#include <string>


enum class IoMode  { Uninitialized, Tty, Sdl };

static void printUsage(const char *exec_filename) {
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
        "\t\t\t   256color/colorcode/1byte: terminal background colors (256 color mode)\n" <<
        "\t\t\t   truecolor/rgb/3byte: terminal background colors (true (RGB) color mode)\n" <<
        std::endl;
}

static int getOptions(int argc, char* const argv[],
                      std::string& map_filename, IoMode& io_mode,
                      TtyDisplayMode& tty_display_mode) {
    // colon after opt char signifies that it takes an arg
    constexpr char optstring[] { "Xtm:" };
    constexpr struct option long_options[] {
        {"SDL",     no_argument,       nullptr,  'X' },
        {"tty",     optional_argument, nullptr,  't' },
        {"map",     required_argument, nullptr,  'm' },
        {nullptr, 0, 0, 0 }   // required sentinel with null name field
    };

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
            // string from nullptr is undefined behavior
            std::string optarg_s { optarg ? optarg : "" };
            for (auto& c : optarg_s)
                c = std::tolower(c);
            if (optarg_s == "ascii") {
                tty_display_mode = TtyDisplayMode::Ascii;
            } else if (optarg_s == "256color" || optarg_s == "colorcode" ||
                optarg_s == "1byte") {
                tty_display_mode = TtyDisplayMode::ColorCode;
            } else if (optarg_s == "truecolor" || optarg_s == "rgb" ||
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

int main(int argc, char* const argv[]) {
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

    // TBD: wrap in try/except to do emergency cleanup on throwing
    //   (eg ungrab keyboard device and ensure cursor is not hidden when in tty mode)
    App app(argv[0], map_filename, (io_mode == IoMode::Tty), tty_display_mode);
    app.run();
}
