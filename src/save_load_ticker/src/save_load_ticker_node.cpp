#include <ros/ros.h>
#include <ros/time.h>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <ticker_publisher/ticker.h>

std::vector<std::string> tickers_;
std::vector<ros::Subscriber> subs_;
std::map<std::string, std::fstream*> file_handles_;//have to keep pointers to ofstream unfortunately
std::string base_file_;

void ticker_callback(const ticker_publisher::ticker::ConstPtr &message){
  ticker_publisher::ticker msg = *message;
  
  //construct filename
  std::string filename = base_file_ + msg.trade_pair + "/" + 
    msg.trade_pair + "_" + msg.server_time.substr(0,10) + ".tkr";
  std::replace(filename.begin(), filename.end(), '-', '_');//replace hyphens with underscores
  std::fstream *file_handle;
  std::map<std::string, std::fstream*>::const_iterator it = file_handles_.find(filename);
  if(it == file_handles_.end()){//need to create new file handle
    file_handle = new std::fstream(filename.c_str(),std::ios::app | std::ios::out);
    file_handles_[filename] = file_handle;
    ROS_INFO("New file handle created for pair %s: %s", msg.trade_pair.c_str(), filename.c_str());
  }else{//use old file handle
    file_handle = (file_handles_[filename]);
  }

  if(msg.trade_pair == "btc_usd"){
    //ROS_INFO("Callback for bitcoin");
  }

  //save data to file
  (*file_handle) << msg.high << "," << msg.low << "," << msg.avg << "," << msg.vol << "," <<
    msg.vol_cur << "," << msg.last << "," << msg.buy << "," << msg.sell << "," << msg.updated << "," <<
    msg.server_time << "\n";
  file_handle->flush();//write to file each time (slower but better for when data is read later)
  
  
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
  //loop through and subscribe to all of the sell topics
  for(int i = 0; i < topics.size(); i++){
    topics[i].erase(0, topics[i].find_first_not_of('\n'));
    topics[i].erase(topics[i].find_last_not_of('\n')+1); 
    //all sell topics will begin with "/ticker_"
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

int main(int argc, char** argv){
  //Node setup
  ros::init(argc, argv, "save_load_ticker_node");
  ros::NodeHandle n;//global
  ros::NodeHandle nh("~"); //local

  //TODO:make this a param
  base_file_ = "/home/bob/Documents/btce_bot/ticker_data/";

  ROS_INFO("Checking for new tickers");
  //check to see what tickers are running, populate vector
  check_for_new_tickers(n);

  ros::Rate rate(1);
  //only use one thread for callbacks for now
  ros::AsyncSpinner spinner(1);
  spinner.start();
  while(ros::ok()){
    check_for_new_tickers(n);
    rate.sleep();
  }
  spinner.stop();


}
