#include "chip.h"

#include <SDL2/SDL.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const char font[FONT_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

uint8_t _wrapping_add8(uint8_t a, uint8_t b) {
  uint8_t r = a + b;
  // wrapping add
  if (r < a || r < b) {
    r = 0 + (b - (UINT8_MAX - a + 1));
  }
  return r;
}

uint16_t _wrapping_add16(uint16_t a, uint16_t b) {
  uint16_t r = a + b;
  // wrapping add
  if (r < a || r < b) {
    r = 0 + (b - (UINT16_MAX - a + 1));
  }
  return r;
}

uint16_t fetch_opcode(chip_t *chip) {
  uint8_t byte1 = chip->memory[chip->pc];
  uint8_t byte2 = chip->memory[chip->pc + 0x1];

  uint16_t opcode = (byte1 << 8) | byte2;

  chip->pc += 0x2;

  return opcode;
}

void decode(chip_t *chip, uint16_t opcode) {
  srand(time(NULL));

  uint8_t x = (opcode >> 8) & 0x0F;
  uint8_t y = (opcode >> 4) & 0x0F;
  uint8_t n = opcode & 0x000F;
  uint8_t nn = opcode & 0x00FF;
  uint16_t nnn = opcode & 0x0FFF;

  switch ((opcode >> 12) & 0xF) {
  case 0x0:
    if (opcode == 0x00E0) { /* cls */
      memset(chip->display, false, DISPLAY_WIDTH * DISPLAY_HEIGHT);
    } else if (opcode == 0x00EE) { /* return */
      if (chip->sp > 0 && chip->sp < chip->stack_capacity) {
        chip->pc = chip->stack[chip->sp--];
      } else {
        chip->pc = 0x200; // error, restart
      }
    }
    break;
  case 0x1: /* jump to nnn */
    chip->pc = nnn;
    break;
  case 0x2: /* call nnn */
    if (chip->sp == chip->stack_capacity - 1) {
      uint16_t *new_stack =
          realloc(chip->stack, sizeof(uint16_t) * chip->stack_capacity * 2);
      if (new_stack == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
      }
      chip->stack = new_stack;
      chip->stack_capacity *= 2;
    }
    chip->stack[++chip->sp] = chip->pc;
    chip->pc = nnn;
    break;
  case 0x3: /* se vx,nn */
    if (chip->V[x] == nn) {
      chip->pc += 2;
    }
    break;
  case 0x4: /* sne vx,nn */
    if (chip->V[x] != nn) {
      chip->pc += 2;
    }
    break;
  case 0x5: /* se vx,vy */
    if (chip->V[x] == chip->V[y]) {
      chip->pc += 2;
    }
    break;
  case 0x6: /* mov vx,nn */
    chip->V[x] = nn;
    break;
  case 0x7: { /* add vx,nn */
    uint8_t vx = _wrapping_add8(chip->V[x], nn);
    chip->V[x] = vx;
  } break;
  case 0x8:
    switch (opcode & 0x000F) {
    case 0x0: /* mov vx,vy */
      chip->V[x] = chip->V[y];
      break;
    case 0x1: /* or vx,vy */
      chip->V[x] |= chip->V[y];
      chip->V[0xF] = 0;
      break;
    case 0x2: /* and vx,vy */
      chip->V[x] &= chip->V[y];
      chip->V[0xF] = 0;
      break;
    case 0x3: /* xor vx,vy */
      chip->V[x] ^= chip->V[y];
      chip->V[0xF] = 0;
      break;
    case 0x4: { /* add vx,vy */
      uint8_t vx = chip->V[x];
      chip->V[x] += chip->V[y];
      chip->V[0xF] = (vx + chip->V[y] > 0xFF);
    } break;
    case 0x5: { /* sub vx,vy */
      uint8_t vx = chip->V[x];
      chip->V[x] -= chip->V[y];
      chip->V[0xF] = (vx >= chip->V[y]); // >= !!
    } break;
    case 0x6: { /* shr vx {,vy} */
      uint8_t vx = chip->V[x];
      chip->V[x] = chip->V[y] >> 1;
      chip->V[0xF] = vx & 0x01;
    } break;
    case 0x7: /* subn vx,vy */
      chip->V[x] = chip->V[y] - chip->V[x];
      chip->V[0xF] = (chip->V[y] > chip->V[x]);
      break;
    case 0xE: { /* shl vx {,vy} */
      uint8_t vx = chip->V[x];
      chip->V[x] = chip->V[y] << 1;
      // superchip : chip->V[x] <<= 1;
      chip->V[0xF] = (vx & 0x80) >> 7;
    } break;
    }
    break;
  case 0x9: /* sne vx,vy */
    if (chip->V[x] != chip->V[y]) {
      chip->pc += 2;
    }
    break;
  case 0xA: /* mvi nnn */
    chip->I = nnn;
    break;
  case 0xB: /* jump to nnn + V0 */
    chip->pc = nnn + (uint16_t)chip->V[0];
    // superchip : chip->pc = nnn + chip->V[x];
    break;
  case 0xC: { /* rnd vx,nn */
    uint8_t randv = rand() & 0xFF;
    chip->V[x] = randv & nn;
  } break;
  case 0xD: { /* dxyn (draw) */
    uint8_t vx = chip->V[x] & 63;
    uint8_t vy = chip->V[y] & 31;

    chip->V[0xF] = 0;

    for (int row = 0; row < n; row++) {
      uint8_t sprite = chip->memory[chip->I + row];
      int cy = (vy + row) % DISPLAY_HEIGHT;
      for (int col = 0; col < 8; col++) {
        int cx = (vx + col) % DISPLAY_WIDTH;
        bool current_pixel = chip->display[cx][cy];
        if (sprite & (0x80 >> col)) {
          if (current_pixel) {
            chip->display[cx][cy] = false;
            chip->V[0xF] = 1;
          } else {
            chip->display[cx][cy] = true;
          }
        }
        if (cx == DISPLAY_WIDTH - 1) {
          break;
        }
      }
      if (cy == DISPLAY_HEIGHT - 1) {
        break;
      }
    }
    break;
  }
  case 0xE:
    switch (opcode & 0x00FF) {
    case 0x9E: /* skp vx */
      if (chip->keys[chip->V[x]]) {
        chip->pc = _wrapping_add16(chip->pc, 0x2);
      }
      break;
    case 0xA1: /* sknp vx */
      if (!chip->keys[chip->V[x]]) {
        chip->pc += 2;
      }
      break;
    }
    break;
  case 0xF:
    switch (opcode & 0x00FF) {
    case 0x07: /* mov vx,dt */
      chip->V[x] = chip->delay_timer;
      break;
    case 0x0A: { /* key vx */
      int keycode = -1;
      chip->halted = false;
      for (int i = 0; i < 16; ++i) { // should wait for release but idk how
        if (chip->keys[i]) {
          keycode = i;
          chip->halted = true;

          SDL_Delay(200); // short delay after reading the key before checking
                          // for its release
          break;
        }
      }
      if (keycode >= 0 && chip->halted) {
        if (!chip->keys[keycode]) {
          chip->V[x] = keycode;
          break;
        }
      } else {
        chip->pc -= 2;
      }
    } break;
    case 0x15: /* mov dt,vx */
      chip->delay_timer = chip->V[x];
      break;
    case 0x18: /* mov st,vx */
      chip->sound_timer = chip->V[x];
      break;
    case 0x1E: { /* add i,vx */
      uint16_t tmpi = chip->I;
      chip->I = _wrapping_add16(tmpi, (uint16_t)chip->V[x]);
      chip->V[0xF] = (tmpi + chip->V[x] > 0xFFF);
    } break;
    case 0x29: { /* ld f,vx */
      uint8_t digit = chip->V[x] & 0x0F;
      chip->I = 0x000 + (uint16_t)(digit * 5);
    } break;
    case 0x33: /* ld b,vx */
      chip->memory[chip->I] = chip->V[x] / 100;
      chip->memory[chip->I + 1] = (chip->V[x] / 10) % 10;
      chip->memory[chip->I + 2] = chip->V[x] % 10;
      break;
    case 0x55: /* ld [i],vx */
      for (int i = 0; i <= x; i++) {
        chip->memory[chip->I + i] = chip->V[i];
      }
      chip->I = _wrapping_add16(chip->I, (uint16_t)x + 1);
      // superchip : without this ^
      break;
    case 0x65: /* ld vx,[i] */
      for (int i = 0; i <= x; i++) {
        chip->V[i] = chip->memory[chip->I + i];
      }
      chip->I = _wrapping_add16(chip->I, (uint16_t)x + 1);
      // superchip : without this ^
      break;
    }
    break;
  default:
    printf("unknown opcode: %x", opcode);
    break;
  }
}
void chip_init(chip_t *chip, char *filename) {
  chip->pc = 0x200;
  chip->I = 0;
  chip->sp = -1;

  chip->halted = false;

  chip->delay_timer = 0;
  chip->sound_timer = 0;

  chip->stack_capacity = 64;
  chip->stack = calloc(chip->stack_capacity, sizeof(uint16_t));

  memset(chip->memory, 0, MEMORY_SIZE);
  memset(chip->display, false, DISPLAY_HEIGHT * DISPLAY_WIDTH);
  memset(chip->keys, false, 16);
  memset(chip->V, 0, 0xF);

  // load font
  for (int i = 0; i < FONT_SIZE; ++i) {
    chip->memory[i + 0x50] = font[i];
  }

  if (chip_load(filename, chip->memory + chip->pc) == -1) {
    exit(-1);
  }
}

int chip_load(char *filename, uint8_t *buffer) {
  FILE *file;
  int file_size;

  file = fopen(filename, "r");
  if (file == NULL) {
    printf("File not found\n");
    return -1;
  }

  fseek(file, 0, SEEK_END);
  file_size = ftell(file);
  printf("ROM path: %s\n", filename);
  printf("ROM size: %d b\n", file_size);
  rewind(file);

  fread(buffer, 1, file_size, file);
  return 0;
}

void chip_free(chip_t *chip) { free(chip->stack); }

void chip_timers(chip_t *chip) {
  if (chip->delay_timer > 0) {
    chip->delay_timer -= 1;
  }

  if (chip->sound_timer > 0) {
    chip->delay_timer -= 1;
  }
}

void chip_run(chip_t *chip) {
  uint16_t opcode = fetch_opcode(chip);
  decode(chip, opcode);
}
