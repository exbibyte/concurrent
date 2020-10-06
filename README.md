# concurrent
multithreaded programming in C++

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
	- quiescent state (todo)
	- hyaline (todo)
  - add working memory reclamation strategies in concurrent datastructures (todo)

# currently supported implementations with safe memory reclamation:
  - lockfree queue
    - queue_lockfree_total with hazard pointer

# other notes
  - currently uses C++1z
  - release build: make DEBUG=0 all
