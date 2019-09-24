#ifndef ICONCURRENCY_HPP
#define ICONCURRENCY_HPP

enum class trait_concurrency {
    none,
    global,
    granular,
    lockfree,
    waitfree,
};

enum class trait_method {
    total,
    partial,
    synchronous,
};

enum class trait_fairness {
    fifo,
    lifo,
};

#endif
