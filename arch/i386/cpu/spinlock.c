#include <spinlock.h>

void spinlock_lock(spinlock_t *lock) {
    while (__sync_lock_test_and_set(&lock->locked)) {
        asm volatile("hlt");
    }
}

void spinlock_unlock(spinlock_t *lock) {
    __sync_lock_release(&lock->locked);
}
