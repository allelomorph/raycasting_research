#include "App.hh"            // WINDOW_WIDTH WINDOW_HEIGHT

#include <SDL2/SDL.h>        // SDL_*
#include <SDL2/SDL_mixer.h>  // Mix_*
#include <SDL2/SDL_ttf.h>    // TTF_*
#include <SDL2/SDL_image.h>  // IMG_*

#include <time.h>            // clock clock_t CLOCKS_PER_SEC

#include <cmath>             // floor

#include <algorithm>         // for_each
#include <sstream>           // ostringstream


App::App() {
    state = State::getInstance();
}

App::~App() {
    delete state;
    // TBD: reorder these to match inverse of init process?
    // safe to pass nullptr per https://wiki.libsdl.org/SDL_FreeSurface
    // void return, no error checking
    if (buffer)
        SDL_FreeSurface(buffer);
    // TBD: SDL_DestroyTexture sets errors but returns void, can't use safeSdlExec
    if (buffer_texture)
        SDL_DestroyTexture(buffer_texture);
    std::for_each(textures.begin(), textures.end(),
                  [] (SDL_Surface *s) { if (s != nullptr) SDL_FreeSurface(s); });
    if (sky)
        SDL_DestroyTexture(sky);
    // void return for remaining functions, so no error checking
    if (music)
        Mix_FreeMusic(music);
    Mix_Quit();
    SDL_Quit();
    TTF_Quit();
    IMG_Quit();
}

// TBD: there is no need to return a bool, as function can only return true
void App::run(std::string filename) {
    initialize(filename);

    // TBD: convert to idiomatic C++ time calculation vs libc
    // TBD: encapsulate in FPSCalc class
    // Init FPS calculation
    clock_t curr_tp, prev_tp;
    double moving_avg_frame_length = 0.015;
    prev_tp = clock();

    Mix_FadeInMusic(music, -1, 3000);

    while(!state->done) {
        // render backdrop
        // "Copy a portion of the texture to the current rendering target"
        if (sky)  // this->sky
            SDL_RenderCopy(state->renderer, sky, nullptr, nullptr);

        render3d();

        if (state->show_map)
            render2d();

        // FPS calculation per frame
        curr_tp = clock();
        double frame_length { ((double(curr_tp - prev_tp)) / CLOCKS_PER_SEC) };
        prev_tp = curr_tp;
        moving_avg_frame_length =
            (moving_avg_frame_length * 19.0 / 20.0) + (frame_length / 20.0);
        // update FPS HUD
        if (state->show_fps)
            displayFPS(1 / moving_avg_frame_length);

        // "Update the screen with any rendering performed since the previous call."
        SDL_RenderPresent(state->renderer);

        // poll user input events
        getEvents();

        // update State based on input
        updateData(moving_avg_frame_length);
    }
}

// TBD: font_filename can be a const char*
// initializes font and texture for FPS counter in HUD, helper to initialize
void App::makeGlyphs(std::string font_filename) {
    // TBD: per https://wiki.libsdl.org/SDL_LOG_CATEGORY, it looks like SDL keeps
    //   multiple logs, so maybe this call of SDL_LogError promotes the error
    //   to the main application-level log?
    // TBD: standardize exception on error with safeSDLExec
    if (TTF_Init() != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't initialize true type font system: %s", TTF_GetError());
        throw std::runtime_error("TTF_Init failed");
    }

    // Render FPS HUD prefix text to a new ARGB surface
    // TBD: was this originally goiing to populate App.font?
    TTF_Font *font { TTF_OpenFont(font_filename.c_str(), 24) };
    if (font == nullptr)
        throw std::runtime_error("TTF_OpenFont failed");
    SDL_Color fg { 0xff, 0xff, 0xff, 0xff };  // text foreground color, pure white
    SDL_Surface *text_surface { TTF_RenderText_Blended(font, "FPS: ", fg) };
    if (text_surface == nullptr)
        throw std::bad_alloc();
    state->fps_texture = SDL_CreateTextureFromSurface(state->renderer, text_surface);
    SDL_FreeSurface(text_surface);
    if (state->fps_texture == nullptr)
        throw std::bad_alloc();

    // init font cache with numeric chars for live FPS display
    for (const char &c : std::string(".0123456789")) {
        SDL_Surface *glyph_surface { TTF_RenderGlyph_Blended(font, c, fg) };
        if (glyph_surface == nullptr)
            throw std::bad_alloc();
        SDL_Texture *glyph_texture { SDL_CreateTextureFromSurface(state->renderer, glyph_surface) };
        SDL_FreeSurface(glyph_surface);
        if (glyph_texture == nullptr)
            throw std::bad_alloc();
        state->font_cache[c] = glyph_texture;
    }
}

