all:client
CFLAGS += -lstdc++ -pthread -Wall -g

client:client.o
	gcc $^ -o $@
	mv $^ $@ ./bin

client.o:client.c
	gcc $^ -c -o $@

clean:
	rm -i ./bin/*
