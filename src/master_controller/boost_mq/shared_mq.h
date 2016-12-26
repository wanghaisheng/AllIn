#include <boost/interprocess/ipc/message_queue.hpp>

// could easily be made a template; make sure T is a POD!
class shared_mq {
public:
    shared_mq(const char* const name,
        const unsigned max_queue_size) {
        shared_mq(name, max_queue_size, delete_queue(name));
    }

    shared_mq(const char* const name) {
        mq_ = new boost::interprocess::message_queue(
            boost::interprocess::open_only,
            name);
    }

    void send(int i) {
        mq_->send(&i, sizeof(i), 0 /* priority */);
    }

    int receive() {
        int result;
        boost::interprocess::message_queue::size_type recvsize;
        unsigned recvpriority;
        mq_->receive(&result, sizeof(result), recvsize, recvpriority);
        return result;
    }

private:
    struct did_delete_t {};

    did_delete_t delete_queue(const char* const name) {
        boost::interprocess::message_queue::remove(name);
        return did_delete_t();
    }

    shared_mq(
        const char* const name,
        const unsigned max_queue_size,
        did_delete_t) {
        mq_ = new boost::interprocess::message_queue(
            boost::interprocess::create_only,
            name, 
            max_queue_size, 
            sizeof(int));
    }

    boost::interprocess::message_queue* mq_;
};
