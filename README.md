# concurrent
multithreaded programming in C++, experimental..

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
    - epoch (simplify usage, dynamic thread addition/removal)
	- quiescent state (todo)
  - add working memory reclamation strategies in concurrent datastructures (in progress)
    - sync queue, split ref stack
  - more tests for list, combining tree, etc.

# currently supported implementations with safe memory reclamation:
  - lockfree queue
    - queue_lockfree_total with hazard pointer / epoch based reclamation
    - queue_lockfree_sync with hazard pointer
  - lockfree stack
    - stack_lockfree_total_simple with hazard pointer / epoch based reclamation
    - stack_lockfree_elim with hazard pointer
    - stack_lockfree_split_reference with hazard pointer

# other notes
  - currently uses C++1z
  - release build: make DEBUG=0 all
  - user data need to be move constructible and move assignable

# sample usages
  - lockfree queue w/ hazard pointer:
```
#include "queue_lockfree_total.hpp"
#include "threadwrap.hpp"
//...
    using container_type = queue_lockfree_total<int, trait_reclamation::hp>;
    container_type queue;
    unsigned int num_threads = std::thread::hardware_concurrency()/2;
    size_t nums = num_threads * 3000000;
    
    auto f = [&](){
            size_t count = queue.size(); //this is approximate if queue is under concurrent access
            assert( 0 == count );
            int val = 5;
            queue.put(val);
            count = queue.size(); 
            assert( 1 == count );

            auto ret = queue.get();
            count = queue.size();
            assert( 0 == count );
            assert( ret );
            assert( 5 == *ret );
    };
    threadwrap::this_thread_run<container_type::mem_reclam>(f);

    //spawn some threads
    vector<threadwrap> threads( num_threads );
    vector<threadwrap> threads2( num_threads );    
    for( int i = 0; i < num_threads; ++i ){
        threads[i] = threadwrap( [ &, i ](){
                int val = nums/num_threads*i;
                for( int j = 0; j < nums/num_threads; ++j ){
                    queue.put( val + j ); //blocking put
                }
            }, identity<container_type::mem_reclam>() );
    }

    for( int i = 0; i < num_threads; ++i ){
        threads2[i] = threadwrap( [&](){
                for( int j = 0; j < nums/num_threads; ++j ){
                    if(auto ret = queue.get()){ //non-blocking get
                        //...
                    } 
                }
            }, identity<container_type::mem_reclam>() );
    }
  
    for( auto & i : threads ){
        i.join();
    }
    for( auto & i : threads2 ){
        i.join();
    }
```
  - lockfree synchronous queue w/ hazard pointer:
```
#include "queue_lockfree_sync.hpp"
    using container_type = queue_lockfree_sync<int, trait_reclamation::hp>;
    //... same as above
```
  - lockfree stack w/ hazard pointer:
```
#include "stack_lockfree_total_simple.hpp"
    using container_type = stack_lockfree_total_simple<int, trait_reclamation::hp>;
    //... same as above
```
  - lockfree elimination stack w/ hazard pointer:
```
#include "stack_lockfree_elim.hpp"
    using container_type = stack_lockfree_elim<int, trait_reclamation::hp>;
    //... same as above
```
  - lockfree split reference stack w/ hazard pointer:
```
#include "stack_lockfree_split_reference.hpp"
    using container_type = stack_lockfree_split_reference<int, trait_reclamation::hp>;
    //... same as above
```

# ballpark performance tests
    
 - throughput:

    Hardware: Intel E5-2630, 6 cores, 12 threads

    Test setup: force enqueue / dequeue 1000,000 integers per thread
    
    | container | threads (enqueue/dequeue) | throughput /sec |
    |---|---|---|
    | queue_lockfree_total_hp  | 6/6 | 3.65e6 |
    | queue_lockfree_total_epoch  | 6/6 | 2.83e6 |
    | queue_lockfree_sync_hp | 6/6 | 835422 |
    | stack_lockfree_total_hp | 6/6 | 3.75e6 |
    | stack_lockfree_total_epoch | 6/6 | 4.68e6 |
    | stack_lockfree_elim_hp | 6/6 | 3.70e6 |
    | stack_lockfree_split_reference_hp | 6/6 | 1.55e6 |

  - latency: todo