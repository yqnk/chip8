#include "stack.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

stack_t stack_init() {
  stack_t stack;
  stack.data = (uint16_t *)malloc(INITIAL_CAPACITY * sizeof(uint16_t));
  if (stack.data == NULL) {
    exit(STACK_ALLOCATION_ERROR);
  }
  stack.size = 0;
  stack.capacity = INITIAL_CAPACITY;
  return stack;
}

void stack_push(stack_t *stack, uint16_t val) {
  if (stack->size == stack->capacity) {
    uint16_t *new_data = (uint16_t *)realloc(stack->data, 2 * stack->capacity *
                                                              sizeof(uint16_t));
    stack->capacity *= 2;
    if (new_data == NULL) {
      exit(STACK_ALLOCATION_ERROR);
    }

    stack->data = new_data;
  }
  stack->data[stack->size++] = val;
}

uint16_t stack_pop(stack_t *stack) {
  if (stack->size == 0) {
    exit(STACK_POP_EMPTY);
  }
  if (stack->size >= 4096) { // just safe
    exit(STACK_POTENTIAL_OVERFLOW);
  }
  if (stack->size > INITIAL_CAPACITY && stack->size < stack->capacity / 2) {
    uint16_t *new_data = (uint16_t *)realloc(
        stack->data, (stack->capacity / 2) * sizeof(uint16_t));
    stack->capacity /= 2;
    if (new_data == NULL) {
      exit(STACK_ALLOCATION_ERROR);
    }
  }
  return stack->data[--stack->size];
}

void print_stack(stack_t *stack) {
  printf("Size: %zu, Capacity: %zu\n", stack->size, stack->capacity);
  printf("Elements: ");
  for (size_t i = 0; i < stack->size; ++i) {
    printf("%d ", stack->data[i]);
  }
  printf("\n");
}
