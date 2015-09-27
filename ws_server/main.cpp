#include "ws_server.h"

ws_server *my_server;

void signal_stop(int signum){
    syslog(LOG_INFO, "ws_server stoped");
    exit(0);
}
    
void handler_SIGUSR1(int signum){
    syslog(LOG_INFO, "SIGUSR1");
    my_server->connections_log();
    //my_server->init_SQL();
    my_server->init_settings();
    my_server->reset_SQL();    
}


int main( int argc, const char* argv[] ) {
        
    const std::string ini_file = "./ws_server.ini";
    std::ifstream my_file;
    my_file.open(ini_file, std::ifstream::in);
    if (!my_file.is_open()){
    	std::cout << "open file error with PID " << getpid() << std::endl;
    	exit(1);
    }
    my_file.close();
    
    signal( SIGINT,  signal_stop );
    signal( SIGTSTP, signal_stop );
    signal( SIGKILL, signal_stop );
    signal( SIGTERM, signal_stop );
    

    std::string output;
    #if defined(BOOST_ASIO_HAS_IOCP)
      output = "Using 'iocp' based polling.";
    #elif defined(BOOST_ASIO_HAS_EPOLL)
      output = "Using 'epoll' based polling." ;
    #elif defined(BOOST_ASIO_HAS_KQUEUE)
      output = "Using 'kqueue' based polling." ;
    #elif defined(BOOST_ASIO_HAS_DEV_POLL)
      output = "Using '/dev/poll' " ;
    #else
      output = "Using 'select' based polling." ;
    #endif
        std::cout << output << std::endl;

    pid_t pid = fork();
    if (pid < 0){
        std::cout << "Error fork child worker process" << std::endl; 
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0){ //процесс = потомок
        close( STDIN_FILENO ); 
        close( STDOUT_FILENO ); 
        close( STDERR_FILENO ); 
        my_server = new ws_server(ini_file);
        openlog("ws_server", LOG_PID, LOG_LOCAL0);
        syslog(LOG_INFO, "ws_server starting... TCP PORT: %d, PID: %d, num threads: %d", my_server->current_settings.general_port, getpid(), my_server->current_settings.general_num_threads);
        signal( SIGUSR1, handler_SIGUSR1 );        
        my_server->run();
    } else { //родитель
        exit(EXIT_SUCCESS);
    }
    
    return 0;    
}