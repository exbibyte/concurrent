# concurrent
practicing shared memory concurrency in C++

Relocated some chunks of code from one of my previous repository. These mainly contain code that is related to concurrency and shared memory multi-threading. They are based on references such as C++ Concurrency in Action and The Art of Multiprocessor Programming.

# implementations
  - combining tree
  - lock striped hashtable (chained, universal hashing)
  - lockfree list
    - total
  - lockfree queue
    - total
    - synchronous
  - lockfree stack
    - total
    - split reference
    - partial elimination
  - elimination array
  - other primitves
    - locks
      - blocking
      - recursive
      - reader-writer
    - lockfree exchanger
