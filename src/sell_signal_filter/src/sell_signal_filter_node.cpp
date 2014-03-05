#include <ros/ros.h>
#include <ros/time.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <fstream>
#include <iomanip>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <macd_sell_signal/sell.h>

void sell_callback(const macd_sell_signal::sell::ConstPtr &msg){

}


int main(int argc, char** argv){
  ros::init(argc, argv, "sell_signal_filter_node");
  ros::NodeHandle n;
  ros::NodeHandle nh("~");

  //sleep for a few seconds, give topics time to start publishing
  usleep(1000000);
  
  //call rostopic list
  std::string cmd = "rostopic list";
  FILE * cmd_output = popen(cmd.c_str(), "r");

  //put this crap in a vector
  std::vector<std::string> topics;
  char line[128];
  while( fgets (line, sizeof line, cmd_output)){
    topics.push_back(line);
  }
  std::vector<ros::Subscriber> subscribers;
  //loop through and subscribe to all of the sell topics
  for(int i = 0; i < topics.size(); i++){
    topics[i].erase(0, topics[i].find_first_not_of('\n'));
    topics[i].erase(topics[i].find_last_not_of('\n')+1); 
    //all sell topics will begin with "/macd" and end with _sell
    if((topics[i].substr(0,5) == "/macd") &&
       (topics[i].substr(topics[i].size()-4, topics[i].size()) == "sell")){
      subscribers.push_back(n.subscribe(topics[i], 1, sell_callback));
    }
  }

  ros::Rate rate(100);
  //use a seperate thread for callbacks, might do something with current thread later
  ros::AsyncSpinner spinner(1);
  spinner.start();
  while(ros::ok()){
    rate.sleep();
  }
  spinner.stop();

  return 0;
}
