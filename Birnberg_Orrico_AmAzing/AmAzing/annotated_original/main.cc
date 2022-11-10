#include "App.hh"

#include <cstdlib>  // EXIT_FAILURE

#include <iostream>


int main(int argc, const char * argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <map_filename>" << std::endl;
        return EXIT_FAILURE;
    }

    App app;
    app.run(argv[1]);
}
