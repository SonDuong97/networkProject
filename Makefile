all: build setup

build:
		gcc server.c -o server cjson/cJSON.o -g
		gcc client.c -o client -g
setup: 
		sudo chmod +x setup.sh
		./setup.sh


clean:
		rm -rf server client images info logs processing result