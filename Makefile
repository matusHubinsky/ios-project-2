.PHONY : all clear memory run test zip 

CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic -pthread -lrt
PROJ = proj2
XLOGIN = xhubin04
SERVER = merlin.fit.vutbr.cz

$(PROJ): main.c
	$(CC) $(CFLAGS) $^ -o $@

all: $(PROJ)

clear:
	rm *.o *.out *.txt $(PROJ) $(PROJ).zip

test: all
	./$(PROJ) 3 2 10 10 100

valgrind: all
	@valgrind ./$(PROJ) --leak-check=full

helgrind: all
	@valgrind ./$(PROJ) --tool=helgrind --read-var-info=yes

zip: all  
	zip $(PROJ).zip main.c main.h Makefile

upload: zip
	scp $(PROJ).zip $(XLOGIN)@$(SERVER):~/$(PROJ).zip

merlin: upload
	ssh $(XLOGIN)@$(SERVER)

bash_test:
	@./test.sh -c

kill:
	@ipcs -tm | grep "$$(whoami)" | awk '{print $$1};' | xargs -L1 ipcrm -m
	@ipcs -ts | grep "$$(whoami)" | awk '{print $$1};' | xargs -L1 ipcrm -s
	@find /dev/shm -user "$$(whoami)" -delete
	@killall proj2

show:
	@ipcs -tm | grep "$(whoami)"
	@ipcs -ts | grep "$$(whoami)"