### Embedded-systems

This repository contains the source code of my project that was developed as part of the course "Real Time and Embedded Systems" assignment that took place in the Department of Electrical and Computer Engineering at Aristotle University of Thessaloniki in 2020-2021.
The goal of this assignment was to implement the producer-consumer algorithm using pthreads while trying to minimize the energy consumption of the system. 

PThreads were used in order to implement concurrent communication between the producer and the consumer.

### Execution
To execute the code you need to compile it using:
```sh
gcc -pthread prod-cons.c -lm -o output
```
and then run it using:
```
./output
```