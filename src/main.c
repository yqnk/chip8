#include "core/chip.h"

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#define SCALE 10
#define FPS 6000
#define FRAME_TIME (1000 / FPS)

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

void set_key(chip_t *chip, SDL_KeyCode keycode, bool state) {
  switch (keycode) {
  case SDLK_1:
    chip->keys[0x1] = state;
    break;
  case SDLK_2:
    chip->keys[0x2] = state;
    break;
  case SDLK_3:
    chip->keys[0x3] = state;
    break; // "
  case SDLK_4:
    chip->keys[0xC] = state;
    break;
  case SDLK_a:
    chip->keys[0x4] = state;
    break;
  case SDLK_z:
    chip->keys[0x5] = state;
    break;
  case SDLK_e:
    chip->keys[0x6] = state;
    break;
  case SDLK_r:
    chip->keys[0xD] = state;
    break;
  case SDLK_q:
    chip->keys[0x7] = state;
    break;
  case SDLK_s:
    chip->keys[0x8] = state;
    break;
  case SDLK_d:
    chip->keys[0x9] = state;
    break;
  case SDLK_f:
    chip->keys[0xE] = state;
    break;
  case SDLK_w:
    chip->keys[0xA] = state;
    break;
  case SDLK_x:
    chip->keys[0x0] = state;
    break;
  case SDLK_c:
    chip->keys[0xB] = state;
    break;
  case SDLK_v:
    chip->keys[0xF] = state;
    break;
  default:
    break;
  }
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

  SDL_Window *window = SDL_CreateWindow(
      "Chip-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      DISPLAY_WIDTH * SCALE, DISPLAY_HEIGHT * SCALE, SDL_WINDOW_SHOWN);
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
  int frame_time;
  uint32_t frame_start;
  time_t start = time(NULL);
  int cycles = 0;
  int quit = 0;

  while (quit != 1) {
    frame_start = SDL_GetTicks();
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT ||
          (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
        quit = 1;
      } else if (event.type == SDL_KEYDOWN) {
        set_key(&chip, event.key.keysym.sym, true);
      } else if (event.type == SDL_KEYUP) {
        set_key(&chip, event.key.keysym.sym, false);
      }
    }

    if (quit != 1) {
      chip_run(&chip);
      update_renderer(renderer, chip.display);
      cycles++;

      // [cycles % c] | c: 1 = fast emulation, inf = slow emulation
      if (cycles % 20 == 0) {
        chip_timers(&chip);
      }

      // adjust fps
      frame_time = SDL_GetTicks() - frame_start;
      if (frame_time < FRAME_TIME) {
        SDL_Delay(FRAME_TIME - frame_time);
      }
    }
  }

  // useful for debug etc etc
  double duration = (double)(time(NULL) - start);
  printf("\nSession:\n");
  printf("Duration: %.2fs\n", duration);
  printf("Cycles: %i\n", cycles);

  chip_free(&chip);

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
