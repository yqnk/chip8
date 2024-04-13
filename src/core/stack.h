#ifndef STACK_H
#define STACK_H

#include <stddef.h>
#include <stdint.h>

#define INITIAL_CAPACITY 16 

#define STACK_POP_EMPTY 11
#define STACK_ALLOCATION_ERROR 12
#define STACK_MEMORY_EXCEEDED 13
#define STACK_POTENTIAL_OVERFLOW 10

struct stack {
  uint16_t *data;
  size_t size;
  size_t capacity;
};
typedef struct stack stack_t;

stack_t stack_init();

void stack_push(stack_t *stack, uint16_t val);
uint16_t stack_pop(stack_t *stack);

void print_stack(stack_t *stack);

#endif // !STACK_H
