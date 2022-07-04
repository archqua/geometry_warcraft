#pragma once
#ifndef GAME_LOG_H_SENTRY
#define GAME_LOG_H_SENTRY

#include <iostream>

template <class T>
void log(T msg) {
  std::cerr << msg;
}


#endif // GAME_LOG_H_SENTRY
