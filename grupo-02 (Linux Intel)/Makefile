all: server client

server: bin/aurrasd

client: bin/aurras

serverc: 
	bin/aurrasd ./etc/aurrasd.conf bin/aurrasd-filters/

test4: 
	bin/aurras transform samples/sample-1-so.m4a output.m4a alto eco eco rapido

test3: 
	bin/aurras transform samples/sample-1-so.m4a output.m4a alto eco rapido 

test2: 
	bin/aurras transform samples/sample-1-so.m4a output.m4a alto eco

test1: 
	bin/aurras transform samples/sample-1-so.m4a output.m4a eco 	

testMusic:
	bin/aurras transform samples/Ievan\ Polkka\ \(Loituma\).m4a output.m4a baixo lento baixo		

testStatus:
	bin/aurras status		

bin/aurrasd: obj/aurrasd.o
	gcc -g obj/aurrasd.o -o bin/aurrasd

obj/aurrasd.o: src/aurrasd.c
	gcc -Wall -g -c src/aurrasd.c -o obj/aurrasd.o

bin/aurras: obj/aurras.o
	gcc -g obj/aurras.o -o bin/aurras

obj/aurras.o: src/aurras.c
	gcc -Wall -g -c src/aurras.c -o obj/aurras.o

memcheckServer:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s bin/aurrasd ./etc/aurrasd.conf bin/aurrasd-filters/

memcheckClient:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s bin/aurras transform samples/Ievan\ Polkka\ \(Loituma\).m4a output.m4a eco baixo  lento baixo
clean:
	rm obj/* tmp/* bin/{aurras,aurrasd}

test:
	bin/aurras samples/sample-1.mp3 tmp/sample-1.mp3
	bin/aurras samples/sample-2.mp3 tmp/sample-2.mp3
