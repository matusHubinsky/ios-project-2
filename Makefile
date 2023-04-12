.PHONY : all clear memory run test zip 

CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic -pthread -lrt
PROJ = proj2
XLOGIN = xhubin04
SERVER = merlin.vutbr.fit.cz

$(PROJ): main.c
	$(CC) $(CFLAGS) $^ -o $@

all: $(PROJ)

clear:
	rm *.o *.out *.txt $(PROJ) $(PROJ).zip

test: all
	./$(PROJ) 3 2 100 100 100

valgrind:
	@valgrind ./$(PROJ) --leak-check=full

helgrind:
	@valgrind ./$(PROJ) --tool=helgrind --read-var-info=yes


zip: all  
	zip $(PROJ).zip *.c *.h Makefile

upload:
	scp $(XLOGIN)@$(SERVER):$(PROJ)
