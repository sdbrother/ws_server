#include "ws_server.h"
#include <sys/resource.h>

ws_server::ws_server(std::string ini) {
    ini_file = ini;
    init_settings(); //инициализация настроек
    
    if (current_settings.general_num_threads == 0){
        syslog(LOG_ERR, "Error num_threads == 0, exit");    
        exit(1);
    }
    
   
    struct rlimit limit;
    getrlimit(RLIMIT_NOFILE, &limit);     
    limit_rlim_cur = limit.rlim_cur;
   
    m_server.clear_access_channels( websocketpp::log::alevel::all );
    m_server.clear_error_channels( websocketpp::log::alevel::all );
    m_server.set_access_channels( websocketpp::log::alevel::none );
    m_server.set_error_channels( websocketpp::log::alevel::none );
    m_server.set_user_agent("WOTS-1.0.0");
    m_server.set_reuse_addr(true);
    m_server.init_asio();
    m_server.set_tcp_pre_init_handler(bind( &ws_server::on_pre_init, this, ::_1));
    m_server.set_tls_init_handler(bind( &ws_server::on_tls_init,     this, ::_1));
    m_server.set_validate_handler(bind( &ws_server::validate,        this, ::_1));
    m_server.set_open_handler(    bind( &ws_server::on_open,         this, ::_1));
    m_server.set_message_handler( bind( &ws_server::on_message,      this, ::_1, ::_2));                                    
    m_server.set_close_handler(   bind( &ws_server::on_close,        this, ::_1));
    
    init_BA();   // Инициализация значений базисных активов из SHM
    init_TLS();  // Инициализация TLS - защищенные соединения sll (сертификат, ключ, Диффи-Хелман)
    init_SQL();  // инициализация соединений с PostgreSQL
    accept_new_connections = true; // Разрешить принимать новые соединения       

    //accept_new_connections = false;       
}

//Завершение работы
ws_server::~ws_server(){
    syslog(LOG_INFO, "ws_server stoping...");
    for (auto it : m_connections) {
        websocketpp::lib::error_code ec;
        connection_hdl hdl = it;
        m_server.close( hdl, websocketpp::close::status::going_away, "", ec);
    }
    m_server.stop_listening();
    for(;;){
        if (mtx.try_lock()){
            break;
        }
        boost::this_thread::sleep(boost::posix_time::milliseconds( 10 ));        
    }
}

void ws_server::connections_log(){
    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    std::stringstream ss_now;
    ss_now << now;
    ss_now << " " << m_connections.size();
    ss_now << " " << con_data_value.size();
    ss_now << " " << con_data_flag.size();
    ss_now << " " << con_data_time.size();
    std::string str_now = ss_now.str();
    syslog( LOG_INFO, "TIMER %s", str_now.c_str());    
}

void ws_server::init_settings(){
    std::ifstream my_file;
    my_file.open(ini_file, std::ifstream::in);
    if (!my_file.is_open()){
        syslog(LOG_ERR, "could not open \"%s\" settings file!", ini_file.c_str());
    } else {
        boost::property_tree::ptree pt;
        boost::property_tree::read_ini(my_file, pt);
        boost::property_tree::ptree pt_child;
        
        pt_child = pt.get_child("DATABASE");
        current_settings.database_port               =            pt_child.get("port",                "5432");
        current_settings.database_hostaddr           =            pt_child.get("hostaddr",            "127.0.0.1");
        current_settings.database_dbname             =            pt_child.get("dbname",              "template1");
        current_settings.database_user               =            pt_child.get("user",                "postgres");
        current_settings.database_password           =            pt_child.get("password",            "");
        current_settings.database_connections_number = std::stoi( pt_child.get("connections_number",  "3"));
        current_settings.database_application_name   =            pt_child.get("application_name",    "ws_server");
        
        pt_child = pt.get_child("GENERAL");
        current_settings.general_port        = std::stoi( pt_child.get("port", "9000"));
        current_settings.general_crt_file    =            pt_child.get("crt_file", "server.pem");
        current_settings.general_key_file    =            pt_child.get("key_file", "server.pem");
        current_settings.general_maxlifetime = std::stoi( pt_child.get("maxlifetime", "900")); //времея бездействия в секундах после которого отключение клиента
        current_settings.general_num_threads = std::stoi( pt_child.get("num_threads", "1"));
        my_file.close();        
    }
}

