#ifndef IFUNCWRAP_H
#define IFUNCWRAP_H

class IFuncWrap {
public:
    ~IFuncWrap(){}
    template< class F, class... Args >
    virtual void set( FunCallType calltype, F f, Args... args ){}
    virtual void apply(){}
};

#endif
