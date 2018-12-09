all: build

build:
		gcc server.c -o server cjson/cJSON.o
		gcc client.c -o client -g
clean:
		rm -rf server client