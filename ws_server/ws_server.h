
#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>
#include <map>
#include <set>
#include <iostream>
#include <boost/thread/thread.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <libpq-fe.h>
#include "settings.h"
#include "buy_data.h"
#include "shm_timevalues.h"
#include <syslog.h>
#include <signal.h>


using namespace std;
using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef std::set<connection_hdl,std::owner_less<connection_hdl>> con_list;
//typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::server<websocketpp::config::asio_tls> server;
typedef websocketpp::lib::shared_ptr<boost::asio::ssl::context> context_ptr;

//typedef std::map<void*, map<std::string, std::string>>::iterator                con_user_data_it;

enum class UserFlags {
    AUTH,               // авторизация
    LOGIN,              // 
    GET_TIME,           // получать время
    LAST_ACT,           // время последнего действия
    LAST_TIME,          //
    LAST_GRAPH,         //
    IP_ADDR,            //
    GRAPH,              // подписан на получение графика
    INFO                //
};

typedef std::map<void*, map<UserFlags, std::string>>::iterator                 con_data_value_it;
typedef std::map<void*, map<UserFlags, bool>>::iterator                        con_data_flag_it;
typedef std::map<void*, map<UserFlags, boost::posix_time::ptime>>::iterator    con_data_time_it;
typedef std::map<boost::posix_time::ptime, map<int, float>>::reverse_iterator  server_data_reverse_it;
//typedef websocketpp::client<websocketpp::config::asio_client> client;


struct sql_hdl{
    std::string sql;
    connection_hdl hdl;
    std::string ev;
    bool send_data;
};
  
class ws_server {
public:
    ws_server(std::string ini);
    ~ws_server();
    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    bool validate(connection_hdl hdl);
    void on_pre_init(connection_hdl hdl);
    void on_message(connection_hdl hdl, server::message_ptr msg);
    void run();
    void worker_timer();
    void worker_SQL( connection_hdl hdl, PGconn *conn, std::string SQL, std::string event_name, bool send_data );
    void worker_SQL_pop();
    void worker_data_ba();
    void load_data();

    void init_settings();
    void init_TLS();
    void init_BA();
    void init_SQL();
    void reset_SQL();
        
    context_ptr on_tls_init(connection_hdl hdl);
    settings current_settings;
    void connections_log();
private:;
    
    
    std::string ini_file;
    std::mutex mtx;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;
    boost::thread* m_ptr_Thread1;
    boost::thread* m_ptr_Thread2;
    server m_server;
    con_list m_connections;
    map <void*, map<std::string, std::string>> con_user_data;
    
    map <void*, map<UserFlags, std::string>>               con_data_value; // строковые значения по строковому ключу
    map <void*, map<UserFlags, bool>>                      con_data_flag;  // флаги по строковому ключу
    map <void*, map<UserFlags, boost::posix_time::ptime>>  con_data_time;  // времена по строковому ключу
    
    
    unsigned int limit_rlim_cur;
   
    //базисные активы

    //опционы

    //условия опционов
    map<int, map<float, float>> opt_wins;
        //107 <0, 0>
    
    
    map <int, PGconn*> db_conn;
    map <connection_hdl*, boost::thread*> hdl_workers;
    //map <connection_hdl*, std::string> hdl_auth;
    
    map <boost::posix_time::ptime, map<int, float>> server_data;
    
    bool accept_new_connections;    
    
    queue<sql_hdl> SQLHDL_QUEUE;
    
    queue<connection_hdl> SQL_QUEUE;
    queue<PGconn*> PG_QUEUE;
    long count_msg;
    boost::condition_variable Condition_;

    boost::thread *thread_SQL_pop;
    BuyValues_set buy_set;
    
    std::string key_file;
    std::string crt_file;
    
    std::string ptime_tostr(boost::posix_time::ptime date_time);
    
    void on_timer(websocketpp::lib::error_code const & ec);
    void set_timer();
    void on_timer_ba(websocketpp::lib::error_code const & ec);
    void set_timer_ba();
    
    void on_timer_SQL_push(websocketpp::lib::error_code const & ec);
    void set_timer_SQL_push();
    void on_timer_SQL_pop(websocketpp::lib::error_code const & ec);
    void set_timer_SQL_pop();

    void *hdl_test;
    context_ptr ctx;
    
    managed_shared_memory *segment;
    TimeValues_set *es;

};
