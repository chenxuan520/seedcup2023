all:main

cc=g++
obj_source=./main.cpp ../util/config.cpp ../server/game/game.cpp
obj=main
header=-I../util
link=-lfmt
.PHONY:main

main: $(source) ./test.h
	$(cc) -g $(obj_source) -o $(obj) $(link) $(header)

init:
	wget https://gitee.com/chenxuan520/cpptest/raw/master/test.h -O test.h
