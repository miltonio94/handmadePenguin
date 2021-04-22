#include <SDL.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdint.h>

// NOTE: MAP_ANONYMOUS is not defined on Mac OS X and some other UNIX systems.
// On the vast majority of those systems, one can use MAP_ANON instead.
// Huge thanks to Adam Rosenfield for investigating this, and suggesting this
// workaround:
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

#define internal static
#define local_persist static
#define global  static

define MAX_CONTROLLERS 4

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

SDL_GameController *ControllerHandles[MAX_CONTROLLERS];

struct sdl_offscreen_buffer{
  SDL_Texture *Texture;
  void *Memory;
  int Width;
  int Height;
  int Pitch;
};

struct sdl_window_dimension{
  int Width;
  int Height;
};


global sdl_offscreen_buffer GlobalBackBuffer;

sdl_window_dimension SDLGetWindowDimension(SDL_Window *Window){
  sdl_window_dimension Result;

  SDL_GetWindowSize(Window, &Result.Width, &Result.Height);

  return Result;
} 


internal void RenderWeirdGradient(sdl_offscreen_buffer Buffer, int BlueOffset, int GreenOffset){
  uint8 *Row = (uint8 *)Buffer.Memory;

  for(int Y = 0; Y < Buffer.Height; ++Y){
    uint32 *Pixel = (uint32 *)Row;
    for(int X = 0; X < Buffer.Width; ++X){
      uint8 Red = X - Y;
      uint8 Green = X + BlueOffset;
      uint8 Blue = Y + GreenOffset;

      *Pixel++ = ((Red << 16) | (Green << 8) | Blue);
    }
    Row += Buffer.Pitch;
  }
}

internal void SDLResizeTexture(sdl_offscreen_buffer *Buffer, SDL_Renderer *Renderer, int Width, int Height){
  const int BytesPerPixel = 4;
  if(Buffer->Memory){
    munmap(Buffer->Memory, Buffer->Width * Buffer->Height * BytesPerPixel);
  }
  if(Buffer->Texture){
    SDL_DestroyTexture(Buffer->Texture);
  }
  Buffer->Texture = SDL_CreateTexture(Renderer,
			      SDL_PIXELFORMAT_ARGB8888,
			      SDL_TEXTUREACCESS_STREAMING,
			      Width, Height);
  Buffer->Width = Width;
  Buffer->Height = Height;
  Buffer->Pitch = Width * BytesPerPixel;
  Buffer->Memory = mmap(0, Width * Height * BytesPerPixel,
			PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS,
			-1, 0);
}

internal void SDLUpdateWindow(SDL_Window *Window, SDL_Renderer *Renderer, sdl_offscreen_buffer Buffer){
  SDL_UpdateTexture(Buffer.Texture, 0, Buffer.Memory, Buffer.Pitch);
  SDL_RenderCopy(Renderer, Buffer.Texture, 0, 0);
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
	  printf("SDL_WINDOWEVENT_SIZE_CHANGED (%d, %d)\n", Event->window.data1, Event->window.data2);
	} break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
	{
	  printf("SDL_WINDOWEVENT_FOCUS_GAINED\n");
	} break;
        case SDL_WINDOWEVENT_EXPOSED:
	{
	  SDL_Window *Window = SDL_GetWindowFromID(Event->window.windowID);
	  SDL_Renderer *Renderer = SDL_GetRenderer(Window);
	  SDLUpdateWindow(Window, Renderer, GlobalBackBuffer);
	  printf("exposing\n");
	} break;
      }
    } break;
  }

  return ShouldQuit;
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
    // Create our window.
    SDL_Window *Window = SDL_CreateWindow("Handmade Hero",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          640,
                                          480,
                                          SDL_WINDOW_RESIZABLE);
    if(Window)
    {
        // Create a "Renderer" for our window.
        SDL_Renderer *Renderer = SDL_CreateRenderer(Window,
                                                    -1,
                                                    0);
        if (Renderer)
        {
            bool Running = true;
            sdl_window_dimension Dimension = SDLGetWindowDimension(Window);
            SDLResizeTexture(&GlobalBackBuffer, Renderer, Dimension.Width, Dimension.Height);
            int XOffset = 0;
            int YOffset = 0;
            while(Running)
            {
                SDL_Event Event;
                while(SDL_PollEvent(&Event))
                {
                    if (HandleEvent(&Event))
                    {
                        Running = false;
                    }
                }
                RenderWeirdGradient(GlobalBackBuffer, XOffset, YOffset);
                SDLUpdateWindow(Window, Renderer, GlobalBackBuffer);

                ++XOffset;
                YOffset += 2;
            }
        }
        else
        {
            // TODO(casey): Logging
        }
    }
    else
    {
        // TODO(casey): Logging
    }
    
    SDL_Quit();
    return(0);
}
