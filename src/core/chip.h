#ifndef CHIP_H
#define CHIP_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32

#define FONT_SIZE 80
#define MEMORY_SIZE 4096
#define REGISTER_COUNT 16

struct chip {
  uint8_t delay_timer;
  uint8_t sound_timer;

  uint16_t pc;

  uint16_t sp;
  uint16_t *stack;
  int stack_capacity;

  uint8_t memory[MEMORY_SIZE];
  bool display[DISPLAY_WIDTH][DISPLAY_HEIGHT];
  bool keys[16];

  uint16_t I;                // index register
  uint8_t V[REGISTER_COUNT]; // V0 -> VF

  bool halted;
};
typedef struct chip chip_t;

uint16_t fetch_opcode(chip_t *chip);
void decode(chip_t *chip, uint16_t opcode);

void chip_init(chip_t *chip, char *filename);
int chip_load(char *filename, uint8_t *buffer);
void chip_free(chip_t *chip);

void chip_timers(chip_t *chip);
void chip_run(chip_t *chip);

#endif // !CHIP_H
