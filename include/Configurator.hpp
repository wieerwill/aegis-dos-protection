#pragma once

#include <boost/log/trivial.hpp>

#include "Definitions.hpp"
#include "iostream"
#include <fstream>
#include <json.hpp>
#include <stdexcept>

using json = nlohmann::json;

class Configurator{
    public:
        /**
         * @brief Checks if attribute exists in either the normal or the default config file
         * 
         * @param att_name Name of attribute that is searched for
         * @return true when attribute was found
         * @return false when attribute wasn't found
         */
        bool entry_exists(const std::string att_name);

        //-----------------------------------------------------------------------------------
        //------------------------------- Get-Methods ---------------------------------------
        //-----------------------------------------------------------------------------------
        
        /**
         * @brief Get value from config as string object
         * 
         * @param att_name Name of attribute that is searched for
         * @param default_value When set to true the method searches in the default config (default: false)
         * @return std::string value
         */
        std::string get_config_as_string(const std::string& att_name,
                                         bool default_value = false);
        /**
         * @brief Get value from config as unsigned integer
         * 
         * @param att_name Name of attribute that is searched for
         * @param default_value When set to true the method searches in the default config (default: false)
         * @return std::string value
         */
        unsigned int get_config_as_unsigned_int(const std::string& att_name,
                                                bool default_value = false);
        /**
         * @brief Get value from config as boolean
         * 
         * @param att_name Name of attribute that is searched for
         * @param default_value When set to true the method searches in the default config (default: false)
         * @return std::string value
         */
        bool get_config_as_bool(const std::string& att_name,
                                bool default_value = false);
        /**
         * @brief Get value from config as float
         * 
         * @param att_name Name of attribute that is searched for
         * @param default_value When set to true the method searches in the default config (default: false)
         * @return std::string value
         */
        float get_config_as_float(const std::string& att_name,
                                  bool default_value = false);
        /**
         * @brief Get value from config as double
         * 
         * @param att_name Name of attribute that is searched for
         * @param default_value When set to true the method searches in the default config (default: false)
         * @return std::string value
         */
        double get_config_as_double(const std::string& att_name,
                                    bool default_value = false);
        /**
         * @brief Reads config and default config from json files
         * 
         * @param path Path to standard config (which can be modified by the user)
         * @param default_path Path to default config (default: "../default_config.json")
         */
        void read_config(std::string path,
                         std::string default_path = "../default_config.json");
        
        /**
         * @brief The Configurator is implemented as a singleton. This method returns a poiter to the Configurator object
         * 
         * @return Configurator* poiter to singleton Configurator
         */
        static Configurator* instance();
        
    private:
        json _config; ///< standard config (can be modified by the user)
        json _default_config; ///< default config
        template <typename T> T get_value_from_config(const std::string& att_name, bool default_value);
        static Configurator* _instance; ///< singleton Configurator
        Configurator(); 
        Configurator(const Configurator&);
        ~Configurator();       
        class CGuard{
            public:
                ~CGuard(){
                    if( NULL != Configurator::_instance ){
                        delete Configurator::_instance;
                        Configurator::_instance = nullptr;
                    }
                }
        };

};
