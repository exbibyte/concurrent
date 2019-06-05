#ifndef WORKERPOOL0_H
#define WORKERPOOL0_H

#include <vector>

template< template<class> class SourceQueue, class WorkType >
class Workerpool0 : public IWorkerpool {
public:
    void set_worker_num( unsigned int ){}
    void set_action( IWorkerpool::Action ){}
private:
    SourceQueue<WorkType> _queue_work;
    std::vector<IWorker*> _workers;
};

#endif
