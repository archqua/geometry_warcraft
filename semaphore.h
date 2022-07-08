#pragma once
#ifndef SEMAPHORE_H_SENTRY
#define SEMAPHORE_H_SENTRY

#include <mutex>

class Semaphore {
  mutable std::mutex mx;
  int _count = 0;
public:
  using Lock = std::unique_lock<std::mutex>;
  Semaphore(int c = 0): _count(c) {}
  Lock lock() const { return Lock(mx); }
  int countUnsafe() const { return _count; }
  int count() const { auto l = lock(); return countUnsafe(); }
  // bool isLocked() const { return locked; }
  int incUnsafe() { return ++_count; }
  int decUnsafe() { return --_count; }
  int inc() { auto l = lock(); return incUnsafe(); }
  int dec() { auto l = lock(); return decUnsafe(); }
  class Error {
  };
  class BadCount : public Error {
    int count;
  public:
    BadCount(int c): count(c) {}
    int what() const { return count; }
  };
};






#endif
