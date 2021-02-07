#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdint.h>
#include <SDL.h>

#ifndef MAP_ANONYMOUS
  #define MAP_ANONYMOUS MAP_ANON
#endif

#define internal static
#define local_persist static
#define global static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

global SDL_Texture *Texture;
global void *Pixels;
global int TextureWidth;


internal void SDLResizeTexture(SDL_Renderer *Renderer, int Width, int Height){
  if(Pixels){
    free(Pixels);
  }
  if(Texture){
    SDL_DestroyTexture(Texture);
  }
  Texture = SDL_CreateTexture(Renderer,
			      SDL_PIXELFORMAT_ARGB8888,
			      SDL_TEXTUREACCESS_STREAMING,
			      Width, Height);
  TextureWidth = Width;
  Pixels = malloc(Width * Height * 4);
}

internal void SDLUpdateWindow(SDL_Window *Window, SDL_Renderer *Renderer){
  SDL_UpdateTexture(Texture, 0, Pixels, TextureWidth * 4);
  SDL_RenderCopy(Renderer, Texture, 0, 0);
  SDL_RenderPresent(Renderer);
}


bool HandleEvent(SDL_Event *Event){
  bool ShouldQuit = false;

  switch(Event->type){
    case SDL_QUIT:
    {
      printf("SDL_QUIT \n");
      ShouldQuit = true;
    } break;

    case SDL_WINDOWEVENT:
    {
      switch(Event->window.event){
        case SDL_WINDOWEVENT_RESIZED:
	{
	  SDL_Window *Window = SDL_GetWindowFromID(Event->window.windowID);
	  SDL_Renderer *Renderer = SDL_GetRenderer(Window);
	  SDLResizeTexture(Renderer, Event->window.data1, Event->window.data2);
	} break;
        case SDL_WINDOWEVENT_EXPOSED:
	{
	  SDL_Window *Window = SDL_GetWindowFromID(Event->window.windowID);
	  SDL_Renderer *Renderer = SDL_GetRenderer(Window);
	  SDLUpdateWindow(Window, Renderer);
	} break;
      }
    } break;
  }

  return ShouldQuit;
}

int main(int argc, char *argv[]){
  if(SDL_Init(SDL_INIT_VIDEO) != 0){
    // SDL Init didn't work
  }
  SDL_Window *Window;
  Window = SDL_CreateWindow("Handmade Penguin",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			640, 480,
			SDL_WINDOW_RESIZABLE);
  if(Window){
    SDL_Renderer *Renderer = SDL_CreateRenderer(Window, -1, 0);

    if(Renderer){
      while(true){
	SDL_Event Event;
	SDL_WaitEvent(&Event);
	printf("window size is %d x %d \n", GlobalWindowWidth, GlobalWindowHeight);
	if(HandleEvent(&Event)){
	  break;
	}
      }
    }
    SDL_Quit();
  }
  return 0;
}
