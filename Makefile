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
	@./test.sh -c

valgrind: all
	@valgrind ./$(PROJ) --leak-check=full

helgrind: all
	@valgrind ./$(PROJ) --tool=helgrind --read-var-info=yes

zip: all  
	zip $(PROJ).zip *.c *.h Makefile

upload: zip
	scp $(XLOGIN)@$(SERVER):$(PROJ)

kill:
	@ipcs -tm | grep "$$(whoami)" | awk '{print $$1};' | xargs -L1 ipcrm -m
	@ipcs -ts | grep "$$(whoami)" | awk '{print $$1};' | xargs -L1 ipcrm -s
	@find /dev/shm -user "$$(whoami)" -delete
	@killall proj2

show:
	@ipcs -tm | grep "$(whoami)"
	@ipcs -ts | grep "$$(whoami)"