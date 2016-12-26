#include <iostream>
#include <random>
#include <boost/thread/thread.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include "shared_mq.h"

using namespace boost::interprocess;

void send_ints(shared_mq& mq, const unsigned count) {
//     std::random_device rd;
//     std::mt19937 mt{ rd() };
//     std::uniform_int_distribution<int> dist{ 0, 10000 };
    for (unsigned i = 0; i != count; ++i) {
        mq.send(i);
    }
}

// int main()
// {
//     std::cout << "Starting client." << std::endl;
//     try {
//         std::cout << "Creating queue..." << std::endl;
//         const unsigned kQueueSize = 100;
//         shared_mq mq("my_queue", kQueueSize);
// 
//         std::cout << "Sending ints..." << std::endl;
//         boost::thread t1(boost::bind(&send_ints, mq, 1));
//         boost::thread t2(boost::bind(&send_ints, mq, 1));
// 
//         t1.join();
//         t2.join();
// 
//         mq.send(-1);  // magic sentinel value
//     }
//     catch (interprocess_exception& ex) {
//         std::cerr << ex.what() << std::endl;
//         return 1;
//     }
// 
//     std::cout << "Finished client." << std::endl;
//     return 0;
// }

int main()
{
    try{
        //Erase previous message queue   
        message_queue::remove("my_queue");

        //Create a message_queue.   
        message_queue mq
            (create_only                //only create   
            , "my_queue"                //name   
            , 1024                       //max message number   
            , 512               //max message size   
            );

        //Send 100 numbers   
        unsigned int priority;
        message_queue::size_type recvd_size;
        char buf[128] = { 0 };
        for (int i = 0; i < 100; ++i){
            sprintf_s(buf, "MSG %d from client", i);
            mq.send(buf, strlen(buf), 0);
        }
    }
    catch (interprocess_exception &ex){
        std::cout << ex.what() << std::endl;
    }

    std::cout << "Finished client." << std::endl;
    getchar();
    return 0;
}