void App::initialize(std::string filename) {
    // parse map file into unit grid
    // TBD: why does the original pass std::ref?
    // state->layout = new Layout(filename, std::ref(state->pos));
    // TBD: pointer to allow for reinit? why not use normal scope destruction?
    state->layout = new Layout(filename, state->pos);

    // init raycasting variables
    state->dir << 0, 1;
    state->view_plane << 2.0/3, 0;

    // init SDL window and renderer
    // As with TTF_Init in makeGlyphs, the authors elect to log many of
    //   these init failures at the application log level
    // TBD: research best practices on SDL error logging
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't initialize SDL: %s", SDL_GetError());
        throw std::runtime_error("SDL_Init failed");
    }
    state->window = SDL_CreateWindow(
        "AmAzing",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE);
    if (state->window == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't create window: %s", SDL_GetError());
        throw std::bad_alloc();
    }
    state->renderer = SDL_CreateRenderer(
        state->window, -1, SDL_RENDERER_ACCELERATED);
    if (state->renderer == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't create renderer: %s", SDL_GetError());
        throw std::bad_alloc();
    }

    // init state->fps_texture, state->font_cache
    // TBD: make file path into header constexpr
    makeGlyphs("AmAzing/fonts/Courier New.ttf");

    buffer = SDL_CreateRGBSurfaceWithFormat(  // this->buffer
        0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_PIXELFORMAT_BGRA32);
    if (buffer == nullptr)
        throw std::bad_alloc();
    buffer_texture = SDL_CreateTexture(       // this->buffer_texture
        state->renderer, buffer->format->format, SDL_TEXTUREACCESS_STREAMING,
        WINDOW_WIDTH, WINDOW_HEIGHT);
    if (buffer_texture == nullptr)
        throw std::bad_alloc();
    // fails with non-0 return
    SDL_SetTextureBlendMode(buffer_texture, SDL_BLENDMODE_BLEND);

    // init texture access
    if (IMG_Init(IMG_INIT_JPG) != IMG_INIT_JPG)
        throw std::runtime_error("Texture initialization failed");
    // TBD: no error checking for IMG_Load
    // TBD: make texture paths macros/constexprs in header
    textures[1] = IMG_Load("AmAzing/images/wood.jpg");
    textures[2] = IMG_Load("AmAzing/images/metal.jpg");
    textures[3] = IMG_Load("AmAzing/images/curtain.jpg");
    textures[4] = IMG_Load("AmAzing/images/stone_moss.jpg");
    textures[5] = IMG_Load("AmAzing/images/bark.jpg");
    textures[6] = IMG_Load("AmAzing/images/privat_parkering.jpg");
    textures[7] = IMG_Load("AmAzing/images/grass.jpg");
    textures[8] = IMG_Load("AmAzing/images/lava.jpg");

    // init sky plane
    SDL_Surface *sky_surface { IMG_Load("AmAzing/images/Vue1.jpg") };
    if (sky_surface == nullptr)
        throw std::bad_alloc();

    // TBD: remaining SDL_* calls have no error checking
    sky = SDL_CreateTextureFromSurface(state->renderer, sky_surface);  // this->sky
    SDL_SetTextureBlendMode(sky, SDL_BLENDMODE_BLEND);
    SDL_FreeSurface(sky_surface);

    // init music
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) < 0)
        throw std::bad_alloc();
    // TBD: make file path into header constexpr
    music = Mix_LoadMUS("AmAzing/audio/Game_of_Thrones.wav");  // this->music
    if (music == nullptr)
        throw std::runtime_error("Could not load music");
}

