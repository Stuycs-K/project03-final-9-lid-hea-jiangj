compile: compile_client compile_server -lncurses -ltinfo

compile_client: client.o networking.o
	@gcc -o client client.o networking.o -lncurses -ltinfo

compile_server: server.o networking.o
	@gcc -o server server.o networking.o 

client.o: client.c networking.h
	@gcc -c client.c -lncurses -ltinfo

server.o: server.c networking.h
	@gcc -c server.c 

networking.o: networking.c networking.h
	@gcc -c networking.c 

server: compile
	@./server

client: compile
	@./client

clean:
	rm *.o
	rm client
	rm server
