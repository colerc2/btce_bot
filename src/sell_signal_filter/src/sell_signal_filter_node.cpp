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
#include <macd_sell_signal/macd.h>
#include <ticker_publisher/ticker.h>
#include <sell_signal_filter/history.h>

std::vector<macd_sell_signal::sell> sells_;
ros::Publisher sell_pub_;
std::vector<ros::Subscriber> subscribers_;
std::vector<std::string> subscriber_names_;

void sell_callback(const macd_sell_signal::sell::ConstPtr &msg){
  //check if sell should actually happen here
  ROS_INFO("Sell signal callback in sell filter");
  
  /*for now, we just pass every sell signal through
    TODO: filter these signals, check btce health,
    basically make sure that we really do want to sell*/

  //add sell to vector of sells
  sells_.push_back(*msg);

  //copy msg using assignment operator!? NOT SURE IF WANT OR NEED
  macd_sell_signal::sell tmp_msg = *msg;
  
  //publish this same sell to the real sell topic if we decide it's valid
  sell_pub_.publish(tmp_msg);
  
}

void check_for_new_sell_signals(ros::NodeHandle &n){
  //call rostopic list
  std::string cmd = "rostopic list";
  FILE * cmd_output = popen(cmd.c_str(), "r");

  //put this crap in a vector
  std::vector<std::string> topics;
  char line[128];
  while( fgets (line, sizeof line, cmd_output)){
    topics.push_back(line);
  }
  //loop through and subscribe to all of the sell topics
  for(int i = 0; i < topics.size(); i++){
    //topics[i].erase(0, topics[i].find_first_not_of('\n'));
    //topics[i].erase(topics[i].find_last_not_of('\n')+1); 
    topics[i].erase(std::remove(topics[i].begin(), topics[i].end(), '\n'), topics[i].end());
    //all sell topics will begin with "/macd" and end with _sell
    if((topics[i].substr(0,5) == "/macd") &&
       (topics[i].substr(topics[i].size()-4, topics[i].size()) == "sell")){
      if(std::find(subscriber_names_.begin(),
		   subscriber_names_.end(), topics[i]) == subscriber_names_.end()){
	subscribers_.push_back(n.subscribe(topics[i], 1, sell_callback));
	subscriber_names_.push_back(topics[i]);
	ROS_INFO("Sell filter subscribed to: %s", topics[i].c_str());
      }
    }
  }
}

bool request_sell_history(sell_signal_filter::history::Request &req,
			  sell_signal_filter::history::Response &res){
  res.history = sells_;
  return true;
}

int main(int argc, char** argv){
  ros::init(argc, argv, "sell_signal_filter_node");
  ros::NodeHandle n;
  ros::NodeHandle nh("~");

  //sleep for a few seconds, give topics time to start publishing
  usleep(1000000);
  
  //subscribers
  check_for_new_sell_signals(n);

  //Publishers
  std::string sell_topic = "sell";
  sell_pub_ = n.advertise<macd_sell_signal::sell>(sell_topic, 10);

  //Services(servers)
  std::string sell_history_service_name = "sell_history";
  ros::ServiceServer sell_history_service = n.advertiseService(sell_history_service_name, request_sell_history);

  ros::Rate rate(0.1);//check for new topics every 10 seconds
  //use a seperate thread for callbacks, might do something with current thread later
  ros::AsyncSpinner spinner(1);
  spinner.start();
  while(ros::ok()){
    check_for_new_sell_signals(n);
    rate.sleep();
  }
  spinner.stop();

  return 0;
}
