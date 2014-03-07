#include <ros/ros.h>
#include <ros/time.h>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>

void print_vector(std::vector<std::string> &print_me){
  std::copy(print_me.begin(),
	    print_me.end(),
	    std::ostream_iterator<std::string>(std::cout,"\n"));
}

void call_command(std::string &cmd, std::vector<std::string> &lines){
  FILE * cmd_output = popen(cmd.c_str(), "r");
  
  //put this crap in a vector
  char line[256];
  while( fgets (line, sizeof line, cmd_output)){
    lines.push_back(line);
  }
  pclose(cmd_output);//shit gets pretty walking dead up in here without this line
}



void plot_filter(){
  std::string cmd = "rostopic list";
  std::vector<std::string> topics;
  std::vector<std::string> print_vec;
  std::vector<std::string> array_topics;

  call_command(cmd, topics);
  for(int i = 0; i <topics.size(); i++){
    //erase this fucker
    topics[i].erase(0, topics[i].find_first_not_of('\n'));
    topics[i].erase(topics[i].find_last_not_of('\n')+1); 
  
    if((topics[i].substr(0,5) == "/macd") &&
       (topics[i].substr(topics[i].size()-4, topics[i].size()) == "macd")){
      array_topics.push_back(topics[i]+"_array");
      std::stringstream ss(topics[i]);
      std::string item;
      std::string push_to_print("MACD(");
      std::getline(ss, item, '_');
      std::getline(ss, item, '_');
      push_to_print = push_to_print + item + ",";
      std::getline(ss, item, '_');
      push_to_print = push_to_print + item + ",";
      std::getline(ss, item, '_');
      push_to_print = push_to_print + item + ") ";
      std::getline(ss, item, '/');
      push_to_print = push_to_print + item;
      print_vec.push_back(push_to_print);
    }
  }
  
  if(print_vec.size() == 0){
    std::cout << "Sorry, there's no MACD topics available to plot yet\n";
    return;
  }
  std::cout << "Here's a list of the current MACD topics and an associated number:" << std::endl;

  for(int i = 0; i < print_vec.size(); i++){
    print_vec[i] = std::to_string(i) + " - " + print_vec[i];
  }
  print_vector(print_vec);
  int which_to_plot = -1;
  while((which_to_plot < 0) || (which_to_plot >= array_topics.size())){
    std::cout << "Choose one of the numbers to plot: ";
    std::cin >> which_to_plot;
  }
  std::cout << "Ctrl+c to quit plot program" << std::endl;
  cmd = "rosrun plot_macd plot_macd_node.py _macd_array_service:=" + array_topics[which_to_plot];
  topics.clear();
  call_command(cmd, topics);
  
}

void list_filters(){
  std::string cmd = "rostopic list";
  std::vector<std::string> topics;
  std::vector<std::string> print_vec;

  call_command(cmd, topics);
  for(int i = 0; i <topics.size(); i++){
    //erase this fucker
    topics[i].erase(0, topics[i].find_first_not_of('\n'));
    topics[i].erase(topics[i].find_last_not_of('\n')+1); 
  
    if((topics[i].substr(0,5) == "/macd") &&
       (topics[i].substr(topics[i].size()-4, topics[i].size()) == "macd")){
      std::stringstream ss(topics[i]);
      std::string item;
      std::string push_to_print("MACD(");
      std::getline(ss, item, '_');
      std::getline(ss, item, '_');
      push_to_print = push_to_print + item + ",";
      std::getline(ss, item, '_');
      push_to_print = push_to_print + item + ",";
      std::getline(ss, item, '_');
      push_to_print = push_to_print + item + ") ";
      std::getline(ss, item, '/');
      push_to_print = push_to_print + item;
      print_vec.push_back(push_to_print);
    }
  }
  print_vector(print_vec);
}

bool new_filter(){
  std::string pair;
  int short_ema, long_ema, sig_ema, period, num_old, spread_window;
  double spread_thresh;
  char yes_no;
  
  while(yes_no != 'y'){
    std::cout << "Enter the params separated by spaces,\n";
    std::cout << "example: btc_usd 11 25 8 10 15 40 1.25\n";
    std::cout << "\t>";
    
    std::cin >> pair;
    std::cin >> short_ema;
    std::cin >> long_ema;
    std::cin >> sig_ema;
    std::cin >> period;
    std::cin >> num_old;
    std::cin >> spread_window;
    std::cin >> spread_thresh;
    
    std::cout << "The MACD you've entered has the following parameters:\n";
    std::cout << "\tPair: " << pair;
    std::cout << "\tMACD(" << short_ema << "," << long_ema << "," << sig_ema << ")x" << period << std::endl;
    std::cout << "\tPlot last " << num_old << " periods\n";
    std::cout << "\tSpread window/thresh: " << spread_window << "/" << spread_thresh << std::endl;

    std::cout << "\nIs this correct(y/n): ";
    std::cin >> yes_no;
  }

  //start a new filter and return
  std::string cmd = "rosrun macd_sell_signal macd_sell_signal_node _trade_pair:=" + pair + 
    " _short:=" + std::to_string(short_ema) +
    " _long:=" + std::to_string(long_ema) + 
    " _sig:=" + std::to_string(sig_ema) +
    " _period:=" + std::to_string(period) + 
    " _num_old_periods:=" + std::to_string(num_old) +
    " _spread_window:=" + std::to_string(spread_window) + 
    " _spread_value:=" + std::to_string(spread_thresh);
  //std::cout << "Command to be issued: " << cmd << std::endl;


  //TODO - set the name of the node?
  int pid = fork();
  if(pid == 0){
    setsid();
    //int pid_2 = fork();
    //if(pid_2 == 0){
      std::system(cmd.c_str());
      //exit(0);
      //}
    exit(0);
  }else{
    usleep(1000000);
    std::cout << "MACD node started\n";
    waitpid(pid, NULL, 0);
  }
}

void print_help_screen(){
  std::cout << "Available commands:" << std::endl;
  std::cout << "\th - help - display this screen" << std::endl;
  std::cout << "\tq - quit - exit user interface" << std::endl;
  std::cout << "\tl - list current filters and associated params" << std::endl;
  std::cout << "\tp - plot one of the macd filters" << std::endl;
  std::cout << "\tn - new MACD filter" << std::endl;
}

int main(int argc, char** argv){
  ros::init(argc, argv, "user_interface_node");
  ros::NodeHandle n;
  ros::NodeHandle nh("~");


  ros::Rate rate(100);
  //use a seperate thread for callbacks, might do something with current thread later
  ros::AsyncSpinner spinner(1);
  spinner.start();
  std::string input;
  while(1){//good programming pracetice, i know
    std::cout << "Please enter a command (h for help): ";
    std::cin >> input;
    if(input == "h"){print_help_screen();}
    else if(input == "q"){break;}
    else if(input == "l"){list_filters();}
    else if(input == "p"){plot_filter();}
    else if(input == "n"){new_filter();}
  }
  spinner.stop();
  
}
