#include "Configurator.hpp"
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

//-------------------------------------------------------------------------------
//------------------------- Constructor/ Destructor -----------------------------
//-------------------------------------------------------------------------------

Configurator::Configurator(){}

Configurator* Configurator::_instance = nullptr;

Configurator* Configurator::instance(){  // Speicherbereinigung
    static CGuard g;
    if (!_instance){
        _instance = new Configurator ();
    }
    return _instance;
}

Configurator::~Configurator(){}

//-------------------------------------------------------------------------------
//-------------------------------- Read Config ----------------------------------
//-------------------------------------------------------------------------------

void Configurator::read_config(std::string path,
                               std::string default_path){
    // Read Standard Config
    std::ifstream config_file(path);
    if(config_file.is_open()){
        config_file >> _config;
        LOG_INFO << "Config loaded Succesfully" << LOG_END;
    }
    else {
        LOG_INFO << "No Config-File found at " + path << LOG_END;
        throw std::runtime_error("No Config-File found at " + path);
    }

    // Read Default Config
    std::ifstream default_config_file(default_path);
    if(default_config_file.is_open()){
        default_config_file >> _default_config;
        LOG_INFO << "Default Config loaded Succesfully" << LOG_END;
    }
    else {
        LOG_ERROR << "No Config-File found at default path " + default_path << LOG_END; 
        throw std::runtime_error("No Config-File found at deafult path " + default_path);
    }
}

//-------------------------------------------------------------------------------
//-------------------------------- Get-Methods ----------------------------------
//-------------------------------------------------------------------------------

bool Configurator::entry_exists(const std::string att_name) {
    bool in_default_config = _default_config.find(att_name) != _default_config.end();
    bool in_standard_config = _config.find(att_name) != _config.end();
    return in_default_config || in_standard_config;
}

std::string Configurator::get_config_as_string(const std::string& att_name, bool default_value) {
    return Configurator::get_value_from_config<std::string>(att_name, default_value);
}

unsigned int Configurator::get_config_as_unsigned_int(const std::string& att_name, bool default_value) {
    return Configurator::get_value_from_config<int>(att_name, default_value);
}

bool Configurator::get_config_as_bool(const std::string& att_name, bool default_value) {
    return Configurator::get_value_from_config<bool>(att_name, default_value);
}

float Configurator::get_config_as_float(const std::string& att_name, bool default_value) {
    return Configurator::get_value_from_config<float>(att_name, default_value);
}

double Configurator::get_config_as_double(const std::string& att_name, bool default_value) {
    return Configurator::get_value_from_config<double>(att_name, default_value);
}

template <typename T> 
T Configurator::get_value_from_config(const std::string& att_name, bool default_value) {
    bool search_in_default_config = !(_config.find(att_name) != _config.end()) || default_value;
    if (!search_in_default_config) { return _config[att_name]; }
    return _default_config[att_name];
}

//TODO Mehrdimensionale Attribute suchen
