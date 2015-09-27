#include "shm_timevalues.h"

int main( int argc, const char* argv[] ) {

    pid_t pid = fork();
    if (pid < 0){
        std::cout << "Error fork child worker process" << std::endl; 
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0){ //процесс = потомок
        std::cout << "starting with PID: " << getpid() << " ..." << std::endl;
    } else { //родитель
        exit(EXIT_SUCCESS);
    }

    boost::interprocess::shared_memory_object::remove("ws_server_data");
    managed_shared_memory segment(boost::interprocess::open_or_create,"ws_server_data", mapped_region::get_page_size() * 20000);
    TimeValues_set *es = segment.construct<TimeValues_set>("TimeValues") ( TimeValues_set::ctor_args_list(), segment.get_allocator<TimeValues>() );

    boost::posix_time::ptime now_old = boost::posix_time::second_clock::local_time();
    
    for(;;){
        boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
        //if (now_old != now){ // наступила новая секундв
        //    now_old = now;

            int LO = 10000;
            int HI = 10010;

            float val = LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX / ( HI - LO )));
            val = val / (float) 10;
            es->insert(TimeValues( val, 1, now));
        
            auto it = es->begin();
            boost::posix_time::ptime my_time;
        
            if ( my_time < ( now - boost::posix_time::seconds(3600) ) ){
                es->erase( it );
            }
            
        //}
        boost::this_thread::sleep(boost::posix_time::milliseconds( 500 ));
    }


    return 0;
	
}