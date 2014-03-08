#include <ros/ros.h>
#include <ros/time.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <ticker_publisher/ticker.h>
#include <btce_health/server_time.h>

std::vector<std::string> tickers_;
std::vector<ros::Subscriber> subs_;
std::string most_recent_server_time_;

void ticker_callback(const ticker_publisher::ticker::ConstPtr &message){
  most_recent_server_time_ = message->server_time;
}

void check_for_new_tickers(ros::NodeHandle &n){
  //call rostopic list
  std::string cmd = "rostopic list";
  FILE * cmd_output = popen(cmd.c_str(), "r");

  //put this crap in a vector
  std::vector<std::string> topics;
  char line[128];
  while( fgets (line, sizeof line, cmd_output)){
    topics.push_back(line);
  }
  pclose(cmd_output);//shit gets pretty walking dead up in here without this line
  //loop through and subscribe to all of the ticker topics
  for(unsigned int i = 0; i < topics.size(); i++){
    topics[i].erase(std::remove(topics[i].begin(), topics[i].end(), '\n'), topics[i].end());

    //all ticker topics will begin with "/ticker_"
    if(topics[i].substr(0,8) == "/ticker_"){

      //if not subscribed to this one, subscribe to it
      if(std::find(tickers_.begin(), tickers_.end(), topics[i]) == tickers_.end()){
	ROS_INFO("subscribing to %s", topics[i].c_str());
	subs_.push_back(n.subscribe(topics[i], 1, ticker_callback));
	tickers_.push_back(topics[i]);
     
      }
    }
  }
}

bool request_time(btce_health::server_time::Request &req,
		     btce_health::server_time::Response &res){
  res.server_time = most_recent_server_time_;
  
  return true;
}


int main(int argc, char** argv){
  //Node setup
  ros::init(argc, argv, "save_load_ticker_node");
  ros::NodeHandle n;//global
  ros::NodeHandle nh("~"); //local

  //Subscribers
  //check to see what tickers are running, populate vector
  check_for_new_tickers(n);

  //Services
  ros::ServiceServer provide_server_time = n.advertiseService("server_time", request_time);

  //go into loop
  ros::Rate rate(0.1);//check for new tickers every 10 seconds
  //only use one thread for callbacks for now
  ros::AsyncSpinner spinner(1);
  spinner.start();
  while(ros::ok()){
    check_for_new_tickers(n);
    rate.sleep();
  }
  spinner.stop();


}
