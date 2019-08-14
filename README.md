# concurrent
practicing shared memory concurrency in C++

Relocated some chunks of code from one of my previous repository. These mainly contain code that is related to concurrency and shared memory multi-threading.

They are based on references such as: C++ Concurrency in Action, Art of Multiprocessor Programming, Hands-On Concurrency with Rust.
  
# core implementations
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

# work in progress
  - implement safe memory reclamation strategies
    - hazard pointer (done)
    - epoch (todo)
  - add working memory reclamation strategies in core implementations (todo)

# currently supported implementations with safe memory reclamation:
  - lockfree queue
    - queue_lockfree_total with hazard pointer

# language
  - currently moving from C++11 to C++14/17 for some features