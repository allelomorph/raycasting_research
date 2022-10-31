#include "include/App.hh"
#include <iostream>

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <map_filename>" << std::endl;
        return EXIT_FAILURE;
    }

    App app(argv[0], argv[1]);
    app.run();
}
