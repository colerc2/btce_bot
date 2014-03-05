#include <ros/ros.h>
#include <ros/time.h>
#include <string>
#include <vector>
#include <macd_sell_signal/macd_indicator.h>//macd_indicator object
#include "ticker_publisher/ticker.h"//ticker msg
#include "macd_sell_signal/macd.h"//macd msg
#include "macd_sell_signal/macd_array.h"//macd array service

//passed by reference to the macd_indicator object
double short_emaf_line_, long_emaf_line_, macd_line_, signal_line_, macd_hist_;

//keeping vector of old points around for service
std::vector<macd_sell_signal::macd> old_points_;

//MACD object
macd_indicator macd_;

//parameters read in at runtime
int short_, long_, sig_, period_, num_old_periods_, spread_window_;
double spread_value_;
std::string trade_pair_;

//publishers/subscribers
ros::Subscriber ticker_sub_;
ros::Publisher macd_pub_;

void ticker_callback(const ticker_publisher::ticker::ConstPtr &msg){
  //ROS_INFO("\nhigh:\t%f\nlow:\t%f\navg\t%f\nvol\t%f\nvol_cur\t%f\nlast\t%f\nbuy\t%f\nsell\t%f",
  //	   msg->high, msg->low, msg->avg, msg->vol, msg->vol_cur, msg->last, msg->buy, msg->sell);
 
  //grab data from msg and pass through MACD
  macd_.update(msg->last, short_emaf_line_, long_emaf_line_, macd_line_, signal_line_, macd_hist_);
  //ROS_INFO("(%f,%f,%f,%f,%f)",short_emaf_line_, long_emaf_line_, macd_line_, signal_line_, macd_hist_);
  
  //publish MACD info for current time
  macd_sell_signal::macd macd_msg;
  //header
  macd_msg.header.stamp = ros::Time::now();
  //save ticker values
  macd_msg.tick = *msg;
  //macd stuff
  macd_msg.short_emaf = short_emaf_line_;
  macd_msg.long_emaf = long_emaf_line_;
  macd_msg.macd = macd_line_;
  macd_msg.signal = signal_line_;
  macd_msg.macd_hist = macd_hist_;
  macd_pub_.publish(macd_msg);

  //pop front, push back
  if(old_points_.size() > num_old_periods_*period_){
    old_points_.erase(old_points_.begin());
  }
  old_points_.push_back(macd_msg);
}

bool request_array(macd_sell_signal::macd_array::Request &req,
		   macd_sell_signal::macd_array::Response &res){
  res.macd_array = old_points_;

  return true;
}

int main(int argc, char** argv){
  //Node setup
  ros::init(argc, argv, "macd_sell_signal_node");
  ros::NodeHandle n;//global
  ros::NodeHandle nh("~"); //local

  //grab parameters
  nh.param("trade_pair", trade_pair_, std::string("error"));
  nh.param("short", short_, 0);
  nh.param("long", long_, 0);
  nh.param("sig", sig_, 0);
  nh.param("period", period_, 0);
  nh.param("num_old_periods", num_old_periods_, 0);
  nh.param("spread_window", spread_window_, 0);
  nh.param("spread_value", spread_value_, 0.0);

  //Subscribers
  //construct topic name from trade_pair_ string
  std::string ticker_topic = "ticker_" + trade_pair_;
  ticker_sub_ = n.subscribe(ticker_topic,1,ticker_callback);

  //Publishers
  std::string macd_topic = "macd_" + std::to_string(short_) + 
    "_" + std::to_string(long_) + "_" + std::to_string(sig_) +
    "_x" + std::to_string(period_) + "/macd";
  macd_pub_ = n.advertise<macd_sell_signal::macd>(macd_topic, 10);
  
  //Services
  std::string macd_array_service = "macd_" + std::to_string(short_) + 
    "_" + std::to_string(long_) + "_" + std::to_string(sig_) +
    "_x" + std::to_string(period_) + "/macd_array";
  ros::ServiceServer service = n.advertiseService(macd_array_service, request_array);
  ROS_INFO("Ready to provide macd_array service.");

  //intialize globals
  macd_ = macd_indicator(short_, long_, sig_, period_);
 
 
  //
  ros::Rate rate(100);
  //only use one thread for callbacks, that was the service never responds
  //while the array is being manipulated
  ros::AsyncSpinner spinner(1);
  spinner.start();
  while(ros::ok()){
    rate.sleep();
  }
  spinner.stop();
  
  return 0;
}