// @x               current horizontal pixel index in display window
// @side            TBD: intersection with wall at horizontal or vertical edge?
// @line_height     height of wall strip in pixels
// @perp_wall_dist  TBD: why perp? distance from player to wall intersection
// @draw_start      top y coordinate of wall segment
// @draw_end        bottom y coordinate of wall segment
// @ray             vector direction of ray to cast
// @map_pos         TBD: coordinates of ray/wall intersection?
void App::drawTexture(int x, int side, int line_height, double perp_wall_dist,
                      int draw_start, int draw_end,
                      Vector2d& ray, Vector2i& map_pos) {
    // OC: calculate value of wall_x; where exactly the wall was hit
    double wall_x { (side == 1) ?
        state->pos(0) + perp_wall_dist * ray(0) :
        state->pos(1) + perp_wall_dist * ray(1) };
    // we want a fraction of a map grid unit, as the intersection is likely not
    //   at a vertex
    wall_x -= floor(wall_x);

    // x pixel coordinate in texture
    // TBD: should 256 instead be a constant, or set by analyzing texture image size?
    //   (all wall textures originally in images/ are indeed 256px * 256px)
    // Per https://wiki.libsdl.org/SDL_Surface, we could get the dimensions of
    //   every texture by checking each app.textures[n]->w (or ->h)
    int tex_x = floor(wall_x * 256);
    if ((side == 0 && ray(0) > 0) || (side == 1 && ray(1) < 0))
        tex_x = 256 - tex_x - 1;

    // draw vertical segment of wall texture
    for (int y {draw_start}; y < draw_end; ++y) {
        // TBD: d for delta? note: narrowing conversion from unsigned int
        int d     ( (y + (line_height - WINDOW_HEIGHT) / 2) * 256 );
        // y pixel coordinate in texture
        int tex_y { ((d * 256) / line_height) / 256 };
        // TBD: this is only use of map_pos, could we pass a texture pointer as a param instead?
        SDL_Surface* texture { textures[state->layout->map[map_pos(0)][map_pos(1)]] };
        uint32_t color { ((uint32_t *)texture->pixels)[tex_y * 256 + tex_x] };
        // OC: make color darker for y-sides: R, G and B byte each divided
        //   through two with a "shift" and an "and"
        // TBD: y-sides refer to walls that run mostly away from the camera,
        //   versus those that run left-right (?)
        if (side == 1)
            color = ((color >> 1) & 0x007f7f7f) + 0xff000000;

        // blit wall strip pixel
        // TBD: locking may not be necessary, see render3d
        SDL_LockSurface(buffer);
        ((uint32_t *)buffer->pixels)[y * buffer->w + x] = color;
        SDL_UnlockSurface(buffer);
    }
}

// drawline uses Eigen::Vector2d, Eigen::Vector2i,
//   Eigen::Vector2d::operator+(Eigen::Vector2d),
//   Eigen::Vector2d::operator*(scalar),
//   Eigen::Vector2d::operator()(scalar)
//   Eigen::Vector2d::minCoeff(IndexType *index)
//      finds the minimum of all coefficients of *this and puts in *index its location
//   Eigen::Matrix::cwise*: https://www.eigen.tuxfamily.org/dox/group__CoeffwiseMathFunctions.html
//   Eigen::Vector2d::cwiseAbs()
//      std::abs(a[i]);
//   Eigen::Vector2d::cwiseInverse()
//      a[i] = 1 / a[i];

