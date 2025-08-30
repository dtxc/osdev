#pragma once

typedef struct {
  volatile int locked;
} spinlock_t;

void spinlock_lock(spinlock_t *locK);
void spinlock_unlock(spinlock_t *lock);