void ws_server::init_TLS(){
    ws_server::key_file    = current_settings.general_key_file;
    ws_server::crt_file    = current_settings.general_crt_file;
    std::ifstream crt_file( ws_server::crt_file );
    std::ifstream key_file( ws_server::key_file );
    if ( !crt_file) {
        syslog(LOG_ERR, "could not open \"%s\" certificate file! exit", ws_server::crt_file.c_str());
        exit(0);
    }
    if ( !key_file) {
        syslog(LOG_ERR, "could not open \"%s\" key file! exit", ws_server::key_file.c_str());
        exit(0);
    }
    ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23); 
    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds | 
                         boost::asio::ssl::context::no_sslv2 | 
                         boost::asio::ssl::context::no_sslv3 | 
                         boost::asio::ssl::context::no_tlsv1 | 
                         boost::asio::ssl::context::single_dh_use);        
        ctx->use_certificate_chain_file( ws_server::crt_file );
        ctx->use_private_key_file( ws_server::key_file , boost::asio::ssl::context::pem);    
        ctx->use_tmp_dh_file("dh.pem");                       
    } catch (std::exception& e) {
        syslog(LOG_ERR, "TLS_ERROR %s", e.what());
    }
}

void ws_server::init_BA(){
    //инициализация данныз базисных активов
    try {
        segment = new managed_shared_memory(open_only,"ws_server_data");
        es = segment->find<TimeValues_set>("TimeValues").first;
        for (auto it = es->begin(); it != es->end(); ++it){
            server_data[it->time][107] = it->value1;    
        }
    } catch (std::exception& e) {
        //std::cout << "Error open shared memory segment \"ws_server_data\"" << " exit" << std::endl;    
        syslog(LOG_ERR, "Error open shared memory segment \"ws_server_data\". exit");    
        exit(1);
    }  
}

void ws_server::init_SQL(){
    std::string connection_string = "hostaddr="          + current_settings.database_hostaddr + 
                                    " port="             + current_settings.database_port + 
                                    " dbname="           + current_settings.database_dbname + 
                                    " user="             + current_settings.database_user + 
                                    " password="         + current_settings.database_password + 
                                    " application_name=" + current_settings.database_application_name;

    for (int c = 0; c < current_settings.database_connections_number; c++){
        db_conn[c] = PQconnectdb( connection_string.c_str() );
        if (PQstatus(db_conn[c]) == CONNECTION_BAD){
            syslog(LOG_ERR, "ERROR: could not connect to PostgreSQL server. Connection string: \"%s\"", connection_string.c_str()); 
        }
    }    
}

void ws_server::reset_SQL(){
    for (int c = 0; c < current_settings.database_connections_number; c++){
        if ( PQstatus(db_conn[c]) == CONNECTION_BAD ){
            PQreset(db_conn[c]);
        }
    }    
}

void ws_server::on_pre_init(websocketpp::connection_hdl hdl){
    //Перед TLS перед "рукопожатием"
}

bool ws_server::validate(websocketpp::connection_hdl hdl) {
    return accept_new_connections;
}

