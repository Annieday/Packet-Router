router: router.o
	gcc -Wall -std=c99 -o router router.o -lm

router.o: router.c 
	gcc -Wall -std=c99 -c router.c

pktgen: pktgen.o
	gcc -Wall -std=c99 -o pktgen pktgen.o -lm

pktgen.o: pktgen.c 
	gcc -Wall -std=c99 -c pktgen.c

all: router pktgen
	

clean:
	-rm -f *.o router pktgen core
