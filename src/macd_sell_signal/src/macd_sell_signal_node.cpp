#include <ros/ros.h>
#include <ros/time.h>
#include <string>
#include "ticker_publisher/ticker.h"
#include <macd_sell_signal/macd_indicator.h>

double emaf_;//for testing
//TODO: make these a circular buffer/queue
double short_emaf_line_, long_emaf_line_, macd_line_, signal_line_, macd_hist_;

//MACD object
macd_indicator* macd_;

//parameters read in at runtime
int short_, long_, sig_, period_, spread_window_;
double spread_value_;
std::string trade_pair_;

void ticker_callback(const ticker_publisher::ticker::ConstPtr &msg){
  //ROS_INFO("\nhigh:\t%f\nlow:\t%f\navg\t%f\nvol\t%f\nvol_cur\t%f\nlast\t%f\nbuy\t%f\nsell\t%f",
  //	   msg->high, msg->low, msg->avg, msg->vol, msg->vol_cur, msg->last, msg->buy, msg->sell);
  ticker_publisher::ticker ticker_msg = *msg;
  macd_->update(msg->last, short_emaf_line_, long_emaf_line_, macd_line_, signal_line_, macd_hist_);
  ROS_INFO("(%f,%f,%f,%f,%f)",short_emaf_line_, long_emaf_line_, macd_line_, signal_line_, macd_hist_);
  
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
  nh.param("spread_window", spread_window_, 0);
  nh.param("spread_value", spread_value_, 0.0);

  //Subscribers
  //construct topic name from trade_pair_ string
  std::string ticker_topic = "ticker_" + trade_pair_;
  ros::Subscriber ticker_sub = n.subscribe(ticker_topic,1,ticker_callback);
  
  //intialize globals
  macd_ = new macd_indicator(short_, long_, sig_, period_);
 
 
  //handle callbacks in separate thread, not necessary now, but might be later
  ros::Rate rate(100);
  ros::AsyncSpinner spinner(1);
  spinner.start();
  while(ros::ok()){
    rate.sleep();
  }
  spinner.stop();
  
  return 0;
}
