#include "ws_server.h"

ws_server::ws_server(settings my_settings) {
     
    ws_server::key_file    = my_settings.general_key_file;
    ws_server::crt_file    = my_settings.general_crt_file;

    std::ifstream crt_file( ws_server::crt_file );
    std::ifstream key_file( ws_server::key_file );

    if ( !crt_file) {
        std::cout << "ERROR : " << "could not open \"" << ws_server::crt_file << "\" certificate file!" << " exit" << std::endl;
        exit(0);
    }

    if ( !key_file) {
        std::cout << "ERROR : " << "could not open \"" << ws_server::key_file << "\" key file!" << " exit" << std::endl;
        exit(0);
    }

    ws_server::maxlifetime = my_settings.general_maxlifetime;
    
    count_msg = 0;
    m_server.set_reuse_addr(true);
    m_server.set_access_channels(websocketpp::log::alevel::none);
    m_server.init_asio();
    m_server.set_open_handler(    bind( &ws_server::on_open,     this,::_1));
    m_server.set_close_handler(   bind( &ws_server::on_close,    this,::_1));
    m_server.set_message_handler( bind( &ws_server::on_message,  this,::_1,::_2));                                    
    m_server.set_tls_init_handler(bind( &ws_server::on_tls_init, this,::_1));


    std::string connection_string = "hostaddr="  + my_settings.database_hostaddr + 
                                    " port="     + my_settings.database_port + 
                                    " dbname="   + my_settings.database_dbname + 
                                    " user="     + my_settings.database_user + 
                                    " password=" + my_settings.database_password +";";    

    std::string application_name = "SET application_name TO "+ my_settings.database_application_name + ";";
    DBCONNECTIONS_COUNT= my_settings.database_connections_number;
    for (int c = 0; c < DBCONNECTIONS_COUNT; c++){
        db_conn[c] = PQconnectdb( connection_string.c_str() );

        if (PQstatus(db_conn[c]) != CONNECTION_OK){
            std::cout << "ERROR: could not connect to server. Connection string: \"" << connection_string << "\"" << " exit" << std::endl;
            exit(0);   
        }
        PGresult *myres = PQexec(db_conn[c], application_name.c_str());
        PQclear(myres);
    }
    
    thread_SQL_push = new boost::thread( &ws_server::worker_SQL_push, this );
    thread_SQL_pop  = new boost::thread( &ws_server::worker_SQL_pop,  this );
    thread_timer    = new boost::thread( &ws_server::worker_timer,    this );
    thread_data     = new boost::thread( &ws_server::worker_data,     this );
}


ws_server::~ws_server(){
    
    thread_SQL_push->interrupt();
    thread_SQL_pop->interrupt();
    thread_timer->interrupt();
    
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

void ws_server::on_open(connection_hdl hdl) {
    m_connections.insert(hdl);
    con_data_flag[ hdl.lock().get() ]["AUTH"]     = false; //не авторизован
    con_data_flag[ hdl.lock().get() ]["GET_TIME"] = false; //не получает время
    con_data_time[ hdl.lock().get() ]["LAST_ACT"] = boost::posix_time::second_clock::local_time(); //время последнего действия
    con_data_value[ hdl.lock().get()]["IP_ADDR"]  =  m_server.get_con_from_hdl(hdl)->get_remote_endpoint(); //IP адрес клиента
    
    std::cout << "New connection from IP ADDR: "<< con_data_value[ hdl.lock().get() ]["IP_ADDR"] << std::endl;
}

void ws_server::on_close(connection_hdl hdl) {
    std::cout << "on_close()" << std::endl;    
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
    m_connections.erase(hdl);    
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
                        
                    if (con_data_flag[ hdl.lock().get() ]["AUTH"] == false && action == "auth"){ 
                        if (action == "auth"){
                            boost::property_tree::ptree pt_child = pt.get_child(action, boost::property_tree::ptree());
                            
                            std::string login    = pt_child.get<std::string>("login");
                            std::string password = pt_child.get<std::string>("password");

                            if (login =="user1" && password == "123"){ // успешная авторизация
                                con_data_flag[ hdl.lock().get() ]["AUTH"]     = true;
                                con_data_value[ hdl.lock().get()]["LOGIN"]    = login;
                                con_data_time[ hdl.lock().get() ]["LAST_ACT"] = boost::posix_time::second_clock::local_time();
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
                    } else if (con_data_flag[ hdl.lock().get() ]["AUTH"] == true && action != "auth"){
                        // тестирование SQL
                        if (action == "sql"){
                            for (;;){
                                if (mtx.try_lock()){
                                    SQL_QUEUE.push( hdl );                    
                                    // событие успешного входа
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
                            con_data_time[ hdl.lock().get() ]["LAST_ACT"] = boost::posix_time::second_clock::local_time();
                            int get = pt_child.get<int>("get");
                            if (get == 1) con_data_flag[ hdl.lock().get() ]["GET_TIME"] = true;
                            if (get == 0) con_data_flag[ hdl.lock().get() ]["GET_TIME"] = false;
                        }
                        
                        //Подписка на получение графика
                        if (action == "graph"){
                            
                            boost::property_tree::ptree pt_child = pt.get_child(action, boost::property_tree::ptree());
                            con_data_time[ hdl.lock().get() ]["LAST_ACT"] = boost::posix_time::second_clock::local_time();
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

                                con_data_flag[ hdl.lock().get() ]["GRAPH"] = true;
                            } else {
                                con_data_flag[ hdl.lock().get() ]["GRAPH"] = false;    
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
                                std::string user = con_data_value[ hdl.lock().get() ]["LOGIN"];
                                    
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
                                    con_data_time[ hdl.lock().get() ]["LAST_ACT"] = boost::posix_time::second_clock::local_time();
                                }                                                                        
                            }
                        }
                    } else {
                        websocketpp::lib::error_code ec;
                        m_server.close( hdl, websocketpp::close::status::going_away, "", ec);                            
                    }

                } catch(std::exception e) {
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
                    //m_server.close( hdl, websocketpp::close::status::going_away, "", ec);
                }
            }
        }
    } catch(websocketpp::exception e){
        cout << "exception" << endl;
    }
}

