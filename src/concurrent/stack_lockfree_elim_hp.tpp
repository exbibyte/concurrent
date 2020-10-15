#include <iostream>

template< class T >
bool stack_lockfree_elim_impl<T, trait_reclamation::hp>::put(T && val){
    internal_node * n = new internal_node( val );
    return put_aux(n);    
}
template< class T >
bool stack_lockfree_elim_impl<T, trait_reclamation::hp>::put(T const & val){
    internal_node * n = new internal_node( val );
    return put_aux(n);
}

template< class T >
bool stack_lockfree_elim_impl<T, trait_reclamation::hp>::put_aux(internal_node * n){
    while(true){
        if(_stack.try_push(n->_val)){
            delete n;
            return true;
        }else{
            internal_node * nn = n;
            if(_elim_array.visit(nn, 0, _elim_array.array_size())){
                if(nullptr==nn){
                    eliminated.fetch_add(1, std::memory_order_relaxed);
                    return true;
                }
            }
        }
    }
}   

template< class T >
std::optional<T> stack_lockfree_elim_impl<T, trait_reclamation::hp>::get(){
    if(auto ret = _stack.try_pop()){
        return ret;
    }else{
        internal_node * nn = nullptr;
        if(_elim_array.visit(nn, 0, _elim_array.array_size())){
            if(nullptr!=nn){
                T v(std::move(nn->_val));
                assert(nn);
                delete nn;
                eliminated_2.fetch_add(1, std::memory_order_relaxed);
                return std::optional<T>(v);
            }
        }
    }
    return std::nullopt;
}
