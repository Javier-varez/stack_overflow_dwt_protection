# Stack Overflow Dwt Protection

This is a proof of concept of detecting stack overflows by setting a watchpoint to watch for writes to the bottom of the stack. If they happen, we can safely assume that the stack is dangerously close of running out of memory and we can notify the user and stop the operation before that ever happens.


