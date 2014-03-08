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
#include <btce_health/server_time.h>
#include <time.h>
#include <stdio.h>


ros::ServiceClient server_time_client_;

void print_vector(std::vector<std::string> &print_me){
  std::copy(print_me.begin(),
	    print_me.end(),
	    std::ostream_iterator<std::string>(std::cout,"\n"));
}

void call_command(std::string &cmd, std::vector<std::string> &lines){
  FILE * cmd_output = popen(cmd.c_str(), "r");
  
  //store each line as new element of vector
  char line[256];
  while( fgets (line, sizeof line, cmd_output)){
    lines.push_back(line);
  }
  //comment out to trigger walking dead
  pclose(cmd_output);
}



void plot_filter(){
  std::string cmd = "rostopic list";
  std::vector<std::string> topics;
  std::vector<std::string> print_vec;
  std::vector<std::string> array_topics;

  call_command(cmd, topics);
  for(int i = 0; i <topics.size(); i++){
    //erase newline chars
    topics[i].erase(std::remove(topics[i].begin(), topics[i].end(), '\n'), topics[i].end());

  
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


//convert string in the form "yyyy-mm-dd hh:mm:ss" to a std::tm struct
std::tm create_tm_struct(std::string server_time){
  std::stringstream ss(server_time);
  std::string item;
  std::tm tm_ret;

  std::getline(ss, item, '-');
  tm_ret.tm_year = atoi(item.c_str())- 1900;//year since 1900
  std::getline(ss, item, '-');
  tm_ret.tm_mon = atoi(item.c_str()) - 1;//since months in struct are 0-11 not 1-12
  std::getline(ss, item, ' ');
  tm_ret.tm_mday = atoi(item.c_str());
  std::getline(ss, item, ':');
  tm_ret.tm_hour = atoi(item.c_str());
  std::getline(ss, item, ':');
  tm_ret.tm_min = atoi(item.c_str());
  std::getline(ss, item, '\n');
  tm_ret.tm_sec = atoi(item.c_str());
  tm_ret.tm_isdst = -1; // without this, all fails
  
  return tm_ret;
}

void sell_history_routine(std::vector<macd_sell_signal::sell> &sells){
  if(sells.size() == 0){
    return;
  }
  std::string server_time;
  //get server time
  btce_health::server_time srv;
  if(server_time_client_.call(srv)){
    server_time = srv.response.server_time;
  }else{
    ROS_ERROR("Failed to call service /server_time");
    return;
  }
  
  //construct std::tm struct from server_time string
  std::tm server_time_tm = create_tm_struct(server_time);
  time_t server_time_time_t = mktime(&server_time_tm);
  std::cout << "There are currently " << sells.size() << " sells on record at time " << server_time << ":" << std::endl;;

  //First loop through the sells and gather statistics about them
  int number_completed = 0;
  //std::vector<std::string> 
  for(int i = 0; i < sells.size(); i++){
    macd_sell_signal::sell tmp = sells[i];//so i don't keep having to type []
    std::cout << "Sell number " << i << "    :    ";

    //find time of sell
    std::tm sell_time_tm = create_tm_struct(tmp.current.tick.server_time);
    time_t sell_time_time_t = mktime(&sell_time_tm);

    //find difference between time of sell and now
    double diff_in_seconds = difftime(server_time_time_t, sell_time_time_t);
    
    int hours_ago = ((int)diff_in_seconds)/((int)3600);
    int minutes_ago = (((int)diff_in_seconds)%((int)3600))/60;
    int seconds_ago = ((int)diff_in_seconds)%60;
    
    //construct string representation of time elapsed
    std::string delta_time = std::to_string(hours_ago) + ":" + std::to_string(minutes_ago) + ":" + std::to_string(seconds_ago);
    
    //output some info on sell
    std::cout << tmp.current.tick.server_time << " (" << delta_time << " ago)" << std::endl;
  }
}

//man, this is going to be hard/annoying
void make_printable_vector(std::vector<std::string> &macd_nodes,std::vector<std::string> &macd_nodes_info){
  //get a list of the current node names
  std::string node_list_cmd = "rosnode list";
  std::vector<std::string> node_names;
  call_command(node_list_cmd, node_names);
  
  //trim newline chars and save each topic name
  for(int i = 0; i < node_names.size(); i++){
    node_names[i].erase(std::remove(node_names[i].begin(),
				    node_names[i].end(), '\n'),
			node_names[i].end());
    //is it a macd node?
    if(node_names[i].find("/macd_sell_signal_node") != std::string::npos){
      macd_nodes.push_back(node_names[i]);
    }
  }
  //get info about each node for user
  for(int i = 0; i < macd_nodes.size(); i++){
    std::string node_info_cmd = "rosnode info " + macd_nodes[i];
    std::vector<std::string> node_info;
    call_command(node_info_cmd, node_info);
    for(int j = 0; j < node_info.size(); j++){
      //trim newline chars
      node_info[j].erase(std::remove(node_info[j].begin(), node_info[j].end(), '\n'), node_info[j].end());

      if(node_info[j].find("[macd_sell_signal/macd]") != std::string::npos){
	std::stringstream ss(node_info[j].substr(3,node_info[j].size()-27));
	std::string item;
	std::string push_to_print;
	std::getline(ss, item, '_');
	std::getline(ss, item, '_');
	push_to_print = "Pair:  " + item + "_";
	std::getline(ss, item, '_');
	push_to_print = push_to_print + item + "    \n    MACD(";
	std::getline(ss, item, '_');
	push_to_print = push_to_print + item + ",";
	std::getline(ss, item, '_');
	push_to_print = push_to_print + item + ",";
	std::getline(ss, item, '_');
	push_to_print = push_to_print + item + ")";
	std::getline(ss, item, '/');
	push_to_print = push_to_print + item;
	macd_nodes_info.push_back(macd_nodes[i] + "\n    " + (push_to_print));
	break;
      }
    }
  }
}
  

void kill_individual_nodes(){
  std::vector<std::string> macd_nodes;
  std::vector<std::string> macd_nodes_info;
  make_printable_vector(macd_nodes, macd_nodes_info);
  //first, print list of vectors to screen for user
  for(int i = 0; i < macd_nodes_info.size(); i++){
    std::cout << "Node " << i << ": " << macd_nodes_info[i] << std::endl;
  }
  std::cout << "Enter which nodes you'd like to kill, separated by spaces, enter -1 when finished\n";
  std::cout << "example: 3 6 13 -1" << std::endl;
  std::cout << "\t>";
  int kill_this_node = -2;
  std::vector<int> kill_nodes;
  while(kill_this_node != -1){
    std::cin >> kill_this_node;
    kill_nodes.push_back(kill_this_node);
  }
  std::vector<std::string> throwaway_vector;
  for(int i = 0; i < kill_nodes.size()-1; i++){
    std::string kill_cmd = "rosnode kill " + macd_nodes[kill_nodes[i]];
    call_command(kill_cmd, throwaway_vector);
  }
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
    //node_names[i].erase(0, node_names[i].find_first_not_of('\n'));
    //node_names[i].erase(node_names[i].find_last_not_of('\n')+1); 
    node_names[i].erase(std::remove(node_names[i].begin(), node_names[i].end(), '\n'), node_names[i].end());
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
  std::cout << "\th(elp) - help - display this screen" << std::endl;
  std::cout << "\tq(uit) - quit - exit user interface" << std::endl;
  std::cout << "\tl(ist) - list current filters and associated params" << std::endl;
  std::cout << "\tp(lot) - plot one of the macd filters" << std::endl;
  std::cout << "\tn(ew) - new MACD filter" << std::endl;
  std::cout << "\tkillall - kill all MACD nodes" << std::endl;
  std::cout << "\tk(ill) - kill individual nodes" << std::endl;
  std::cout << "\ts(ells) - list of current sells" << std::endl;
}

int main(int argc, char** argv){
  ros::init(argc, argv, "user_interface_node");
  ros::NodeHandle n;
  ros::NodeHandle nh("~");

  //Services(clients)
  std::string sell_history_service_name = "sell_history";
  ros::ServiceClient sell_history_client = n.serviceClient<sell_signal_filter::history>(sell_history_service_name);
  sell_signal_filter::history srv;
  server_time_client_ = n.serviceClient<btce_health::server_time>("server_time");

  ros::Rate rate(100);
  //use a seperate thread for callbacks, might do something with current thread later
  ros::AsyncSpinner spinner(1);
  spinner.start();
  std::string input;
  while(1){//good programming pracetice, i know
    std::cout << "Please enter a command (h for help): ";
    std::cin >> input;
    if(input == "h" || input == "help"){print_help_screen();}
    else if(input == "q" || input == "quit"){break;}
    else if(input == "l" || input == "list"){
      std::vector<std::string> macd_nodes;
      std::vector<std::string> macd_nodes_info;
      make_printable_vector(macd_nodes, macd_nodes_info);
      print_vector(macd_nodes_info);
    }else if(input == "p" || input == "plot"){plot_filter();}
    else if(input == "n" || input == "new"){
      if(new_filter()){
	break;
      }
    }else if(input == "s" || input == "sells"){
      if(sell_history_client.call(srv)){
	sell_history_routine(srv.response.history);
      }else{
	ROS_ERROR("Failed to call service /sell_history");
	return 1;
      }
    }else if(input == "killall"){
      kill_all_macd_nodes();
    }else if(input == "k" || input == "kill"){
      kill_individual_nodes();
    }
  }
  spinner.stop();
  
}
