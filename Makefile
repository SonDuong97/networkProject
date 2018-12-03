all: build

build:
		gcc server.c -o server cjson/cJSON.o
		gcc client.c -o client 
clean:
		rm -rf server client