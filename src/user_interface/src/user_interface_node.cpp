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
#include <macd_sell_signal/sell.h>
#include <macd_sell_signal/macd.h>
#include <ticker_publisher/ticker.h>
#include <sell_signal_filter/history.h>


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
      std::string push_to_print;
      std::getline(ss, item, '_');
      std::getline(ss, item, '_');
      push_to_print = item + "_";
      std::getline(ss, item, '_');
      push_to_print = push_to_print + item + "-->MACD(";
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
  std::cout << "Choose one of the numbers to plot: ";
  std::cin >> which_to_plot;
  if((which_to_plot < 0) || (which_to_plot >= array_topics.size())){
    std::cout << "Invalid number chosen, exiting plotting subroutine\n";
    return;
  }
  //be sure to redirect stdout and stderr so that when program is killed error output is suppressed
  std::cout << "Ctrl+c to quit plot program" << std::endl;
  cmd = "rosrun plot_macd plot_macd_node.py _macd_array_service:=" + array_topics[which_to_plot] + " > /dev/null 2>&1";
  topics.clear();
  call_command(cmd, topics);
  std::cout << std::endl;
  
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
      std::string push_to_print;
      std::getline(ss, item, '_');
      std::getline(ss, item, '_');
      push_to_print = item + "_";
      std::getline(ss, item, '_');
      push_to_print = push_to_print + item + "-->MACD(";
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

void sell_history_routine(std::vector<macd_sell_signal::sell> &sells){
  std::cout << "There are currently " << sells.size() << " sells on record.\n";
  

}

void kill_all_macd_nodes(){
  //get a list of the current node names
  std::string node_list_cmd = "rosnode list";
  std::vector<std::string> node_names;
  call_command(node_list_cmd, node_names);
  //trim newline chars
  for(int i = 0; i < node_names.size(); i++){
    node_names[i].erase(0, node_names[i].find_first_not_of('\n'));
    node_names[i].erase(node_names[i].find_last_not_of('\n')+1); 
    
    std::vector<std::string> throwaway_vector;//we don't care about output from "rosnode kill"
    //is it a macd node?
    if(node_names[i].substr(0,22) == "/macd_sell_signal_node"){
      std::string kill_cmd = "rosnode kill " + node_names[i];
      std::cout << kill_cmd;
      call_command(kill_cmd, throwaway_vector);
      std::cout << "...Success!" << std::endl;
    }
  }
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
  
  //get a list of the current node names
  std::string node_list_cmd = "rosnode list";
  std::vector<std::string> node_names;
  call_command(node_list_cmd, node_names);
  std::string new_node_name;
  int node_counter = 1;
  //trim newline chars
  for(int i = 0; i < node_names.size(); i++){
    node_names[i].erase(0, node_names[i].find_first_not_of('\n'));
    node_names[i].erase(node_names[i].find_last_not_of('\n')+1); 
  }
  
  //find next available node name
  while(1){
    std::string temp = "/macd_sell_signal_node_" + std::to_string(node_counter);
    if((std::find(node_names.begin(), node_names.end(), temp) == node_names.end())){
      new_node_name = "macd_sell_signal_node_" + std::to_string(node_counter);
      break;
    }
    node_counter++;
  }
  
  //construct the rosrun command
  std::string cmd = "rosrun macd_sell_signal macd_sell_signal_node __name:=" + 
    new_node_name + " _trade_pair:=" + pair + 
    " _short:=" + std::to_string(short_ema) +
    " _long:=" + std::to_string(long_ema) + 
    " _sig:=" + std::to_string(sig_ema) +
    " _period:=" + std::to_string(period) + 
    " _num_old_periods:=" + std::to_string(num_old) +
    " _spread_window:=" + std::to_string(spread_window) + 
    " _spread_value:=" + std::to_string(spread_thresh);


  //fork this process and then fork again to ensure grandchild does it's thang
  int pid = fork();
  if(pid == 0){
    setsid();
    int pid_2 = fork();
    if(pid_2 == 0){
      std::system(cmd.c_str());
      exit(0);
    }
    return true;
    //exit(0);
  }else{
    usleep(1000000);
    std::cout << "MACD node started\n";
    //waitpid(pid, NULL, 0);//still don't know why this doesn't work, oh well
  }
  return false;
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

  //Services(clients)
  std::string sell_history_service_name = "sell_history";
  ros::ServiceClient sell_history_client = n.serviceClient<sell_signal_filter::history>(sell_history_service_name);
  sell_signal_filter::history srv;

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
    else if(input == "n"){
      if(new_filter()){
	break;
      }
    }else if(input == "s"){
      if(sell_history_client.call(srv)){
	sell_history_routine(srv.response.history);
      }else{
	ROS_ERROR("Failed to call service /sell_history");
	return 1;
      }
    }else if(input == "killall"){
      kill_all_macd_nodes();
    }
  }
  spinner.stop();
  
}
