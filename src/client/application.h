#ifndef CS4500_PROJECT_APPLICATION_H
#define CS4500_PROJECT_APPLICATION_H

#include "src/store/kvstore.h"
class Application{
    //each node has one application running
    //each node has one kvstore
    // they can talk
    int idx;
//    net work connection;
    KVstore kvs;
    Application(): idx(0){}
    Application(int index): idx(index){}
    /**
     * This run method starts the all application
     */
    virtual void run_(){}
    /**
     * Return the key of the given node.
     * @param idx given node index
     * @return the key that is homed on master node
     */
    Key* cur_key(size_t idx){}
    /**
     * Megre the dataframe of all nodes(this might not be needed for all apps).
     */
    virtual void reduce(){}
    /**
     * this might all be not needed for computing. It depends on what the app does.
     */
    virtual void merge(){}

};

#endif //CS4500_PROJECT_APPLICATION_H
