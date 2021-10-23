#include "Cli.hpp"

Cli::Cli() {
  add_to_commands_list("help, h", "Help Screen", std::bind(&Cli::print_help, &*this, std::placeholders::_1));
  add_to_commands_list("exit", "Exit Program", std::bind(&Cli::exit_program, &*this, std::placeholders::_1));
}

void Cli::add_to_commands_list(std::string names, std::string desc, Func func) {
  Command cmd = std::make_tuple(names, desc, func);
  _commands.push_back(cmd);
}

//--------------------------------------------------------------------------------------------------
//------------------------------------------- Run --------------------------------------------------
//--------------------------------------------------------------------------------------------------

void Cli::run() {
  while(!_exit) { iterate(); }
}

void Cli::iterate() {
  // Ask for input
  std::cout << "\033[1;36mAegis: \033[0m";
  std::string input;
  std::cin >> input;

  // Get parameters
  std::deque<std::string> parameters;
  std::string line;
  std::getline(std::cin, line);
  std::istringstream iss(line);
  std::string arg;
  while (iss >> arg) {    
    parameters.push_back(arg);
  }

  // Call matching function
  Func todo = get_function(input);
  todo(parameters);
}

Func Cli::get_function(std::string input) {
  for (Command cmd : _commands) {
    // Split command names from _commands
    std::vector<std::string> names;
    boost::split(names, std::get<0>(cmd), boost::is_any_of(", "));

    // Check if input matches any of the names
    for (std::string name : names) {
      if (name == input) {
        return std::get<2>(cmd);
      }
    }
  }

  // Default: Basic help command
  Cli cli;
  return std::bind(&Cli::default_help, cli, std::placeholders::_1);
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------- Local Methods --------------------------------------------
//--------------------------------------------------------------------------------------------------

void Cli::default_help(Args args) {
  print("Try 'help'");
}

void Cli::exit_program(Args args) {
  _exit = true;
}

void Cli::print_help(Args args) {
  print("Commands:\n");
  if (_commands.size() == 0) {
    print("Expty.");
    return;
  }

  // determine max length of first elements
  int max_size = 0;
  for (auto& c : _commands) {
    int len = std::get<0>(c).length();
    max_size = (len > max_size) ? len : max_size;
  }

  // print every available command
  for (auto& c : _commands) {
    int spaces_to_add = max_size - std::get<0>(c).length();
    if (spaces_to_add < 0 || spaces_to_add > 50) { throw "Internal fatal error."; }

    
    std::string command_text;
    command_text += std::get<0>(c);
    for (int i = 0; i < spaces_to_add; i++) { command_text += " "; }
    command_text += "  " + std::get<1>(c);
    print(command_text);
  }
}

void Cli::syn(Args args) {
  if (!args.empty()) {
    print("SYN " + args[0]);
  } else {
    print("SYN");
  }
}
