#include "sdl2_smart_ptr.hh"

namespace sdl2_smart_ptr {

namespace deleter {

void Cursor::operator()(SDL_Cursor* cp) const { SDL_FreeCursor(cp); }

void CondVar::operator()(SDL_cond* cp) const { SDL_DestroyCond(cp); }

void Mutex::operator()(SDL_mutex* mp) const { SDL_DestroyMutex(mp); }

void Renderer::operator()(SDL_Renderer* rp) const { SDL_DestroyRenderer(rp); }

void Semaphore::operator()(SDL_sem* sp) const { SDL_DestroySemaphore(sp); }

void Surface::operator()(SDL_Surface* sp) const { SDL_FreeSurface(sp); }

void Texture::operator()(SDL_Texture* tp) const { SDL_DestroyTexture(tp); }

void Window::operator()(SDL_Window* wp) const { SDL_DestroyWindow(wp); }

}  // namespace deleter

unique::Cursor    make_unique(SDL_Cursor* cp) {
    static const deleter::Cursor dltr;
    return unique::Cursor{ cp, dltr };
}

unique::CondVar   make_unique(SDL_cond* cvp) {
    static const deleter::CondVar dltr;
    return unique::CondVar{ cvp, dltr };
}

unique::Mutex     make_unique(SDL_mutex* mp) {
    static const deleter::Mutex dltr;
    return unique::Mutex{ mp, dltr };
}

unique::Renderer  make_unique(SDL_Renderer* rp) {
    static const deleter::Renderer dltr;
    return unique::Renderer{ rp, dltr };
}

unique::Semaphore make_unique(SDL_sem* sp) {
    static const deleter::Semaphore dltr;
    return unique::Semaphore{ sp, dltr };
}

unique::Surface   make_unique(SDL_Surface* sp) {
    static const deleter::Surface dltr;
    return unique::Surface{ sp, dltr };
}

unique::Texture   make_unique(SDL_Texture* tp) {
    static const deleter::Texture dltr;
    return unique::Texture{ tp, dltr };
}

unique::Window    make_unique(SDL_Window* wp) {
    static const deleter::Window dltr;
    return unique::Window{ wp, dltr };
}

shared::Cursor    make_shared(SDL_Cursor* cp) {
    static const deleter::Cursor dltr;
    return shared::Cursor{ cp, dltr };
}

shared::CondVar   make_shared(SDL_cond* cvp) {
    static const deleter::CondVar dltr;
    return shared::CondVar{ cvp, dltr };
}

shared::Mutex     make_shared(SDL_mutex* mp) {
    static const deleter::Mutex dltr;
    return shared::Mutex{ mp, dltr };
}

shared::Renderer  make_shared(SDL_Renderer* rp) {
    static const deleter::Renderer dltr;
    return shared::Renderer{ rp, dltr };
}

shared::Semaphore make_shared(SDL_sem* sp) {
    static const deleter::Semaphore dltr;
    return shared::Semaphore{ sp, dltr };
}

shared::Surface   make_shared(SDL_Surface* sp) {
    static const deleter::Surface dltr;
    return shared::Surface{ sp, dltr };
}

shared::Texture   make_shared(SDL_Texture* tp) {
    static const deleter::Texture dltr;
    return shared::Texture{ tp, dltr };
}

shared::Window    make_shared(SDL_Window* wp) {
    static const deleter::Window dltr;
    return shared::Window{ wp, dltr };
}

}  // namespace sdl2_smart_ptr
