#include "../include/app.hh"
#include <time.h>  // clock clock_t CLOCKS_PER_SEC

App::App() {
    state = State::getInstance();
}

App::~App() {
    delete state;
    if (buffer)
        SDL_FreeSurface(buffer);
    if (buffTex)
        SDL_DestroyTexture(buffTex);
    std::for_each(textures.begin(), textures.end(),
                  [] (SDL_Surface *s) { if (s) SDL_FreeSurface(s); });
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
bool App::run(std::string filename) {
    initialize(filename);
    clock_t newTime, oldTime;
    double movingAverage = 0.015;
    oldTime = clock();
    Mix_FadeInMusic(music, -1, 3000);
    while(!state->done) {
        if (sky)
            SDL_RenderCopy(state->renderer, sky, nullptr, nullptr);
        render3d();
        if (state->showMap)
            render2d();
        newTime = clock();
        double frameTime = ((double(newTime - oldTime)) / CLOCKS_PER_SEC);
        oldTime = newTime;
        movingAverage = (movingAverage * 19.0 / 20.0) + (frameTime / 20.0);
        if (state->showFPS)
            displayFPS(1/movingAverage);
        SDL_RenderPresent(state->renderer);
        getEvents();
        updateData(movingAverage);
    }
    return (true);
}

// TBD: all this just to make a custom font for the FPS counter? Needs further investigation
void App::makeGlyphs(std::string fontname) {
    if (TTF_Init()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't initialize true type font system: %s", TTF_GetError());
        throw std::runtime_error("TTF_Init failed");
    }
    TTF_Font *font = TTF_OpenFont(fontname.c_str(), 24);
    if (!font)
        throw std::runtime_error("TTF_OpenFont failed");
    SDL_Color fg = {0xff,0xff,0xff,0xff};
    SDL_Surface *textSurf = TTF_RenderText_Blended(font, "FPS: ", fg);
    if (!textSurf)
        throw std::bad_alloc();
    SDL_Texture *fpsTex = SDL_CreateTextureFromSurface(state->renderer, textSurf);
    if (!fpsTex)
        throw std::bad_alloc();
    state->fpsTex = fpsTex;
    SDL_FreeSurface(textSurf);
    for (char c : std::string(".0123456789")) {
        SDL_Surface *glyphSurf = TTF_RenderGlyph_Blended(font, c, fg);
        if (!glyphSurf)
            throw std::bad_alloc();
        SDL_Texture *glyphTex = SDL_CreateTextureFromSurface(state->renderer, glyphSurf);
        SDL_FreeSurface(glyphSurf);
        if (!glyphTex)
            throw std::bad_alloc();
        state->fontCache[c] = glyphTex;
    }
}

// uses Eigen::Vector2d::operator<<(scalar) (mulitple terms popoulate matrix in level order)
void App::initialize(std::string filename) {
    // parse map file
    state->layout = new Layout(filename, std::ref(state->pos));
    state->dir << 0, 1;
    state->viewPlane << 2.0/3, 0;

    // init SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        throw std::runtime_error("SDL_Init failed");
    }
    if (!(state->window = SDL_CreateWindow(
              "AmAzing", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
              WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE))) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s", SDL_GetError());
        throw std::bad_alloc();
    }
    if (!(state->renderer = SDL_CreateRenderer(
              state->window, -1, SDL_RENDERER_ACCELERATED))) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create renderer: %s", SDL_GetError());
        throw std::bad_alloc();
    }

    // init state->fpsTex, state->fontCache
    makeGlyphs("AmAzing/fonts/Courier New.ttf");

    // further SDL init
    buffer = SDL_CreateRGBSurfaceWithFormat(0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_PIXELFORMAT_BGRA32);
    if (!buffer) {
        throw std::bad_alloc();
    }
    buffTex = SDL_CreateTexture(
        state->renderer, buffer->format->format, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!buffTex) {
        throw std::bad_alloc();
    }
    SDL_SetTextureBlendMode(buffTex, SDL_BLENDMODE_BLEND);

    // init texture access
    if (IMG_Init(IMG_INIT_JPG) != IMG_INIT_JPG)
        throw std::runtime_error("Texture initialization failed");
    textures[1] = IMG_Load("AmAzing/images/wood.jpg");
    textures[2] = IMG_Load("AmAzing/images/metal.jpg");
    textures[3] = IMG_Load("AmAzing/images/curtain.jpg");
    textures[4] = IMG_Load("AmAzing/images/stone_moss.jpg");
    textures[5] = IMG_Load("AmAzing/images/bark.jpg");
    textures[6] = IMG_Load("AmAzing/images/privat_parkering.jpg");
    textures[7] = IMG_Load("AmAzing/images/grass.jpg");
    textures[8] = IMG_Load("AmAzing/images/lava.jpg");

    // init skyplane
    SDL_Surface *skySurf = IMG_Load("AmAzing/images/Vue1.jpg");
    if (!skySurf)
        throw std::bad_alloc();
    sky = SDL_CreateTextureFromSurface(state->renderer, skySurf);
    SDL_SetTextureBlendMode(sky, SDL_BLENDMODE_BLEND);
    SDL_FreeSurface(skySurf);

    // init music
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) < 0)
        throw std::bad_alloc();
    if (!(music = Mix_LoadMUS("AmAzing/audio/Game_of_Thrones.wav")))
        throw std::runtime_error("Could not load music");
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
    Eigen::Vector2d ray = state->dir + state->viewPlane * t;
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
    SDL_UpdateTexture(buffTex, nullptr, buffer->pixels, buffer->pitch);
    SDL_RenderCopy(state->renderer, buffTex, nullptr, nullptr);
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
    SDL_QueryTexture(state->fpsTex, NULL, NULL, &w, &h);
    auto fpsString = std::to_string(fps);
    SDL_Rect destR = {0, 0, w, h};
    SDL_RenderCopy(state->renderer, state->fpsTex, NULL, &destR);
    for (char c : fpsString) {
        // TBD: feels hacky to ignore exceptions here, can we determine what is throwing?
        try {
            SDL_Texture *tex = state->fontCache.at(c);
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
                state->keyHandler.handleKeyEvent(e.key);
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
    if (state->keyHandler.isPressed(SDLK_q) || state->keyHandler.isPressed(SDLK_ESCAPE)) {
        state->done = true;
        return;
    }
    // left arrow key: rotate left
    if (state->keyHandler.isPressed(SDLK_LEFT)) {
        state->dir = rotate2d(state->dir, rotSpeed);
        state->viewPlane = rotate2d(state->viewPlane, rotSpeed);
    }
    // right arrow key: roatate right
    if (state->keyHandler.isPressed(SDLK_RIGHT)) {
        state->dir = rotate2d(state->dir, -rotSpeed);
        state->viewPlane = rotate2d(state->viewPlane, -rotSpeed);
    }
    // up arrow key: move forward
    if (state->keyHandler.isPressed(SDLK_UP)) {
        double tmp = state->pos(0);
        if (!state->layout->map[int(state->pos(0) + state->dir(0) * moveSpeed)][int(state->pos(1))]) {
            state->pos(0) += state->dir(0) * moveSpeed;
        }
        if (!state->layout->map[int(tmp)][int(state->pos(1) + state->dir(1) * moveSpeed)]) {
            state->pos(1) += state->dir(1) * moveSpeed;
        }
    }
    // down arrow key: move backward
    if (state->keyHandler.isPressed(SDLK_DOWN)) {
        double tmp = state->pos(0);
        if (!state->layout->map[int(state->pos(0) - state->dir(0) * moveSpeed)][int(state->pos(1))])
            state->pos(0) -= state->dir(0) * moveSpeed;
        if (!state->layout->map[int(tmp)][int(state->pos(1) - state->dir(1) * moveSpeed)])
            state->pos(1) -= state->dir(1) * moveSpeed;
    }
    // a key: TBD: ??? strafe?
    if (state->keyHandler.isPressed(SDLK_a)) {
        double tmp = state->pos(0);
        if (!state->layout->map[int(state->pos(0) - state->dir(1) * moveSpeed)][int(state->pos(1))])
            state->pos(0) -= state->dir(1) * moveSpeed;
        if (!state->layout->map[int(tmp)][int(state->pos(1) + state->dir(0) * moveSpeed)])
            state->pos(1) += state->dir(0) * moveSpeed;
    }
    // d key: TBD: ??? strafe?
    if (state->keyHandler.isPressed(SDLK_d)) {
        double tmp = state->pos(0);
        if (!state->layout->map[int(state->pos(0) + state->dir(1) * moveSpeed)][int(state->pos(1))])
            state->pos(0) += state->dir(1) * moveSpeed;
        if (!state->layout->map[int(tmp)][int(state->pos(1) - state->dir(0) * moveSpeed)])
            state->pos(1) -= state->dir(0) * moveSpeed;
    }
    // p key: (un)pause music
    if (state->keyHandler.isPressed(SDLK_p) && Mix_PlayingMusic())
        Mix_PauseMusic();
    else if (state->keyHandler.isReleased(SDLK_p) && Mix_PausedMusic())
        Mix_ResumeMusic();
    // f key: toggle FPS
    state->showFPS = state->keyHandler.isPressed(SDLK_f);
    // m key: toggle map overlay
    state->showMap = state->keyHandler.isPressed(SDLK_m);
}
