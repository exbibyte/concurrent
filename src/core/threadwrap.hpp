#ifndef THREADWRAP_H
#define THREADWRAP_H

template<typename T> struct identity { typedef T type; };

class threadwrap {
public:
    threadwrap(){}

    //pass in memory managers as Ts
    template< class ... Ts >
    threadwrap(std::function<void()> f, identity<Ts> ... ts){
        t = std::thread([=](){
                (Ts::thread_init(), ...);
                f();
                (Ts::thread_deinit(), ...);
            });
    }

    //pass in memory managers as Ts
    template< class ... Ts >
    static void this_thread_run(std::function<void()> f){
        (Ts::thread_init(), ...);
        f();
        (Ts::thread_deinit(), ...);
    }
    threadwrap( threadwrap const & ) = delete;
    threadwrap( threadwrap && other ) = default;
    threadwrap& operator=(threadwrap const & other) = delete;
    threadwrap& operator=(threadwrap && other){
        t = std::move(other.t);
        return *this;
    }
    void join(){
        t.join();
    }
private:
    std::thread t;
};

#endif
