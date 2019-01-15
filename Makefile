all: build setup

build:
		gcc server.c search.c message.c createfile.c -o server cjson/cJSON.o -g -pthread
		gcc client.c message.c -o client -g
setup: 
		sudo chmod +x setup.sh
		./setup.sh

clean:
		rm -rf server client images info logs processing result event.txt