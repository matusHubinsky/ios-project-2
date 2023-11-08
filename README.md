# IOS 2. project post office

The task was inpired by **Allen B. Downey: The Little Book of Semaphores (The barbershop problem)**
https://greenteapress.com/wp/semaphores/

## Result
points: 15/15

## Usage:
```
Options: 
 all                    compiles program
 clear                  clears all .txt .o and .out files
 test                   run program with test intut
 valgrind               test program with valgrind 
 helgrind               test program with valringds mode helgrind
 zip                    compess program files to zip
 upload                 upload proj2.zip to merlin
 merlin                 login into merlin throught ssh
 kill                   delete semaphoresm, kill all processes named proj2
 show                   show information about shared memory and semaphores
 help                   print help
```

## Arguments
`$ ./proj2 NZ NU TZ TU F` \
• NZ: Number of customers \
• NU: Number of officials \
• TZ: Maximum time in miliseconds, that customers waits after creation and before they
      enters the post office (eventually leaves, when post office is closed) **0 <= TZ <= 10000** \
• TU: Maximum time of official break in miliseconds **0 <= TU <= 100** \
• F:  Maximum time in miliseconds in which post office is open for new customers **0 <= F <= 10000**

## Testing:
https://github.com/Blacki005/IOS_tester_2023 \
https://github.com/jakubblaha-ids/IOS-Project-2-Tests-2023 \
https://github.com/matusHubinsky/IOS-2-project-deadlock-tester