void ws_server::run() {
    try{
        m_server.listen(current_settings.general_port);
        m_server.start_accept();

        set_timer(); //каждусю секунду - главный таймер отправка данных клиентам
        set_timer_ba(); //чтение данных БА из SHM
        set_timer_SQL_pop();
        
        typedef websocketpp::lib::shared_ptr<websocketpp::lib::thread> thread_ptr;
        std::vector<thread_ptr> ts;
        for (size_t i = 0; i < current_settings.general_num_threads; i++) {
            ts.push_back(websocketpp::lib::make_shared<websocketpp::lib::thread>(&server::run, &m_server));
        }
        for (size_t i = 0; i < current_settings.general_num_threads; i++) {
            ts[i]->join();
        }  
              
    } catch (websocketpp::exception e){
        cout << e.what() << endl;
    }
}

void ws_server::set_timer_SQL_pop(){
    server::timer_ptr m_timer = m_server.set_timer(10, websocketpp::lib::bind( &ws_server::on_timer_SQL_pop, this, websocketpp::lib::placeholders::_1));
}

void ws_server::set_timer_ba(){
    server::timer_ptr m_timer = m_server.set_timer(500, websocketpp::lib::bind( &ws_server::on_timer_ba, this, websocketpp::lib::placeholders::_1));
}

void ws_server::on_timer_ba(websocketpp::lib::error_code const & ec) {
    try {
        const boost::posix_time::ptime t = boost::posix_time::second_clock::local_time() - boost::posix_time::seconds(5);
        auto it_find = es->get<index_by_t_id_ba>().find(std::make_tuple(t, 1));
        for (auto it = it_find; it != es->end(); ++it){
            server_data[it->time][107] = it->value1;    
        }
        if ( server_data.size() > 300 ){
            server_data.erase( server_data.begin()->first );
        }
    } catch (std::exception& e) {
        std::cout << "Error open shared memory segment \"ws_server_data\"" << std::endl;        
    }
    set_timer_ba();    
}

void ws_server::set_timer(){
    server::timer_ptr m_timer = m_server.set_timer(1000, websocketpp::lib::bind( &ws_server::on_timer, this, websocketpp::lib::placeholders::_1));
}

