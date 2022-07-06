Game.o: Game.cpp Engine.h physical_object.h geometry.h sprite.h player.h \
 weapon.h collider.h enemy.h log.h
geometry.o: geometry.cpp geometry.h
physical_object.o: physical_object.cpp physical_object.h geometry.h
player.o: player.cpp player.h weapon.h geometry.h physical_object.h \
 sprite.h collider.h Engine.h
sprite.o: sprite.cpp sprite.h physical_object.h geometry.h
weapon.o: weapon.cpp weapon.h geometry.h physical_object.h sprite.h \
 collider.h
enemy.o: enemy.cpp enemy.h weapon.h geometry.h physical_object.h sprite.h \
 collider.h