void ws_server::run(uint16_t port) {
    try{
        m_server.listen(port);
        m_server.start_accept();
        m_server.run(); 
    } catch (websocketpp::exception e){
        cout << e.what() << endl;
    }
}

void ws_server::worker_SQL_push(){
    for(;;){
        if (mtx.try_lock()){
            if ( PG_QUEUE.empty() ){
                for (int c = 0; c < DBCONNECTIONS_COUNT; c++){
                    if ( PQisBusy(db_conn[c]) == 0 ){
                        PG_QUEUE.push(db_conn[c]);
                    }
                }
            }            
            mtx.unlock();
        } else {
            boost::this_thread::sleep(boost::posix_time::milliseconds( 5 ));
        }
        boost::this_thread::sleep(boost::posix_time::milliseconds( 10 ));
    }
}

void ws_server::worker_SQL_pop(){
    for(;;){
        if (mtx.try_lock()){
            if ( !SQL_QUEUE.empty() && !PG_QUEUE.empty() ){
                connection_hdl my_hdl = SQL_QUEUE.front();
                PGconn *mycon = PG_QUEUE.front();
                if ( my_hdl.lock().get() ){
                    cout << "X1" << endl;
                    boost::thread my_thread3( &ws_server::worker_SQL, this, my_hdl, mycon);
                }
                cout << "POP" << endl;
                PG_QUEUE.pop();
                SQL_QUEUE.pop();
            }            
            mtx.unlock();
        } else {
            boost::this_thread::sleep(boost::posix_time::milliseconds( 10 ));
        }
        boost::this_thread::sleep(boost::posix_time::milliseconds( 10 ));
    }
}

void ws_server::worker_SQL( connection_hdl hdl, PGconn *conn ){
    cout << "POP , QUEUE SIZE: " << SQL_QUEUE.size() << endl;
    std::string SQL = "SELECT pg_sleep(10); SELECT * FROM t1 ORDER BY random() LIMIT 1;";
    //string SQL = "SELECT * FROM t1 ORDER BY random() LIMIT 1;";
    PGresult *res = PQexec(conn, SQL.c_str());
    std::string myRes;

    std::stringstream my_json;
    boost::property_tree::ptree p;
    boost::property_tree::ptree rows;


    for (int row = 0; row < PQntuples(res); row++){
        boost::property_tree::ptree r;
        for (int col = 0; col < PQnfields(res); col++){
            char *val = PQgetvalue(res, row, col);
            char *nam = PQfname(res, col);
            std::string my_val(val);
            std::string my_nam(nam);
            r.put(my_nam, my_val);
        }
        rows.push_back(std::make_pair("", r));
    }

    PQclear(res);

    p.put("event", "SQL_TEST");
    p.add_child("rows", rows);

    boost::property_tree::write_json(my_json, p);
    for (;;){
        if (hdl.lock()){
            m_server.send(hdl,my_json.str(), websocketpp::frame::opcode::text);
            break;
        }
    }
}

