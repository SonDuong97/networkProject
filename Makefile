all: build

build:
		gcc server.c -o server
		gcc client.c -o client cjson/cJSON.o -g
clean:
		rm -rf server client