struct settings {
    std::string database_port;
    std::string database_hostaddr;
    std::string database_dbname;
    std::string database_user;
    std::string database_password;
    int         database_connections_number;
    std::string database_application_name;
    
    int         general_port;
    std::string general_crt_file;
    std::string general_key_file;
    int         general_maxlifetime;
};