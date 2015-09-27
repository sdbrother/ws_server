struct settings {
    //DATABASE
    std::string   database_port;
    std::string   database_hostaddr;
    std::string   database_dbname;
    std::string   database_user;
    std::string   database_password;
    unsigned int  database_connections_number;
    std::string   database_application_name;
    //GENERAL
    unsigned int  general_port;
    std::string   general_crt_file;
    std::string   general_key_file;
    unsigned long general_maxlifetime;
    unsigned int  general_num_threads;
};