#include "sdl_unique_ptrs.hh"


void SdlDeleter::Window::operator()(SDL_Window* wp) const { SDL_DestroyWindow(wp); }

void SdlDeleter::Renderer::operator()(SDL_Renderer* rp) const { SDL_DestroyRenderer(rp); }

void SdlDeleter::Surface::operator()(SDL_Surface* sp) const { SDL_FreeSurface(sp); }

void SdlDeleter::Texture::operator()(SDL_Texture* tp) const { SDL_DestroyTexture(tp); }

void SdlDeleter::TtfFont::operator()(TTF_Font* fp) const { TTF_CloseFont(fp); }
