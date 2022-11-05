#include "App.hh"
#include "Matrix.hh"      // Matrix2d Vector2d

#include <time.h>         // clock clock_t CLOCKS_PER_SEC
#include <csignal>        // signal SIG* sig_atomic_t
#include <linux/input.h>  // KEY_*

#include <iostream>
#include <iomanip>        // setw
#include <string>

#include <sys/types.h>    // pid_t
#include <unistd.h>       // getpid

#include <cmath>          // sin cos

//#include <functional>  // ref see TBD in initialize


void FpsCalculator::initialize() {
    prev_timepoint = clock();
}

void FpsCalculator::calculate() {
    double frame_duration;

    curr_timepoint = clock();
    frame_duration = ((double(curr_timepoint - prev_timepoint)) / CLOCKS_PER_SEC);
    prev_timepoint = curr_timepoint;
    moving_avg_frame_time = (moving_avg_frame_time * 19.0 / 20.0) +
        (frame_duration / 20.0);
}


App::App(const char* efn, const char* mfn) :
    exec_filename(efn), map_filename(mfn) {
    state = State::getInstance();
}

App::~App() {
    if (state != nullptr)
        delete state;
}

// Needs to be global to be visible to signal
// TBD: find a more idiomatic STL signal handling
volatile std::sig_atomic_t stop = 0;

// TBD: due to getEvents->getKeyEvents blocking on select(), program will hang when killed until some input events are read in that frame
static void interrupt_handler(int /*signal*/) {
    stop = 1;
}

void App::initialize() {
    // parse map file, also sets pos
    try {
        // TBD: why std::ref to create a reference_wrapper here?
        state->layout = new Layout(map_filename, /*std::ref(*/state->pos/*)*/);
    } catch (std::runtime_error &re) {
        std::cerr << re.what() << std::endl;
        state->done = true;
        return;
    }

    state->dir << 0, 1;
    state->viewPlane << 2.0/3, 0;

    fps_calc.initialize();
    state->key_handler.initialize(exec_filename);

    signal(SIGINT, interrupt_handler);
    signal(SIGTERM, interrupt_handler);
}

void App::run() {
    initialize();

    while(!stop && !state->done) {
        fps_calc.calculate();
        getEvents();
        updateData(/*fps_calc.moving_avg_frame_time*/);

        printDebugHUD();
    }

    // erases last frame printed to terminal
    // TBD: make function and/or coordinate with screen height macro for CSI_CURSOR_UP
    constexpr uint32_t display_lines { 5 };
    for (uint32_t i { 0 }; i < display_lines; ++i)
        std::cout << std::string(100, ' ') << '\n';
    // TBD: macro can't take variable name
    std::cout << CSI_CURSOR_UP(5);
}

void App::getEvents() {
    state->key_handler.getKeyEvents();
}

static Vector2d rotate2d(Vector2d vector, double rotSpeed) {
    Matrix2d rotate;
    rotate <<
        std::cos(rotSpeed), -std::sin(rotSpeed),
        std::sin(rotSpeed), std::cos(rotSpeed);
    return (rotate * vector);
}

void App::updateData() {
    // TBD: rationale for these calcs?
    double moveSpeed { fps_calc.moving_avg_frame_time * 4 };
    double rotSpeed  { fps_calc.moving_avg_frame_time * 2 };

    // q or escape keys: quit
    if (state->key_handler.isPressed(KEY_Q) ||
        state->key_handler.isPressed(KEY_ESC)) {
        state->done = true;
        return;
    }

    // left arrow key: rotate left
    if (state->key_handler.isPressed(KEY_LEFT)) {
        state->dir = rotate2d(state->dir, rotSpeed);
        state->viewPlane = rotate2d(state->viewPlane, rotSpeed);
    }

    // right arrow key: roatate right
    if (state->key_handler.isPressed(KEY_RIGHT)) {
        state->dir = rotate2d(state->dir, -rotSpeed);
        state->viewPlane = rotate2d(state->viewPlane, -rotSpeed);
    }

    // up arrow key: move forward
    if (state->key_handler.isPressed(KEY_UP)) {
        double tmp = state->pos(0);
        if (!state->layout->map[int(state->pos(0) + state->dir(0) * moveSpeed)][int(state->pos(1))]) {
            state->pos(0) += state->dir(0) * moveSpeed;
        }
        if (!state->layout->map[int(tmp)][int(state->pos(1) + state->dir(1) * moveSpeed)]) {
            state->pos(1) += state->dir(1) * moveSpeed;
        }
    }

    // down arrow key: move backward
    if (state->key_handler.isPressed(KEY_DOWN)) {
        double tmp = state->pos(0);
        if (!state->layout->map[int(state->pos(0) - state->dir(0) * moveSpeed)][int(state->pos(1))])
            state->pos(0) -= state->dir(0) * moveSpeed;
        if (!state->layout->map[int(tmp)][int(state->pos(1) - state->dir(1) * moveSpeed)])
            state->pos(1) -= state->dir(1) * moveSpeed;
    }

    // a key: move left (strafe)
    if (state->key_handler.isPressed(KEY_A)) {
        double tmp = state->pos(0);
        if (!state->layout->map[int(state->pos(0) - state->dir(1) * moveSpeed)][int(state->pos(1))])
            state->pos(0) -= state->dir(1) * moveSpeed;
        if (!state->layout->map[int(tmp)][int(state->pos(1) + state->dir(0) * moveSpeed)])
            state->pos(1) += state->dir(0) * moveSpeed;
    }

    // d key: move right (strafe)
    if (state->key_handler.isPressed(KEY_D)) {
        double tmp = state->pos(0);
        if (!state->layout->map[int(state->pos(0) + state->dir(1) * moveSpeed)][int(state->pos(1))])
            state->pos(0) += state->dir(1) * moveSpeed;
        if (!state->layout->map[int(tmp)][int(state->pos(1) - state->dir(0) * moveSpeed)])
            state->pos(1) -= state->dir(0) * moveSpeed;
    }

    // f key: toggle FPS
    state->showFPS = state->key_handler.isPressed(KEY_F);

    // m key: toggle map overlay
    state->showMap = state->key_handler.isPressed(KEY_M);
}


void App::printDebugHUD() {
    std::cout << std::boolalpha << "State:\n";
    std::cout << "\tflags: done: " << std::setw(5) << state->done <<
        " showFPS: " << std::setw(5) << state->showFPS <<
        " showMap: " << std::setw(5) << state->showMap << '\n';
    std::cout << "\tpos: " << state->pos << " dir: " << state->dir <<
        " viewPlane: " << state->viewPlane << '\n';
    std::cout << "FPS: " << (1 / fps_calc.moving_avg_frame_time) << '\n';

    for (const auto& pair : state->key_handler.key_states) {
        auto key { pair.second };
        std::cout /*<< std::setw(6)*/ << pair.second.repr << ": " << std::noboolalpha << key.isPressed() << ' ';
    }
    std::cout << '\n' << CSI_CURSOR_UP(5);
}
