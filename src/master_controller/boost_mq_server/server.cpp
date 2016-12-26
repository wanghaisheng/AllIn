#include <iostream>
#include "shared_mq.h"

using namespace boost::interprocess;

int main()
{
//     std::cout << "Starting server." << std::endl;
//     try {
//         std::cout << "Opening queue..." << std::endl;
//         shared_mq mq("my_queue");
// 
//         std::cout << "Receiving ints..." << std::endl;
//         for (;;) {
//             const int x = mq.receive();
//             if (x == -1) {
//                 // magic sentinel value
//                 break;
//             }
//             std::cout << "Received: " << x << std::endl;
//         }
//     }
//     catch (boost::interprocess::interprocess_exception& ex) {
//         std::cerr << ex.what() << std::endl;
//         return 1;
//     }
// 
//     std::cout << "Finished server." << std::endl;
//     return 0;


    try{
        //Open a message queue.   
        message_queue mq
            (open_only        //only create   
            , "my_queue"    //name   
            );

        unsigned int priority;
        message_queue::size_type recvd_size;
        //Receive 100 numbers   
        char buf[512] = { 0 };
        for (int i = 0; i < 100; ++i){
            int number;
            mq.receive(buf, sizeof(buf), recvd_size, priority);
            printf("Server received: %s\n", buf);

//             if (number != i || recvd_size != sizeof(number))
//                 return 1;
        }
    }
    catch (interprocess_exception &ex){
        message_queue::remove("my_queue");
        std::cout << ex.what() << std::endl;
        //return 1;
    }

    //message_queue::remove("my_queue");
    std::cout << "Finished server." << std::endl;
    getchar();
    return 0;
}