// draws a single vertical strip of maze wall
// @x horizontal pixel coordinate in display window
void App::drawLine(int x) {
    // TBD: redundant, see render3d
    int w, h;
    SDL_GetWindowSize(state->window, &w, &h);

    // ray to cast
    // TBD: what the roles of t and view_plane here? need to look at other examples
    double t { 2.0 * double(x) / double(w) - 1.0 };
    Vector2d ray { state->dir + state->view_plane * t };

    // temp copy of state->pos
    Vector2i map_pos(state->pos(0), state->pos(1));

    // TBD: delta in distance?
    // TBD: why inversion of absolute values?
    Vector2d d_dist { ray.cwiseAbs().cwiseInverse() };

    // TBD: does this indicate whether ray/wall intersection happens with a
    //   horizontal or vertical wall? Seems to only hold 0 or 1 (indices of a Vector)
    int side { 0 };

    // set up incrementation of ray segments
    // TBD: shouldn't step direction be same as ray direction?
    Vector2i step_dir;
    Vector2d side_dist;
    if (ray(0) < 0) {
        side_dist(0) = (state->pos(0) - double(map_pos(0))) * d_dist(0);
        step_dir(0) = -1;
    } else {
        side_dist(0) = (double(map_pos(0)) + 1.0 - state->pos(0)) * d_dist(0);
        step_dir(0) = 1;
    }
    if (ray(1) < 0) {
        side_dist(1) = (state->pos(1) - double(map_pos(1))) * d_dist(1);
        step_dir(1) = -1;
    } else {
        side_dist(1) = (double(map_pos(1)) + 1.0 - state->pos(1)) * d_dist(1);
        step_dir(1) = 1;
    }

    // incrementally cast ray until it intersects with a wall
    while (state->layout->map[map_pos(0)][map_pos(1)] == 0) {
        // TBD: any reason original used ptrdiff_t for i? To match an Eigen index type?
        int i;
        side_dist.minCoeff(&i);
        side = i;
        side_dist(i) += d_dist(i);
        map_pos(i) += step_dir(i);
    }

    // TBD: what does the perp in the original perpWallDist indicate?
    //   perpendicular? how?
    double perp_wall_dist { (side == 0) ?
            (map_pos(0) + ((1.0 - step_dir(0)) / 2.0) - state->pos(0)) / ray(0) :
            (map_pos(1) + ((1.0 - step_dir(1)) / 2.0) - state->pos(1)) / ray(1) };

    // overall hieght of wall segment in pixels
    int line_height( h / perp_wall_dist );

    // top pixel y coordinate of wall segment
    int draw_start { (h - line_height) / 2 };
    // trim to bottom of window
    if (draw_start < 0)
        draw_start = 0;

    // bottom pixel y coordinate of wall segment
    int draw_end { (line_height + h) / 2 };
    // trim to top of window
    if (draw_end >= h)
        draw_end = h - 1;

    // TBD: `color` unused, perhaps leftover from testing?
    // int color { (side == 1) ? 0x4D : 0x8F };

    // TBD: can we simplify this list of params? line_height can be rederived by
    //   subtracting draw_start from draw_end, for example
    drawTexture(x, side, line_height, perp_wall_dist,
                draw_start, draw_end, ray, map_pos);
}

// draws vertical strips of maze walls
void App::render3d() {
    // TBD: These first 2 SDL API calls were commented out - they seem to be
    //   resetting the frame to white
// OC:    SDL_SetRenderDrawColor(state->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
// OC:    SDL_RenderClear(state->renderer);

    // TBD: SDL_GetWindowSize is called by drawLine, render3d and render2d in
    //   the course of drawing a single frame. Would it make more sense to call
    //   it once per frame and have w and h as App member variables instead?
    int w, h;
    // size in pixels (SDL_WINDOW_ALLOW_HIGHDPI not used)
    SDL_GetWindowSize(state->window, &w, &h);

    // lock, memset, unlock pattern follows example from https://wiki.libsdl.org/SDL_Surface,
    //   but two things to note:
    //   - given that there is no multithreading, it may not be necessary to
    //       lock/unlock; this can be evaluated with SDL_MUSTLOCK(surface)
    //   - this same action of initializing a surface could be done with SDL_FillRect
    SDL_LockSurface(buffer);
    memset(buffer->pixels, 0x00, buffer->h * buffer->pitch);
    SDL_UnlockSurface(buffer);

    // cast ray and draw wall strip for each horizontal pixel in window
    for (int x {0}; x < w; ++x)
        drawLine(x);

    // "blitting" mentioned in SDL_Surface docs, and seems to refer to
    //   updating individual pixels of a display frame instead of re-rendering
    //   the entire frame each time any element changes, see:
    //   - https://gamedevelopment.tutsplus.com/articles/gamedev-glossary-what-is-blitting--gamedev-2247
    // So SDL_surface appears to be the working buffer modified by the game,
    //   which then is finalized into a SDL_Texture, which is then rendered to
    //   a window.
    // TBD: https://wiki.libsdl.org/SDL_Texture page incomplete, and definition
    //   not found in SDL headers - need to find list of members...
    SDL_UpdateTexture(buffer_texture, nullptr, buffer->pixels, buffer->pitch);
    SDL_RenderCopy(state->renderer, buffer_texture, nullptr, nullptr);
}

