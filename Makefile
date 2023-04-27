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
	@./deadlock.sh 3 2 10 5 10
	@./deadlock.sh 3 5 1 3 0
	@./deadlock.sh 3 1 2 3 1
	@./deadlock.sh 3 2 100 100 100
	@./deadlock.sh 100 100 100 100 100
	@./deadlock.sh 100 100 100 100 0
	@./deadlock.sh 30 50 10 10 10
	@./deadlock.sh 30 50 100 100 1000

check: test
	@bash kontrola-vystupu.sh < proj2.out

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

kill:
	@ipcs -tm | grep "$$(whoami)" | awk '{print $$1};' | xargs -L1 ipcrm -m
	@ipcs -ts | grep "$$(whoami)" | awk '{print $$1};' | xargs -L1 ipcrm -s
	@find /dev/shm -user "$$(whoami)" -delete
	@killall proj2

show:
	@ipcs -tm | grep "$(whoami)"
	@ipcs -ts | grep "$$(whoami)"

help:
	@echo "This is a 2. project post office	from subject IOS"
	@echo "Usage: make OPTION"
	@echo "	"
	@echo "Options: "		
	@echo " all			compiles program"
	@echo " clear			clears all .txt .o and .out files"
	@echo " test			run program with test intut"
	@echo " check 			check format of proj2.out, prints warning when format is wrong"
	@echo " valgrind		test program with valgrind "
	@echo " helgrind		test program with valringds mode helgrind"
	@echo " zip			compess program files to zip"
	@echo " upload			upload proj2.zip to merlin"
	@echo " merlin			login into merlin throught ssh" 
	@echo " kill			delete semaphoresm, kill all processes named proj2"
	@echo " show 			show information about shared memory and semaphores"
	@echo " help 			print help"