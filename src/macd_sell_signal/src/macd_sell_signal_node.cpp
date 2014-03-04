#include <ros/ros.h>
#include <ros/time.h>
#include <string>
#include "ticker_publisher/ticker.h"
#include <macd_sell_signal/macd_indicator.h>
#include "macd_sell_signal/macd.h"

//TODO: make these a circular buffer/queue
double short_emaf_line_, long_emaf_line_, macd_line_, signal_line_, macd_hist_;

//MACD object
macd_indicator* macd_;

//parameters read in at runtime
int short_, long_, sig_, period_, spread_window_;
double spread_value_;
std::string trade_pair_;

//publishers/subscribers
ros::Subscriber ticker_sub_;
ros::Publisher macd_pub_;

void ticker_callback(const ticker_publisher::ticker::ConstPtr &msg){
  //ROS_INFO("\nhigh:\t%f\nlow:\t%f\navg\t%f\nvol\t%f\nvol_cur\t%f\nlast\t%f\nbuy\t%f\nsell\t%f",
  //	   msg->high, msg->low, msg->avg, msg->vol, msg->vol_cur, msg->last, msg->buy, msg->sell);
 
  //grab data from msg and pass through MACD
  //ticker_publisher::ticker ticker_msg = *msg;
  macd_->update(msg->last, short_emaf_line_, long_emaf_line_, macd_line_, signal_line_, macd_hist_);
  //ROS_INFO("(%f,%f,%f,%f,%f)",short_emaf_line_, long_emaf_line_, macd_line_, signal_line_, macd_hist_);
  
  //publish MACD info
  macd_sell_signal::macd macd_msg;
  macd_msg.header.stamp = ros::Time::now();
  macd_msg.short_emaf = short_emaf_line_;
  macd_msg.long_emaf = long_emaf_line_;
  macd_msg.macd = macd_line_;
  macd_msg.signal = signal_line_;
  macd_msg.macd_hist = macd_hist_;
  macd_pub_.publish(macd_msg);
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
  ticker_sub_ = n.subscribe(ticker_topic,1,ticker_callback);

  //Publishers
  //TODO: topic should change based on parameters
  std::string macd_topic = "macd_" + std::to_string(short_) + 
    "_" + std::to_string(long_) + "_" + std::to_string(sig_) +
    "_x" + std::to_string(period_);
  macd_pub_ = n.advertise<macd_sell_signal::macd>(macd_topic, 10);
  
  //intialize globals
  macd_ = new macd_indicator(short_, long_, sig_, period_);
 
 
  //
  ros::Rate rate(100);
  ros::AsyncSpinner spinner(1);
  spinner.start();
  while(ros::ok()){
    rate.sleep();
  }
  spinner.stop();
  
  return 0;
}
