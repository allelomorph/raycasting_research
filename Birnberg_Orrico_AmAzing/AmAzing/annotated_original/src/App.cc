#include "App.hh"     // WINDOW_WIDTH WINDOW_HEIGHT FONT_FILE_PATH MUSIC_FILE_PATH

#include <time.h>     // clock clock_t CLOCKS_PER_SEC

#include <algorithm>  // for_each
#include <sstream>    // ostringstream


App::App() {
    state = State::getInstance();
}

App::~App() {
    delete state;
    if (buffer)
        SDL_FreeSurface(buffer);
    if (buffer_texture)
        SDL_DestroyTexture(buffer_texture);
    std::for_each(textures.begin(), textures.end(),
                  [] (SDL_Surface *s) { if (s != nullptr) SDL_FreeSurface(s); });
    if (sky)
        SDL_DestroyTexture(sky);
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
    // Init FPS calculation
    clock_t curr_tp, prev_tp;
    double moving_avg_frame_length = 0.015;
    prev_tp = clock();

    Mix_FadeInMusic(music, -1, 3000);

    while(!state->done) {
        // render backdrop
        if (sky)  // this->sky
            SDL_RenderCopy(state->renderer, sky, nullptr, nullptr);

        render3d();

        if (state->show_map)
            render2d();

        // FPS HUD
        curr_tp = clock();
        double frame_length { ((double(curr_tp - prev_tp)) / CLOCKS_PER_SEC) };
        prev_tp = curr_tp;
        moving_avg_frame_length =
            (moving_avg_frame_length * 19.0 / 20.0) + (frame_length / 20.0);
        if (state->show_fps)
            displayFPS(1 / moving_avg_frame_length);

        SDL_RenderPresent(state->renderer);

        getEvents();

        updateData(moving_avg_frame_length);
    }
}

// TBD: font_filename can be a const char*
// initializes font and texture for FPS counter in HUD, helper to initialize
static void makeGlyphs(std::string font_filename) {
    std::ostringstream err_msg;
    // TBD: per https://wiki.libsdl.org/SDL_LOG_CATEGORY, it looks like SDL keeps
    //   multiple logs, so maybe this call of SDL_LogError promotes the error
    //   to the main application-level log?
    if (TTF_Init() != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't initialize true type font system: %s", TTF_GetError());
        err_msg << "TTF_Init: " << TTF_GetError();
        throw std::runtime_error(err_msg.str());
    }

    // Render FPS HUD prefix text to a new ARGB surface
    TTF_Font *font { TTF_OpenFont(font_filename.c_str(), 24) };
    if (font == nullptr) {
        err_msg << "TTF_OpenFont: " << TTF_GetError();
        throw std::runtime_error(err_msg.str());
    }
    SDL_Color fg { 0xff, 0xff, 0xff, 0xff };  // text foreground color, pure white
    SDL_Surface *text_surface { TTF_RenderText_Blended(font, "FPS: ", fg) };
    if (text_surface == nullptr) {
        err_msg << "TTF_RenderText_Blended: " << TTF_GetError();
        throw std::bad_alloc(err_msg.str());
    }
    state->fps_texture = SDL_CreateTextureFromSurface(state->renderer, text_surface);
    SDL_FreeSurface(text_surface);
    if (state->fps_texture == nullptr) {
        err_msg << "SDL_CreateTextureFromSurface: " << SDL_GetError();
        throw std::bad_alloc(err_msg.str());
    }

    // init font cache with numeric chars for live fps display
    for (const char &c : std::string(".0123456789")) {
        SDL_Surface *glyph_surface { TTF_RenderGlyph_Blended(font, c, fg) };
        if (glyph_surface == nullptr) {
            err_msg << "TTF_RenderGlyph_Blended: " << TTF_GetError();
            throw std::bad_alloc(err_msg.str());
        }
        SDL_Texture *glyph_texture { SDL_CreateTextureFromSurface(state->renderer, glyph_surface) };
        SDL_FreeSurface(glyph_surface);
        if (glyph_texture == nullptr) {
            err_msg << "SDL_CreateTextureFromSurface: " << SDL_GetError();
            throw std::bad_alloc(err_msg.str());
        }
        state->font_cache[c] = glyph_texture;
    }
}