//Отправка сообщений клиентам:
void ws_server::on_timer(websocketpp::lib::error_code const & ec) {
    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();

    //connections_log();

    //Перебор соединений
    for (auto it : m_connections) {
        //ТЕКУЩЕЕ ВРЕМЯ:                
        if( con_data_flag[ it.lock().get() ][UserFlags::GET_TIME] == true ){
            server_data_reverse_it graph_it = server_data.rbegin();
            try{
                std::stringstream message_stream;
                message_stream 
                <<  "{"
                <<      quoted("event") << ":"
                <<          "{"
                <<             quoted("name") << ":" << quoted("TIME") << ","
                //<<             quoted("time") << ":" << quoted(ptime_tostr(now))
                <<             quoted("time") << ":" << quoted( ptime_tostr(graph_it->first) )  
                <<         "}"
                <<  "}";                        
                m_server.send(it, message_stream.str()  , websocketpp::frame::opcode::text);
            } catch(websocketpp::exception e){
                cout << e.what() << endl;
            }                
        }
                
        //Отсылка графика
        if( con_data_flag[ it.lock().get() ][UserFlags::GRAPH] == true ){
            try{
                server_data_reverse_it graph_it = server_data.rbegin();
                if (graph_it->second[107] != 0){
                    std::stringstream message_stream;
                    message_stream 
                    <<  "{"
                    <<      quoted("event") << ":"
                    <<          "{"
                    <<             quoted( "name" )  << ":" << quoted( "GRAPH" )     << ","
                    <<             quoted( "data" )  << ":"
                    <<             "["
                    <<                  "{"
                    <<                      quoted( "time" )   << ":" << quoted( ptime_tostr(graph_it->first) ) << ","
                    <<                      quoted( "value" )  << ":" << graph_it->second[107]
                    <<                  "}"  
                    <<             "]" 
                    <<         "}"
                    <<  "}";
                    m_server.send(it, message_stream.str(), websocketpp::frame::opcode::text);
                }
            } catch(websocketpp::exception e){
                cout << e.what() << endl;
            }                
        } 
                
        // получать системную информацию
        /*
        if ( con_data_flag[ it.lock().get() ][UserFlags::INFO] == true){
            try{
                for (auto it_connections : m_connections) {
                    std::cout << 
                        con_data_value[ it_connections.lock().get() ][UserFlags::IP_ADDR] << "\t" <<
                        con_data_value[ it_connections.lock().get() ][UserFlags::LOGIN] << "\t" 
                        << std::endl;
                }
            } catch(websocketpp::exception e){
                cout << e.what() << endl;
            }                           
        }
        */                       
        //РЕЗУЛЬТАТЫ ПО КУПЛЕННЫМ ОПЦИОНАМ
        //перебор сделанных покупок
        for (auto it_buy = buy_set.begin(); it_buy != buy_set.end(); ++it_buy){
            try {
                if ( it_buy->timeExe <= server_data.rbegin()->first && con_data_value[ it.lock().get() ][UserFlags::LOGIN] == it_buy->user){
                    //std::cout << "it_buy->timeExe <= server_data.rbegin()->first" << std::endl;
                    std::stringstream message_stream;
                    message_stream 
                    <<  "{"
                    <<      quoted("event") << ":"
                    <<          "{"
                    <<             quoted( "name"     )  << ":" << quoted("RES") << ","
                    <<             quoted( "timeBuy"  )  << ":" << quoted(ptime_tostr(it_buy->timeBuy)) << ","
                    <<             quoted( "timeExe"  )  << ":" << quoted(ptime_tostr(it_buy->timeExe)) << "," 
                    <<             quoted( "option"   )  << ":" << it_buy->option  << "," 
                    <<             quoted( "quantity" )  << ":" << it_buy->quantity << ","
                    <<             quoted( "valueBuy" )  << ":" << server_data[it_buy->timeBuy][107] << ","
                    <<             quoted( "valueExe" )  << ":" << server_data[it_buy->timeExe][107]
                    <<         "}"
                    <<  "}";                               
                    m_server.send(it, message_stream.str(), websocketpp::frame::opcode::text);    
                    buy_set.erase(it_buy);
                }                        
            } catch(websocketpp::exception e){
                cout << e.what() << endl;
            }                                              
        }
        
        //разлогин после maxlife бездействия
        /*
        if ( con_data_time[ it.lock().get() ][UserFlags::LAST_ACT] + boost::posix_time::seconds( current_settings.general_maxlifetime ) <= now  ) {
            try {
                std::stringstream message_stream;
                message_stream 
                <<  "{"
                <<      quoted("event") << ":"
                <<          "{"
                <<             quoted("name") << ":" << quoted("LOGOUT") 
                <<         "}"
                <<  "}";
                m_server.send( it,message_stream.str(), websocketpp::frame::opcode::text);                    
                        
                websocketpp::lib::error_code ec;
                m_server.close( it, websocketpp::close::status::going_away, "", ec);       
            } catch(websocketpp::exception e){
                cout << e.what() << endl;
            }   
        }
        */           
    }
    set_timer();
}



void ws_server::on_open(connection_hdl hdl) {
//    accept_new_connections = false;

    if ( m_connections.size() + 10 + current_settings.database_connections_number >= limit_rlim_cur){
        accept_new_connections = false;    
    }

    std::cout << "OPEN" << std::endl;
    m_connections.insert(hdl);
    con_data_flag[ hdl.lock().get() ][ UserFlags::AUTH ]     = false; //не авторизован
    con_data_flag[ hdl.lock().get() ][ UserFlags::GET_TIME ] = false; //не получает время
    con_data_time[ hdl.lock().get() ][ UserFlags::LAST_ACT ] = boost::posix_time::second_clock::local_time(); //время последнего действия
    con_data_value[ hdl.lock().get()][ UserFlags::IP_ADDR ]  =  m_server.get_con_from_hdl(hdl)->get_remote_endpoint(); //IP адрес клиента        
    std::cout << "NEW connection from IP ADDR: "<< con_data_value[ hdl.lock().get() ][UserFlags::IP_ADDR] << std::endl;
    /*
    struct rlimit limit;
    getrlimit(RLIMIT_NOFILE, &limit);     
    if ( m_connections.size() + 10 + current_settings.database_connections_number >= limit.rlim_cur){
        accept_new_connections = false;    
    }
    */
    //если последнее соединение - то новые не принимать
            
}

