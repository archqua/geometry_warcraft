SRCMODULES = Game.cpp  geometry.cpp  physical_object.cpp  player.cpp  sprite.cpp  weapon.cpp
OBJMODULES = $(SRCMODULES:.cpp=.o)
CXX = g++
ifeq (debug, $(MAKECMDGOALS))
CXXFLAGS = -O2 -Wall -Wextra -Werror -lX11 -DDEBUG
else
CXXFLAGS = -O2 -Wall -Wextra -Werror -lX11
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
