#ifndef _PANIC_HPP
#define _PANIC_HPP

#define PanicStartX 400
#define PanicStartY 500

struct InterruptFrame;
struct InterruptFrameError;

__attribute__((no_caller_saved_registers)) void panic(const char* panicMessage);

__attribute__((no_caller_saved_registers)) void panic(InterruptFrame*, const char* panicMessage);

__attribute__((no_caller_saved_registers)) void panic(InterruptFrameError*,
                                                      const char* panicMessage);

#endif  // !_PANIC_HPP