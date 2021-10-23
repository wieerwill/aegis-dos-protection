#include "Threads/Thread.hpp"

Thread::Thread() : _quit(false) {}

void Thread::quit() { _quit = true; }
