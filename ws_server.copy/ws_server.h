
#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>
#include <map>
#include <set>
#include <iostream>
#include <boost/thread/thread.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <libpq-fe.h>
#include "settings.h"
#include "buy_data.h"
#include "shm_timevalues.h"

using namespace std;
using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef std::set<connection_hdl,std::owner_less<connection_hdl>> con_list;
//typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::server<websocketpp::config::asio_tls> server;
typedef websocketpp::lib::shared_ptr<boost::asio::ssl::context> context_ptr;

typedef std::map<void*, map<std::string, std::string>>::iterator                con_user_data_it;

typedef std::map<void*, map<std::string, std::string>>::iterator                con_data_value_it;
typedef std::map<void*, map<std::string, bool>>::iterator                       con_data_flag_it;
typedef std::map<void*, map<std::string, boost::posix_time::ptime>>::iterator   con_data_time_it;
typedef std::map<boost::posix_time::ptime, map<int, float>>::reverse_iterator   server_data_reverse_it;
    
    
class ws_server {
public:
    ws_server(settings my_settings);
    ~ws_server();
    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_message(connection_hdl hdl, server::message_ptr msg);
    void run(uint16_t port);
    void worker_timer();
    void worker_SQL( connection_hdl hdl, PGconn *conn );
    void worker_SQL_push();
    void worker_SQL_pop();
    void worker_data();
    context_ptr on_tls_init(connection_hdl hdl);
private:;
    std::mutex mtx;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;
    boost::thread* m_ptr_Thread1;
    boost::thread* m_ptr_Thread2;
    server m_server;
    con_list m_connections;
    map <void*, map<std::string, std::string>> con_user_data;
    
    map <void*, map<std::string, std::string>>               con_data_value; // строковые значения по строковому ключу
    map <void*, map<std::string, bool>>                      con_data_flag;  // флаги по строковому ключу
    map <void*, map<std::string, boost::posix_time::ptime>>  con_data_time;  // времена по строковому ключу
    
    map <int, PGconn*> db_conn;
    map <connection_hdl*, boost::thread*> hdl_workers;
    //map <connection_hdl*, std::string> hdl_auth;
    
    map <boost::posix_time::ptime, map<int, float>> server_data;
    
    int DBCONNECTIONS_COUNT;
    queue<connection_hdl> SQL_QUEUE;
    queue<PGconn*> PG_QUEUE;
    long count_msg;
    boost::condition_variable Condition_;
    
    boost::thread *thread_SQL_push;
    boost::thread *thread_SQL_pop;
    boost::thread *thread_timer;
    boost::thread *thread_data;
    BuyValues_set buy_set;
    
    std::string key_file;
    std::string crt_file;
    long maxlifetime;
    
    std::string ptime_tostr(boost::posix_time::ptime date_time);
    

};