void ws_server::worker_timer() {
    
    boost::posix_time::ptime now_old = boost::posix_time::second_clock::local_time();
    for(;;){
        boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
        if (now_old != now){ // наступила новая секундв
            now_old = now;
            int i = 0;

            //Перебор соединений
            for (auto it : m_connections) {
                //ТЕКУЩЕЕ ВРЕМЯ:                
                if( con_data_flag[ it.lock().get() ]["GET_TIME"] == true ){
                    try{
                        std::stringstream message_stream;
                        message_stream 
                        <<  "{"
                        <<      quoted("event") << ":"
                        <<          "{"
                        <<             quoted("name") << ":" << quoted("TIME") << ","
                        <<             quoted("time") << ":" << quoted(ptime_tostr(now)) 
                        <<         "}"
                        <<  "}";                        
                        m_server.send(it, message_stream.str()  , websocketpp::frame::opcode::text);
                    } catch(websocketpp::exception e){
                        cout << e.what() << endl;
                    }                
                }
                
                //Отсылка графика
                if( con_data_flag[ it.lock().get() ]["GRAPH"] == true ){
                    try{
                        //берем последний элемент из server_data
                        
                        //int i = 0;
                        //for (server_data_reverse_it graph_it = server_data.rbegin(); graph_it != server_data.rend(); ++graph_it){
                            
                            server_data_reverse_it graph_it = server_data.rbegin();
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
                             
  
                            //if ( i > 0){
                            //    break;
                            //}
                            //i++;
                        //}                            
                        //server_data_reverse_it graph_it = server_data.rbegin();
                        

                        
                        
                    } catch(websocketpp::exception e){
                        cout << e.what() << endl;
                    }                
                }                
                //РЕЗУЛЬТАТЫ ПО КУПЛЕННЫМ ОПЦИОНАМ
                //перебор сделанных покупок
                for (auto it_buy = buy_set.begin(); it_buy != buy_set.end(); ++it_buy){
                    std::cout   << it_buy->timeBuy  << "\t" << it_buy->value1   << "\t" << it_buy->option   << "\t" << it_buy->quantity << "\t" << it_buy->user << "\t" << " IP: " << con_data_value[ it.lock().get()]["IP_ADDR"] << std::endl;                    
                    if ( it_buy->timeExe <= now ){
                        if( con_data_value[ it.lock().get() ]["LOGIN"] == it_buy->user ){
                            std::stringstream message_stream;
                            message_stream 
                            <<  "{"
                            <<      quoted("event") << ":"
                            <<          "{"
                            <<             quoted( "name"     )  << ":" << quoted("RES") << ","
                            <<             quoted( "timeBuy"  )  << ":" << quoted(ptime_tostr(it_buy->timeBuy)) << ","
                            <<             quoted( "timeExe"  )  << ":" << quoted(ptime_tostr(it_buy->timeExe)) << "," 
                            <<             quoted( "option"   )  << ":" << it_buy->option  << "," 
                            <<             quoted( "quantity" )  << ":" << it_buy->quantity 
                            <<         "}"
                            <<  "}";                               
                            m_server.send(it, message_stream.str(), websocketpp::frame::opcode::text);    
                        } 
                        buy_set.erase(it_buy); 
                    }                   
                }
                //разлогин после maxlife бездействия
                if ( con_data_time[ it.lock().get() ]["LAST_ACT"] + boost::posix_time::seconds( ws_server::maxlifetime ) <= now  ) {
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
                }
            }
            
        }
        boost::this_thread::sleep(boost::posix_time::milliseconds( 10 ));
    }
}

void ws_server::worker_data(){

    managed_shared_memory segment(open_only,"ws_server_data");
    TimeValues_set *es = segment.find<TimeValues_set>("TimeValues").first;
    for (auto it = es->begin(); it != es->end(); ++it){
        server_data[it->time][107] = it->value1;    
    }

    boost::posix_time::ptime now_old = boost::posix_time::second_clock::local_time();
    for(;;){
        boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
        if (now_old != now){ // наступила новая секундв
            now_old = now;

            managed_shared_memory segment(open_only,"ws_server_data");
            TimeValues_set *es = segment.find<TimeValues_set>("TimeValues").first;

            const boost::posix_time::ptime t = boost::posix_time::second_clock::local_time() - boost::posix_time::seconds(5);
            auto it_find = es->get<index_by_t_id_ba>().find(std::make_tuple(t, 1));
           
            for (auto it = it_find; it != es->end(); ++it){
                server_data[it->time][107] = it->value1;    
            }

            if ( server_data.size() > 3600 ){
                server_data.erase( server_data.begin()->first );
            }

        }
        boost::this_thread::sleep(boost::posix_time::milliseconds( 10 ));
    }

}

context_ptr ws_server::on_tls_init( connection_hdl hdl ) {
    std::cout << "on_tls_init" << std::endl;
    context_ptr ctx(new boost::asio::ssl::context(boost::asio::ssl::context::tlsv1));
    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds | boost::asio::ssl::context::no_sslv2 | boost::asio::ssl::context::no_sslv3 | boost::asio::ssl::context::no_tlsv1 | boost::asio::ssl::context::single_dh_use);        

        ctx->use_certificate_chain_file( ws_server::crt_file );
        ctx->use_private_key_file( ws_server::key_file , boost::asio::ssl::context::pem);                    
        
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return ctx;
}

std::string ws_server::ptime_tostr(boost::posix_time::ptime date_time){    
    std::stringstream stream;
    stream.imbue(std::locale(std::locale::classic(), new boost::posix_time::time_facet("%d.%m.%Y %H:%M:%S")));
    stream << date_time;
    return stream.str();
}
