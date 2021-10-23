#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <sstream>

#include <boost/algorithm/string.hpp>

using Args = std::deque<std::string>;
using Func = std::function<void(Args)>;
using Command = std::tuple<std::string, std::string, Func>;

class Cli{
    public:
        /**
         * @brief Construct a new Cli object and adds "help, h" and "exit" to _commands.
         */
        Cli();
        /**
         * @brief Adds a command to _commands.
         * 
         * Example: add_to_commands_list("h", "Help", std::bind(&Cli::print_help, &cli, std::placeholders::_1));
         * 
         * @param names string, list of names that execute the command
         * @param par string, description to be displayed when help command is executed
         * @param func function to be executed (for member functions see example above)
         */
        void add_to_commands_list(std::string names, std::string desc, Func func);

        /**
         * @brief Loops the iterate function. Stops when _exit is set to true.
         */
        void run();
        /**
         * @brief Asks user for input and runs matching method from _commands.
         */
        void iterate();

        bool get_exit() { return _exit; }
        Command& get_command(int i) { return _commands[i]; }

        // Functions for commands
        void default_help(Args args);
        void exit_program(Args args);
        void print_help(Args args);
        void syn(Args args);

    private:
        bool _exit = false; ///< When set to true the run method will stop.
        std::vector<Command> _commands; ///< Set of commands that can be accessed in the cli.

        void print(std::string str) { std::cout << "  " << str << "\n"; }
        Func get_function(std::string input);
};