// draws map overlay
// TBD: look up SDL API calls to better understand drawing
void App::render2d() {
    int w, h;
    SDL_GetWindowSize(state->window, &w, &h);
    // set viewport dimensions
    // TBD: note narrowing comversion from double to float
    float vp_scale (  // viewport scale
        (float(w) * 0.25 / state->layout->columns <=
         float(h) * 0.25 / state->layout->rows) ?
        float(w) * 0.25 / state->layout->columns :
        float(h) * 0.25 / state->layout->rows );
    SDL_Rect viewport;
    viewport.h = vp_scale * state->layout->rows;
    viewport.w = vp_scale * state->layout->columns;
    viewport.x = w - viewport.w;
    viewport.y = 0;
    SDL_SetRenderDrawColor(state->renderer, 0x4B, 0x4B, 0x4B, 0x00);
    SDL_RenderSetViewport(state->renderer, NULL);
    SDL_RenderSetScale(state->renderer, 1, 1);
    SDL_RenderFillRect(state->renderer, &viewport);
    SDL_RenderSetViewport(state->renderer, &viewport);
    SDL_RenderSetScale(state->renderer, vp_scale, vp_scale);
    SDL_SetRenderDrawColor(state->renderer, 0x8F, 0x8F, 0x8F, 0x00);
    // TBD: looks like only empty map coordinates are drawn, with walls
    //   implied via negative space?
    for (uint32_t i = 0; i < state->layout->rows; i++) {
        for (uint32_t j = 0; j < state->layout->columns; j++) {
            if (state->layout->map[i][j] == 0) {
                SDL_Rect block = {int(j), int(i), 1, 1};
                SDL_RenderFillRect(state->renderer, &block);
            }
        }
    }
    SDL_RenderSetScale(state->renderer, vp_scale / 4, vp_scale / 4);
    SDL_SetRenderDrawColor(state->renderer, 0xFF, 0x00, 0x00, 0x00);
    SDL_RenderDrawPoint(state->renderer, int(state->pos(1) * 4), int(state->pos(0) * 4));
    SDL_RenderSetViewport(state->renderer, NULL);
    SDL_RenderSetScale(state->renderer, 1, 1);
}

void App::displayFPS(double fps) {
    int w, h;
    SDL_QueryTexture(state->fps_texture, NULL, NULL, &w, &h);
    auto fpsString = std::to_string(fps);
    SDL_Rect destR = {0, 0, w, h};
    SDL_RenderCopy(state->renderer, state->fps_texture, NULL, &destR);
    for (char c : fpsString) {
        // TBD: feels hacky to ignore exceptions here, can we determine what is throwing?
        try {
            SDL_Texture *tex = state->font_cache.at(c);
            destR.x += w;
            SDL_QueryTexture(tex, NULL, NULL, &w, &h);
            destR.w = w;
            SDL_RenderCopy(state->renderer, tex, NULL, &destR);
            // TBD:: original `} catch (std::out_of_range) {`:
            //   gcc error: catching polymorphic type ‘class std::out_of_range’ by value
        } catch (std::out_of_range &oor) {
            continue;
        }
    }
}

