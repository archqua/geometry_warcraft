SRCMODULES = Game.cpp  geometry.cpp  physical_object.cpp  player.cpp  sprite.cpp  weapon.cpp  enemy.cpp  collider.cpp  rand_mx.cpp
OBJMODULES = $(SRCMODULES:.cpp=.o)
CXX = g++
ifeq (debug, $(MAKECMDGOALS))
CXXFLAGS = --std=c++17 -O0 -g -Wall -Wextra -Werror -lX11 -pthread -DDEBUG
else
CXXFLAGS = --std=c++17 -O2 -Wall -Wextra -Werror -lX11 -pthread
endif

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

all: Engine.cpp $(OBJMODULES)
	$(CXX) $(CXXFLAGS) $^ -o main

debug: Engine.cpp $(OBJMODULES)
	$(CXX) $(CXXFLAGS) $^ -o main

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(SRCMODULES)
	$(CXX) -MM $^ > $@

clean:
	rm *.o
