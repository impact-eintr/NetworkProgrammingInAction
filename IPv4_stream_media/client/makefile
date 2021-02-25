all:server
CFLAGS += -lstdc++ -pthread -Wall -g

server:main.c server.o
	gcc $^ -o $@
	mv $@ ./bin

server.o:server.c
	gcc $^ -c -o $@
	mv $@ ./bin


clean:
	rm -i ./bin/*
