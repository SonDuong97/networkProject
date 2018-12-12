all: build

build:
		gcc server.c -o server cjson/cJSON.o
		gcc client.c -o client
setup: 
		sudo chmod +x setup.sh
		./setup.sh


clean:
		rm -rf server client