void App::initialize(std::string filename) {
    // parse map file into unit grid
    state->layout = new Layout(filename, std::ref(state->pos));

    // init raycasting variables
    state->dir << 0, 1;
    state->view_plane << 2.0/3, 0;

    // init SDL window and renderer
    std::ostringstream err_msg;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't initialize SDL: %s", SDL_GetError());
        err_msg << "SDL_Init: " << SDL_GetError();
        throw std::runtime_error(err_msg.str());
    }
    state->window = SDL_CreateWindow(
        "AmAzing",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE);
    if (state->window == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't create window: %s", SDL_GetError());
        err_msg << "SDL_CreateWindow: " << SDL_GetError();
        throw std::bad_alloc(err_msg.str());
    }
    state->renderer = SDL_CreateRenderer(
        state->window, -1, SDL_RENDERER_ACCELERATED);
    if (state->renderer == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't create renderer: %s", SDL_GetError());
        err_msg << "SDL_CreateRenderer: " << SDL_GetError();
        throw std::bad_alloc(err_msg.str());
    }

    // init state->fps_texture, state->font_cache
    makeGlyphs(FONT_FILE_PATH);

    buffer = SDL_CreateRGBSurfaceWithFormat(  // this->buffer
        0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_PIXELFORMAT_BGRA32);
    if (buffer == nullptr) {
        err_msg << "SDL_CreateRGBSurfaceWithFormat: " << SDL_GetError();
        throw std::bad_alloc(err_msg.str());
    }
    buffer_texture = SDL_CreateTexture(       // this->buffer_texture
        state->renderer, buffer->format->format, SDL_TEXTUREACCESS_STREAMING,
        WINDOW_WIDTH, WINDOW_HEIGHT);
    if (buffer_texture == nullptr) {
        err_msg << "SDL_CreateTexture: " << SDL_GetError();
        throw std::bad_alloc(err_msg.str());
    }
    if (SDL_SetTextureBlendMode(buffer_texture, SDL_BLENDMODE_BLEND) != 0) {
        err_msg << "SDL_SetTextureBlendMode: " << SDL_GetError();
        throw std::runtime_error(err_msg.str());
    }

    // init texture access
    if (IMG_Init(IMG_INIT_JPG) != IMG_INIT_JPG) {
        err_msg << "IMG_Init: " << SDL_GetError();
        throw std::runtime_error(err_msg.str());
    }
    // TBD: no error checking for IMG_Load in original source
    textures[1] = IMG_Load("AmAzing/images/wood.jpg");
    textures[2] = IMG_Load("AmAzing/images/metal.jpg");
    textures[3] = IMG_Load("AmAzing/images/curtain.jpg");
    textures[4] = IMG_Load("AmAzing/images/stone_moss.jpg");
    textures[5] = IMG_Load("AmAzing/images/bark.jpg");
    textures[6] = IMG_Load("AmAzing/images/privat_parkering.jpg");
    textures[7] = IMG_Load("AmAzing/images/grass.jpg");
    textures[8] = IMG_Load("AmAzing/images/lava.jpg");

    // init skyplane
    SDL_Surface *sky_surface { IMG_Load("AmAzing/images/Vue1.jpg") };
    if (sky_surface == nullptr) {
        err_msg << "IMG_Load: " << SDL_GetError();
        throw std::bad_alloc(err_msg.str());
    }
    // TBD: remaining SDL_* calls have no error checking in original source
    sky = SDL_CreateTextureFromSurface(state->renderer, skySurf);  // this->sky
    SDL_SetTextureBlendMode(sky, SDL_BLENDMODE_BLEND);
    SDL_FreeSurface(skySurf);

    // init music
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) != 0) {
        err_msg << "Mix_OpenAudio: " << SDL_GetError();
        throw std::bad_alloc(err_msg.str());
    }
    music = Mix_LoadMUS(MUSIC_FILE_PATH);  // this->music
    if (music = nullptr) {
        err_msg << "Mix_LoadMUS: " << SDL_GetError();
        throw std::bad_alloc(err_msg.str());
    }
}

void App::drawTexture(int x, int side, int lineheight, double perpWallDist,
                      int drawstart, int drawend,
                      Eigen::Vector2d& ray, Eigen::Vector2i& mapPos) {
    // OC: calculate value of wallX
    // OC: where exactly the wall was hit
    double wallX { (side == 1) ?
                   state->pos(0) + perpWallDist * ray(0) :
                   state->pos(1) + perpWallDist * ray(1) };
    // only fractional remainder
    wallX -= floor(wallX);

    // OC: x coordinate on the texture
    // TBD: should 256 instead be a constant, or set by analyzing texture image size?
    // (are all current textures 256px wide?)
    int texX = floor(wallX * 256);
    if (side == 0 && ray(0) > 0)
        texX = 256 - texX - 1;
    if (side == 1 && ray(1) < 0)
        texX = 256 - texX - 1;

    // draw vertical segment of wall texture
    for (int y {drawstart}; y < drawend; ++y) {
        int d = (y + (lineheight - WINDOW_HEIGHT) / 2) * 256;
        int texY = ((d * 256) / lineheight) / 256;
        SDL_Surface* texture = textures[state->layout->map[mapPos(0)][mapPos(1)]];
        uint32_t color = ((uint32_t *)texture->pixels)[texY * 256 + texX];
        // OC: make color darker for y-sides: R, G and B byte each divided through two with a "shift" and an "and"
        if (side == 1)
            color = ((color >> 1) & 0x007f7f7f) + 0xff000000;

        SDL_LockSurface(buffer);
        ((uint32_t *)buffer->pixels)[y * buffer->w + x] = color;
        SDL_UnlockSurface(buffer);
    }
}

