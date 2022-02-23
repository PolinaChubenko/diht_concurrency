#if(APPLE)
  #define FUNCTION_NAME(name) _##name
#else
  #define FUNCTION_NAME(name) name
#endif

.globl FUNCTION_NAME(AtomicLoad)
.globl FUNCTION_NAME(AtomicStore)
.globl FUNCTION_NAME(AtomicExchange)


FUNCTION_NAME(AtomicLoad):
    movq (%rdi), %rax
    retq

FUNCTION_NAME(AtomicStore):
    movq %rsi, (%rdi)
    retq

FUNCTION_NAME(AtomicExchange):  # rax; rdi = &cell_; rsi = new_value
    movq %rsi, %rax             # rax := rsi
    lock xchg %rax, (%rdi)      # Data blocked while swap(rax, &rdi)
    retq
