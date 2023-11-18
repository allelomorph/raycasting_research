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

auto make_unique(SDL_Cursor* cp) {
    return unique::Cursor{ cp, deleter::Cursor{} };
}

auto make_unique(SDL_cond* cvp) {
    return unique::CondVar{ cvp, deleter::CondVar{} };
}

auto make_unique(SDL_mutex* mp) {
    return unique::Mutex{ mp, deleter::Mutex{} };
}

auto make_unique(SDL_Renderer* rp) {
    return unique::Renderer{ rp, deleter::Renderer{} };
}

auto make_unique(SDL_sem* sp) {
    return unique::Semaphore{ sp, deleter::Semaphore{} };
}

auto make_unique(SDL_Surface* sp) {
    return unique::Surface{ sp, deleter::Surface{} };
}

auto make_unique(SDL_Texture* tp) {
    return unique::Texture{ tp, deleter::Texture{} };
}

auto make_unique(SDL_Window* wp) {
    return unique::Window{ wp, deleter::Window{} };
}

auto make_shared(SDL_Cursor* cp) {
    static deleter::Cursor dltr;
    return shared::Cursor{ cp, dltr };
}

auto make_shared(SDL_cond* cvp) {
    static deleter::CondVar dltr;
    return shared::CondVar{ cvp, dltr };
}

auto make_shared(SDL_mutex* mp) {
    static deleter::Mutex dltr;
    return shared::Mutex{ mp, dltr };
}

auto make_shared(SDL_Renderer* rp) {
    static deleter::Renderer dltr;
    return shared::Renderer{ rp, dltr };
}

auto make_shared(SDL_sem* sp) {
    static deleter::Semaphore dltr;
    return shared::Semaphore{ sp, dltr };
}

auto make_shared(SDL_Surface* sp) {
    static deleter::Surface dltr;
    return shared::Surface{ sp, dltr };
}

auto make_shared(SDL_Texture* tp) {
    static deleter::Texture dltr;
    return shared::Texture{ tp, dltr };
}

auto make_shared(SDL_Window* wp) {
    static deleter::Window dltr;
    return shared::Window{ wp, dltr };
}

}  // namespace sdl2_smart_ptr
