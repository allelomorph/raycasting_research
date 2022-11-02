#include "include/App.hh"

#include <getopt.h>  // option getopt_long optind

#include <iostream>

enum class IoMode { Uninitialized, Tty, SDL };

static void printUsage(const char *exec_filename) {
    std::cerr << "\nUsage: " << exec_filename << "...\n\n" <<
        "\t-m=mapfile\n" <<
        "\t--map=mapfile\tOpens `mapfile` path to initialize maze map (required)\n" <<
        "\n" <<
        "\t-t\n" <<
        "\t--tty\t\tDisplay via tty, keyboard capture through direct device grab\n" <<
        "\t\t\t  (requires running as root user or as member of input group)\n" <<
        "\n" <<
        "\t-X\n" <<
        "\t--SDL\t\tDisplay and keyboard capture via SDL2/X11\n" <<
        "\t\t\t  (game X11 window will open)\n" <<
        std::endl;
}

static int getOptions(int argc, char* const argv[],
                       std::string& map_filename, IoMode& io_mode) {
    // colon after opt char signifies that it takes an arg
    constexpr char optstring[] { "Xtm:" };
    constexpr struct option long_options[] {
        {"SDL",     no_argument,       nullptr,  'X' },
        {"tty",     no_argument,       nullptr,  't' },
        {"map",     required_argument, nullptr,  'm' },
        {nullptr, 0, 0, 0 }   // required sentinel with null name field
    };

    for (int c, option_i { 0 };
         (c = getopt_long(
             argc, argv, optstring, long_options, &option_i)) != -1; ) {
        switch (c) {
        case 'X':
            if (io_mode != IoMode::Uninitialized) {
                std::cerr << argv[0] << ": Please choose either tty or SDL mode.\n";
                return 1;
            }
            io_mode = IoMode::SDL;
            break;

        case 't':
            if (io_mode != IoMode::Uninitialized) {
                std::cerr << argv[0] << ": Please choose either tty or SDL mode.\n";
                return 1;
            }
            io_mode = IoMode::Tty;
            break;

        case 'm':
            map_filename += optarg;
            break;

        case 0:    // long option parsed
            // should only return if longopts member without option.val matched
        case '?':  // unknown option
        default:
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
    IoMode io_mode { IoMode::Uninitialized };
    if (getOptions(argc, argv, map_filename, io_mode) != 0) {
        printUsage(argv[0]);
        return (EXIT_FAILURE);
    }

    if (io_mode == IoMode::SDL) {
        std::cerr << argv[0] << ": SDL mode not implemented yet!" << std::endl;
        return (EXIT_FAILURE);
    }

    // TBD: currently defaults to tty, eventually to SDL
    App app(argv[0], map_filename.c_str()/*, io_mode*/);
    app.run();
}
