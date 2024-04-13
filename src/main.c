#include "core/chip.h"
#include "core/stack.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include <stdbool.h>
#include <stdio.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 320
#define SCALE 10

void update_renderer(SDL_Renderer *renderer,
                     bool display[DISPLAY_WIDTH][DISPLAY_HEIGHT]) {
  for (int i = 0; i < DISPLAY_WIDTH; ++i) {
    for (int j = 0; j < DISPLAY_HEIGHT; ++j) {
      if (display[i][j]) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      } else {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
      }
      SDL_Rect rect = {i * SCALE, j * SCALE, SCALE, SCALE};
      SDL_RenderFillRect(renderer, &rect);
    }
  }
  SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("not enough argument given.\n");
    return 1;
  }

  chip_t chip;
  chip_init(&chip, argv[1]);

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "error while initializing sdl : %s\n", SDL_GetError());
    return 1;
  }

  SDL_Window *window = SDL_CreateWindow("Chip-8", SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH,
                                        WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
  if (window == NULL) {
    fprintf(stderr, "error while creating window : %s\n", SDL_GetError());
    return 1;
  }

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == NULL) {
    fprintf(stderr, "error while creating renderer : %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // bg col
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);

  SDL_Event event;
  int quit = 0;
  while (!quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        quit = 1;
      }
    }
    for (int fps = 0; fps < 10; fps++) {
      chip_run(&chip);
      update_renderer(renderer, chip.display);
    }
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
