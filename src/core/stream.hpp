#ifndef STREAM_H
#define STREAM_H

#include "queue_lock_free.hpp"

templace< typename T >
class stream {
public:
    using _t_item = T;
    enum class eProcessArgType {
	START = 0,
	END,
	EMPTY_STREAM,
	ADD_INSPECTOR,
	REMOVE_INSPECTOR,
	RESET_INSPECTOR,
    };
    bool init();
    bool deinit();
    unit process( eProcessArgType );
private:
    bool start();
    bool end();
    bool add_inspector();
    bool remove_inspector();
    queue_lock_free< _t_item > _queue;
};

#endif
