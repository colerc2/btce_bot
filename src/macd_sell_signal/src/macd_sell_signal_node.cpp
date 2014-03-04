#include <ros/ros.h>
#include <ros/time.h>
#include <string>
#include "ticker_publisher/ticker.h"

double emaf_;//for testing
//TODO: make these a circular buffer/queue
double short_emaf_, long_emaf_, macd_, signal_macd_;

//parameters read in at runtime
int short_, long_, sig_, period_, spread_window_;
double spread_value_;
std::string trade_pair_;

void exponential_moving_average(double data, double alpha){
  emaf_ = alpha*data + (1-alpha)*emaf_;
}

void ticker_callback(const ticker_publisher::ticker::ConstPtr &msg){
  //ROS_INFO("\nhigh:\t%f\nlow:\t%f\navg\t%f\nvol\t%f\nvol_cur\t%f\nlast\t%f\nbuy\t%f\nsell\t%f",
  //	   msg->high, msg->low, msg->avg, msg->vol, msg->vol_cur, msg->last, msg->buy, msg->sell);
  ticker_publisher::ticker ticker_msg = *msg;
  double alpha = (2.0/130);
  exponential_moving_average(ticker_msg.last, alpha);
  ROS_INFO("EMAF:\t%f", emaf_);
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
  emaf_ = 0;
 
 
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
