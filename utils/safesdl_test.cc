#include "safeSDLExec.hh"

#include <iostream>

#include "typeName.hh"

int main() {
    safeSDLExec(SDL_Init, "SDL_Init",
                static_cast<bool (*)(int)>([](int ret){ return (ret != 0); }),
                0);
    SDL_Quit();
}
