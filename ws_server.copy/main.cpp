#include "ws_server.h"
#include <boost/property_tree/ini_parser.hpp>
#include<signal.h>

void signal_stop(int signum){
    std::cout << "stoping worker with PID " << getpid() << "..." << std::endl;
    exit(0);
}

int main( int argc, const char* argv[] ) {
        
    const std::string ini_file = "./ws_server.ini";
    std::ifstream my_file;
    my_file.open(ini_file, std::ifstream::in);
    if (!my_file.is_open()){
    	std::cout << "open file error with PID " << getpid() << std::endl;
    	return 0;
    }

    boost::property_tree::ptree pt;
    boost::property_tree::read_ini(my_file, pt);
    boost::property_tree::ptree pt_child;
    
    settings set;
    
    pt_child                        =            pt.get_child("DATABASE");
    set.database_port               =            pt_child.get("port",                "5432");
    set.database_hostaddr           =            pt_child.get("hostaddr",            "127.0.0.1");
    set.database_dbname             =            pt_child.get("dbname",              "template1");
    set.database_user               =            pt_child.get("user",                "postgres");
    set.database_password           =            pt_child.get("password",            "");
    set.database_connections_number = std::stoi( pt_child.get("connections_number",  "3"));
    set.database_application_name   =            pt_child.get("application_name",    "ws_server");
    
    pt_child                =            pt.get_child("GENERAL");
    set.general_port        = std::stoi( pt_child.get("port", "9000"));
    set.general_crt_file    =            pt_child.get("crt_file", "server.pem");
    set.general_key_file    =            pt_child.get("key_file", "server.pem");
    set.general_maxlifetime = std::stoi( pt_child.get("maxlifetime", "900")); //времея бездействия в секундах после которого отключение клиента
    
    signal( SIGINT,  signal_stop );
    signal( SIGTSTP, signal_stop );
    signal( SIGKILL, signal_stop );
    signal( SIGTERM, signal_stop );


    std::cout << "boost asio use: " << std::endl;
    std::string output;
    #if defined(BOOST_ASIO_HAS_IOCP)
      output = "iocp" ;
    #elif defined(BOOST_ASIO_HAS_EPOLL)
      output = "epoll" ;
    #elif defined(BOOST_ASIO_HAS_KQUEUE)
      output = "kqueue" ;
    #elif defined(BOOST_ASIO_HAS_DEV_POLL)
      output = "/dev/poll" ;
    #else
      output = "select" ;
    #endif
        std::cout << output << std::endl;
    

    pid_t pid = fork();
    if (pid < 0){
        std::cout << "Error fork child worker process" << std::endl; 
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0){ //процесс = потомок
        std::cout << "starting worker with PID: " << getpid() << " TCP PORT: " << set.general_port << " ..." << std::endl;

        ws_server my_server(set);
        my_server.run(set.general_port);
    } else { //родитель
        exit(EXIT_SUCCESS);
    }
    return 0;
    
}