void ws_server::on_close(connection_hdl hdl) {
     std::cout << "CLOSE" << std::endl;
    //освобождление параметров сессии "значения"
    for (con_data_value_it iterator = con_data_value.begin(); iterator != con_data_value.end(); iterator++) {            
        if ( iterator->first == hdl.lock().get() ) {
            con_data_value.erase(iterator);
            break;
        }
    }
    //освобождения параметров сессии "флаги"
    for (con_data_flag_it iterator = con_data_flag.begin(); iterator != con_data_flag.end(); iterator++) {            
        if ( iterator->first == hdl.lock().get() ) {
            con_data_flag.erase(iterator);
            break;
        }
    }
    //освобождение параметров сессии "дата_время"
    for (con_data_time_it iterator = con_data_time.begin(); iterator != con_data_time.end(); iterator++) {            
        if ( iterator->first == hdl.lock().get() ) {
            con_data_time.erase(iterator);
            break;
        }
    }    
    std::cout << "CLOSE connection" << std::endl;
    m_connections.erase(hdl);   
    accept_new_connections = true; 
}

void ws_server::on_message(connection_hdl hdl, server::message_ptr msg) {
    try{
        auto data = msg->get_payload();
        for (auto it : m_connections) {
            if( hdl.lock().get() == it.lock().get() ){
                std::stringstream my_message;
                my_message << data;
                try {
                    boost::property_tree::ptree pt;
                    boost::property_tree::json_parser::read_json(my_message, pt);

                    boost::property_tree::ptree::iterator iter = pt.begin();     
                    std::string action = iter->first;
                        
                    if (con_data_flag[ hdl.lock().get() ][UserFlags::AUTH] == false && action == "auth"){ 
                        if (action == "auth"){
                            boost::property_tree::ptree pt_child = pt.get_child(action, boost::property_tree::ptree());
                            
                            std::string login    = pt_child.get<std::string>("login");
                            std::string password = pt_child.get<std::string>("password");

                            if (login =="user1" && password == "123"){ // успешная авторизация
                                con_data_flag[ hdl.lock().get() ][UserFlags::AUTH]     = true;
                                con_data_value[ hdl.lock().get()][UserFlags::LOGIN]    = login;
                                con_data_time[ hdl.lock().get() ][UserFlags::LAST_ACT] = boost::posix_time::second_clock::local_time();
                                // событие успешного входа
                                std::stringstream message_stream;
                                message_stream
                                <<  "{"
                                <<      quoted("event") << ":"
                                <<          "{"
                                <<             quoted("name") << ":" << quoted("LOGIN")  
                                <<         "}"
                                <<  "}";
                                m_server.send(it, message_stream.str(), websocketpp::frame::opcode::text);
                            } else {  
                                websocketpp::lib::error_code ec;
                                m_server.close( hdl, websocketpp::close::status::going_away, "", ec);
                            }                         
                        }                            
                    } else if (con_data_flag[ hdl.lock().get() ][UserFlags::AUTH] == true && action != "auth"){
                        // тестирование SQL
                        if (action == "SQL"){
                            for (;;){
                                if (mtx.try_lock()){
                                    //SQLHDL_QUEUE.push( {"SELECT pg_sleep(10); SELECT * FROM t1 ORDER BY random() LIMIT 1;", hdl, true} );
                                    SQLHDL_QUEUE.push( {"SELECT * FROM t1 ORDER BY random() LIMIT 2;", hdl, action, true} );
                                    std::stringstream message_stream;
                                    message_stream 
                                    <<  "{"
                                    <<      quoted("event") << ":"
                                    <<          "{"
                                    <<             quoted("name") << ":" <<  quoted("SQL") 
                                    <<         "}"
                                    <<  "}";
                                    m_server.send(hdl,message_stream.str(), websocketpp::frame::opcode::text);
                                    mtx.unlock();
                                    break;
                                } else {
                                          
                                }
                            }
                        }
                        if (action == "SQL2"){
                            for (;;){
                                if (mtx.try_lock()){
                                    SQLHDL_QUEUE.push( {"INSERT INTO t1(i) VALUES(1);", hdl, action, false} );
                                    std::stringstream message_stream;
                                    message_stream 
                                    <<  "{"
                                    <<      quoted("event") << ":"
                                    <<          "{"
                                    <<             quoted("name") << ":" <<  quoted("SQL") 
                                    <<         "}"
                                    <<  "}";
                                    m_server.send(hdl,message_stream.str(), websocketpp::frame::opcode::text);
                                    mtx.unlock();
                                    break;
                                } else {
                                          
                                }
                            }
                        }

                            
                        // Подписка на получение времени
                        if (action == "time"){
                            boost::property_tree::ptree pt_child = pt.get_child(action, boost::property_tree::ptree());
                            con_data_time[ hdl.lock().get() ][UserFlags::LAST_ACT] = boost::posix_time::second_clock::local_time();
                            int get = pt_child.get<int>("get");
                            if (get == 1) con_data_flag[ hdl.lock().get() ][UserFlags::GET_TIME] = true;
                            if (get == 0) con_data_flag[ hdl.lock().get() ][UserFlags::GET_TIME] = false;
                        }
                        
                        //Подписка на получение графика
                        if (action == "graph"){
                            
                            boost::property_tree::ptree pt_child = pt.get_child(action, boost::property_tree::ptree());
                            con_data_time[ hdl.lock().get() ][UserFlags::LAST_ACT] = boost::posix_time::second_clock::local_time();
                            int option = pt_child.get<int>("option");
                            int get    = pt_child.get<int>("get");

                            if ( get <= 300 && get > 0 ){

                                std::stringstream message_stream;
                                message_stream 
                                <<  "{"
                                <<      quoted("event") << ":"
                                <<          "{"
                                <<             quoted( "name" )  << ":" << quoted( "GRAPH" )     << ","
                                <<             quoted( "data" )  << ":"
                                <<             "[";
                                                    int i = 0;
                                                    for (server_data_reverse_it it = server_data.rbegin(); it != server_data.rend(); ++it){
                                                        i++;
                                message_stream
                                <<                      "{"
                                <<                      quoted( "time" )   << ":" << quoted( ptime_tostr( it->first ) ) << ","
                                <<                      quoted( "value" )  << ":" << it->second[107]
                                <<                      "}";
                                                        if (i >= get){
                                                            break;
                                                        } else {
                                message_stream                                                            
                                <<                          ",";                                                            
                                                        }
                                                    }
                                message_stream
                                <<             "]" 
                                <<         "}"
                                <<  "}";
                                m_server.send(it, message_stream.str(), websocketpp::frame::opcode::text);

                                con_data_flag[ hdl.lock().get() ][UserFlags::GRAPH] = true;
                            } else {
                                con_data_flag[ hdl.lock().get() ][UserFlags::GRAPH] = false;    
                            }
                        }
                        
                                                        
                        // Покупка опциона
                        if (action == "buy"){
                            boost::property_tree::ptree pt_child = pt.get_child(action, boost::property_tree::ptree());
                            const boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
                            float value1 = 0; 
                            int option = 0;
                            int quantity = 0; 
                            boost::posix_time::ptime time = now; 
                            
                            // массив объектов
                            for( boost::property_tree::ptree::iterator iter = pt_child.begin(); iter != pt_child.end(); iter++ ) {
                                option   = iter->second.get<int>("option");
                                quantity = iter->second.get<int>("quantity");
                                std::string user = con_data_value[ hdl.lock().get() ][UserFlags::LOGIN];
                                    
                                if (option > 0 && quantity > 0){
                                    boost::posix_time::ptime timeBuy = now;
                                    boost::posix_time::ptime timeExe = now + boost::posix_time::seconds( 3 );
                                    BuyValues rec = {value1, option, quantity, user, timeBuy, timeExe };
                                        
                                    // событие покупки опциона
                                    std::stringstream message_stream;
                                    message_stream 
                                    <<  "{"
                                    <<      quoted("event") << ":"
                                    <<          "{"
                                    <<             quoted( "name" )     << ":" << quoted( "BUY" )     << ","
                                    <<             quoted( "timeBuy" )  << ":" << quoted( ptime_tostr(timeBuy) ) << ","
                                    <<             quoted( "timeExe" )  << ":" << quoted( ptime_tostr(timeExe) ) << ","
                                    <<             quoted( "option" )   << ":" << option << ","
                                    <<             quoted( "quantity" ) << ":" << quantity
                                    <<         "}"
                                    <<  "}";
                                        
                                    bool inserted = buy_set.insert(rec).second;
                                    if (inserted){
                                        m_server.send(it, message_stream.str(), websocketpp::frame::opcode::text);      
                                    }
                                    con_data_time[ hdl.lock().get() ][UserFlags::LAST_ACT] = boost::posix_time::second_clock::local_time();
                                }                                                                        
                            }
                        }
                        
                        if (action == "kill"){
                            boost::property_tree::ptree pt_child = pt.get_child(action, boost::property_tree::ptree());
                            
                            std::string login    = pt_child.get<std::string>("login");
                            std::string password = pt_child.get<std::string>("password");

                            if (login =="admin" && password == "123"){ // успешная авторизация
                                for (auto it : m_connections) {
                                    websocketpp::lib::error_code ec;
                                    m_server.close( it.lock(), websocketpp::close::status::going_away, "", ec);
                                    break;
                                }
                                                                
                            } else {  
                                websocketpp::lib::error_code ec;
                                m_server.close( hdl, websocketpp::close::status::going_away, "", ec);
                            }                         
                        }   
                    } else {
                        websocketpp::lib::error_code ec;
                        m_server.close( hdl, websocketpp::close::status::going_away, "", ec);                            
                    }

                } catch(std::exception e) {
                    try{
                        websocketpp::lib::error_code ec;
                        //Событие ошибочного запроса:
                        std::stringstream message_stream;
                        message_stream 
                        <<  "{"
                        <<      quoted("event") << ":"
                        <<          "{"
                        <<              quoted("name") << ":" << quoted("ERR_REQUEST")
                        <<         "}"
                        <<  "}";
                        m_server.send(it, message_stream.str(), websocketpp::frame::opcode::text);
                    } catch(websocketpp::exception e2){
                        cout << e2.what() << endl;
                    }                         
                    //m_server.close( hdl, websocketpp::close::status::going_away, "", ec);                        
                }
            }
        }
    } catch(websocketpp::exception e){
        cout << "exception" << endl;
    }
}

