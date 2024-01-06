all:main

cc=g++
obj_source=./main.cpp ../util/config.cpp ../server/game/game.cpp
obj=main
header=-I../util
link=-lfmt
.PHONY:main

main: $(source)
	$(cc) -g $(obj_source) -o $(obj) $(link) $(header)