// polls SDL input events every game loop
void App::getEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch(e.type) {
            case SDL_QUIT:
                state->done = true;
                break;
            case SDL_KEYDOWN | SDL_KEYUP:
                state->key_handler.handleKeyEvent(e.key);
                break;
            default:
                break;
        }
    }
}

// uses Eigen::Vector2d Eigen::Matrix2d
//   Eigen::Matrix2d::operator<<(scalar)
//   Eigen::Matrix2d::operator*(Eigen::Vector2d)
// TBD: if returning a Vector2d, why do we need to multiply by a matrix?
Vector2d App::rotate2d(Vector2d vector, double rotSpeed) {
    Matrix2d rotate;
    rotate << std::cos(rotSpeed), -std::sin(rotSpeed),
    std::sin(rotSpeed), std::cos(rotSpeed);
    return (rotate * vector);
}

void App::updateData(double frameTime) {
    double moveSpeed = frameTime * 4;
    double rotSpeed = frameTime * 2;
    // q key: quit
    if (state->key_handler.isPressed(SDLK_q) || state->key_handler.isPressed(SDLK_ESCAPE)) {
        state->done = true;
        return;
    }
    // left arrow key: rotate left
    if (state->key_handler.isPressed(SDLK_LEFT)) {
        state->dir = rotate2d(state->dir, rotSpeed);
        state->view_plane = rotate2d(state->view_plane, rotSpeed);
    }
    // right arrow key: roatate right
    if (state->key_handler.isPressed(SDLK_RIGHT)) {
        state->dir = rotate2d(state->dir, -rotSpeed);
        state->view_plane = rotate2d(state->view_plane, -rotSpeed);
    }
    // up arrow key: move forward
    if (state->key_handler.isPressed(SDLK_UP)) {
        double tmp = state->pos(0);
        if (!state->layout->map[int(state->pos(0) + state->dir(0) * moveSpeed)][int(state->pos(1))]) {
            state->pos(0) += state->dir(0) * moveSpeed;
        }
        if (!state->layout->map[int(tmp)][int(state->pos(1) + state->dir(1) * moveSpeed)]) {
            state->pos(1) += state->dir(1) * moveSpeed;
        }
    }
    // down arrow key: move backward
    if (state->key_handler.isPressed(SDLK_DOWN)) {
        double tmp = state->pos(0);
        if (!state->layout->map[int(state->pos(0) - state->dir(0) * moveSpeed)][int(state->pos(1))])
            state->pos(0) -= state->dir(0) * moveSpeed;
        if (!state->layout->map[int(tmp)][int(state->pos(1) - state->dir(1) * moveSpeed)])
            state->pos(1) -= state->dir(1) * moveSpeed;
    }
    // a key: move left (strafe)
    if (state->key_handler.isPressed(SDLK_a)) {
        double tmp = state->pos(0);
        if (!state->layout->map[int(state->pos(0) - state->dir(1) * moveSpeed)][int(state->pos(1))])
            state->pos(0) -= state->dir(1) * moveSpeed;
        if (!state->layout->map[int(tmp)][int(state->pos(1) + state->dir(0) * moveSpeed)])
            state->pos(1) += state->dir(0) * moveSpeed;
    }
    // d key: move right (strafe)
    if (state->key_handler.isPressed(SDLK_d)) {
        double tmp = state->pos(0);
        if (!state->layout->map[int(state->pos(0) + state->dir(1) * moveSpeed)][int(state->pos(1))])
            state->pos(0) += state->dir(1) * moveSpeed;
        if (!state->layout->map[int(tmp)][int(state->pos(1) - state->dir(0) * moveSpeed)])
            state->pos(1) -= state->dir(0) * moveSpeed;
    }
    // p key: (un)pause music
    if (state->key_handler.isPressed(SDLK_p) && Mix_PlayingMusic())
        Mix_PauseMusic();
    else if (state->key_handler.isReleased(SDLK_p) && Mix_PausedMusic())
        Mix_ResumeMusic();
    // f key: toggle FPS
    state->show_fps = state->key_handler.isPressed(SDLK_f);
    // m key: toggle map overlay
    state->show_map = state->key_handler.isPressed(SDLK_m);
}
