#pragma once
#ifndef SEMAPHORE_H_SENTRY
#define SEMAPHORE_H_SENTRY

#include <mutex>

class Semaphore {
  // bruh 2 of 3 members are mutable...
  mutable std::mutex mx;
  int _count = 0;
  mutable bool locked = false;
public:
  class Lock: std::unique_lock<std::mutex> {
    using Base = std::unique_lock<std::mutex>;
    const Semaphore& host;
  public:
    Lock(const Semaphore& h): Base(h.mx), host(h) { host.locked=true; }
    Lock(Lock&& other) = default;
    ~Lock() { host.locked=false; }
  };
  Semaphore(int c = 0): _count(c) {}
  Lock lock() const { return Lock(*this); }
  int countUnsafe() const { return _count; }
  int count() const { auto l = lock(); return countUnsafe(); }
  bool isLocked() const { return locked; }
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
