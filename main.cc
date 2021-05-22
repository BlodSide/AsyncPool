#include <iostream>
#include <cstdlib>
#include <unistd.h>

#include "async_pool.h"

int main()
{
    AsyncPool pool(3);
    std::vector<std::future<void>> vec;
    for (int i =0; i< 10; i++) {
        vec.push_back(pool.async([i](){
            std::cout << "i=" + std::to_string(i) + " begin \n";
            usleep(rand() % 100 + 1000);
            std::cout << "i=" + std::to_string(i) + " end \n";}));

    }
    for (auto& f: vec) f.wait();
    std::cout << "Finish!" << std::endl;
}

/*
program_output: 
i=1 begin 
i=0 begin 
i=2 begin 
i=1 end 
i=3 begin 
i=2 end 
i=0 end 
i=4 begin 
i=5 begin 
i=3 end 
i=6 begin 
i=5 end 
i=7 begin 
i=4 end 
i=8 begin 
i=7 end 
i=9 begin 
i=6 end 
i=8 end 
i=9 end 
Finish!

*/