// uses Eigen::Vector2d, Eigen::Vector2i,
//   Eigen::Vector2d::operator+(Eigen::Vector2d),
//   Eigen::Vector2d::operator*(scalar),
//   Eigen::Vector2d::operator()(scalar)
//   Eigen::Vector2d::minCoeff(IndexType *index)
//      finds the minimum of all coefficients of *this and puts in *index its location
//   Eige::Matrix::cwise*: https://www.eigen.tuxfamily.org/dox/group__CoeffwiseMathFunctions.html
//   Eigen::Vector2d::cwiseAbs()
//      std::abs(a[i]);
//   Eigen::Vector2d::cwiseInverse()
//      a[i] = 1 / a[i];
// draws a single vertical strip of maze wall
void App::drawLine(int x) {
    int w, h;
    SDL_GetWindowSize(state->window, &w, &h);
    double t = 2.0 * double(x) / double(w) - 1.0;
    Eigen::Vector2d ray = state->dir + state->view_plane * t;
    Eigen::Vector2i mapPos(state->pos(0), state->pos(1));
    Eigen::Vector2d dDist = ray.cwiseAbs().cwiseInverse();
    int side = 0;

    Eigen::Vector2i stepDir;
    Eigen::Vector2d sideDist;

    // cast ray
    if (ray(0) < 0) {
        sideDist(0) = (state->pos(0) - double(mapPos(0))) * dDist(0);
        stepDir(0) = -1;
    } else {
        sideDist(0) = (double(mapPos(0)) + 1.0 - state->pos(0)) * dDist(0);
        stepDir(0) = 1;
    }
    if (ray(1) < 0) {
        sideDist(1) = (state->pos(1) - double(mapPos(1))) * dDist(1);
        stepDir(1) = -1;
    } else {
        sideDist(1) = (double(mapPos(1)) + 1.0 - state->pos(1)) * dDist(1);
        stepDir(1) = 1;
    }

    // extend ray until it intersects wall
    while (state->layout->map[mapPos(0)][mapPos(1)] == 0) {
        // TBD: any reason why ptrdiff_t and not size_t? to match Eigen IndexType?
        std::ptrdiff_t i;
        sideDist.minCoeff(&i);
        side = int(i);
        sideDist(i) += dDist(i);
        mapPos(i) += stepDir(i);
    }

    // calculate top and bottom heights of wall segment
    double perpWallDist { (side == 0) ?
            (mapPos(0) + ((1.0 - stepDir(0)) / 2.0) - state->pos(0)) / ray(0) :
            (mapPos(1) + ((1.0 - stepDir(1)) / 2.0) - state->pos(1)) / ray(1) };
    int lineHeight = (int)(h / perpWallDist);
    int drawStart = (h - lineHeight) / 2;
    // trim to bottom of window
    if (drawStart < 0)
        drawStart = 0;
    int drawEnd = (lineHeight + h) / 2;
    // trim to top of window
    if (drawEnd >= h)
        drawEnd = h - 1;

    // TBD: `color` is perhaps leftover from testing?
    // int color { (side == 1) ? 0x4D : 0x8F };

    // TBD: is it necessary to pass both the explicit line height and the start and end y values?
    drawTexture(x, side, lineHeight, perpWallDist,
                drawStart, drawEnd, ray, mapPos);
}

// draws vertical strips of maze walls
// TBD: why are first 2 SDL API calls commented out?
void App::render3d() {
// OC:    SDL_SetRenderDrawColor(state->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
// OC:    SDL_RenderClear(state->renderer);
    int w, h;
    SDL_GetWindowSize(state->window, &w, &h);
    SDL_LockSurface(buffer);
    // TBD: why the manual reset of SDL_Surface bytes?
    memset(buffer->pixels, 0x00, buffer->h * buffer->pitch);
    SDL_UnlockSurface(buffer);
    for (int x {0}; x < w; ++x)
        drawLine(x);
    SDL_UpdateTexture(buffer_texture, nullptr, buffer->pixels, buffer->pitch);
    SDL_RenderCopy(state->renderer, buffer_texture, nullptr, nullptr);
}

// draws map overlay
// TBD: look up SDL API calls to better understand drawing
void App::render2d() {
    int w, h;
    SDL_GetWindowSize(state->window, &w, &h);
    // set viewport dimensions
    float vp_scale {  // viewport scale
        (float(w) * 0.25 / state->layout->columns <=
         float(h) * 0.25 / state->layout->rows) ?
        float(w) * 0.25 / state->layout->columns :
        float(h) * 0.25 / state->layout->rows };
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
        } catch (std::out_of_range) {
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
static Eigen::Vector2d rotate2d(Eigen::Vector2d vector, double rotSpeed) {
    Eigen::Matrix2d rotate;
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
    // a key: TBD: ??? strafe?
    if (state->key_handler.isPressed(SDLK_a)) {
        double tmp = state->pos(0);
        if (!state->layout->map[int(state->pos(0) - state->dir(1) * moveSpeed)][int(state->pos(1))])
            state->pos(0) -= state->dir(1) * moveSpeed;
        if (!state->layout->map[int(tmp)][int(state->pos(1) + state->dir(0) * moveSpeed)])
            state->pos(1) += state->dir(0) * moveSpeed;
    }
    // d key: TBD: ??? strafe?
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
