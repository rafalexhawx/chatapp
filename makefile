#Makefile for the chat interface


all:
	gcc server.c -o server -Wall -lpthread -lncurses
	@echo "Compiled the server program"
	gcc client.c -o client -Wall -lpthread -lncurses
	@echo "Compiled the client program"
	gcc chat.c -o chat -Wall -lpthread -lncurses
	@echo "Compiled the chat program"