void ws_server::on_timer_SQL_pop(websocketpp::lib::error_code const & ec){
    if (mtx.try_lock()){
        for (int c = 0; c < current_settings.database_connections_number; c++) {
            if ( PQisBusy(db_conn[c]) == 0 && !SQLHDL_QUEUE.empty() ){
                boost::thread my_thread( &ws_server::worker_SQL, this, SQLHDL_QUEUE.front().hdl, db_conn[c], SQLHDL_QUEUE.front().sql, SQLHDL_QUEUE.front().ev, SQLHDL_QUEUE.front().send_data );
                SQLHDL_QUEUE.pop();
            }
        }
        mtx.unlock();            
    } 
    set_timer_SQL_pop();
}

void ws_server::worker_SQL( connection_hdl hdl, PGconn *conn, std::string SQL, std::string event_name, bool send_data ){
    PGresult *res = PQexec(conn, SQL.c_str());
    /*
    boost::property_tree::ptree p0;
    boost::property_tree::ptree p1;
    boost::property_tree::ptree rows;
    for (int row = 0; row < PQntuples(res); row++){
        boost::property_tree::ptree p_row;
        for (int col = 0; col < PQnfields(res); col++){
            char *val = PQgetvalue(res, row, col);
            char *nam = PQfname(res, col);
            std::string my_val(val);
            std::string my_nam(nam);
            p_row.put(my_nam, my_val);
        }
        rows.push_back(std::make_pair("", p_row));
    }
    PQclear(res);
    p1.add("name", event_name);
    p1.add_child("data", rows);
    p0.put_child("event", p1);
    */
    
    
    if ( PQstatus(conn) == CONNECTION_OK){
        //std::stringstream my_json;
        //boost::property_tree::write_json(my_json, p0);
        for (;;){
            if ( hdl.lock() && send_data == true){
                //m_server.send(hdl,my_json.str(), websocketpp::frame::opcode::text);
                int row_count = PQntuples(res);
                int col_count = PQnfields(res);
                std::stringstream message_stream;
                message_stream 
                <<  "{"
                <<      quoted("event") << ":"
                <<          "{"
                <<             quoted( "name" )  << ":" << quoted( event_name ) << ","
                <<             quoted( "data" )  << ":"
                <<             "[";
                                    for (int row = 0; row < row_count; row++){
                message_stream
                <<                      "{";
                                        for (int col = 0; col < PQnfields(res); col++){
                                            char *val = PQgetvalue(res, row, col);
                                            char *nam = PQfname(res, col);
                                            std::string my_val(val);
                                            std::string my_nam(nam);                                                            
                message_stream
                <<                          quoted( my_nam )   << ":";

                                            switch (PQftype(res, col)) {
                                                case 21:   // INT2
                                                case 23:   // INT4
                                                case 20:   // INT8
                                                case 1700: // NUMERIC
                                                case 700:  // FLOAT4
                                                case 701:  // FLOAT8
                message_stream  
                <<                              my_val;      
                                                break;
                                            default:
                message_stream
                <<                              quoted( my_val );
                                            }
                                            if (col == col_count - 1) {
                                                break;                                  
                                            } else {
                message_stream                                                            
                <<                          ",";                                                            
                                            } 
                                        }
                message_stream
                <<                      "}";
                                        if (row == row_count - 1 ){
                                            break;
                                        } else {
                message_stream                                                            
                <<                          ",";                                                            
                                        }
                                    }
                message_stream
                <<             "]" 
                <<         "}"
                <<  "}";   
                m_server.send(hdl, message_stream.str() , websocketpp::frame::opcode::text);
                break;
            }
        }            
    } else {
        for (;;){
            if (hdl.lock() && mtx.try_lock()){
                SQLHDL_QUEUE.push( {SQL, hdl, event_name, send_data} );
                PQreset(conn);
                mtx.unlock();        
                break;
            }
        }        
    } 
     
}

void ws_server::load_data(){
    
}


context_ptr ws_server::on_tls_init( connection_hdl hdl ) {
    return ctx;        
}

std::string ws_server::ptime_tostr(boost::posix_time::ptime date_time){    
    std::stringstream stream;
    stream.imbue(std::locale(std::locale::classic(), new boost::posix_time::time_facet("%d.%m.%Y %H:%M:%S")));
    stream << date_time;
    return stream.str();
}
