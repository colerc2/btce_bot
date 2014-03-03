#include <ros/ros.h>
#include <ros/time.h>
#include "ticker_publisher/ticker.h"

//TODO: make these a circular buffer/queue
double short_emaf_, long_emaf_, macd_, signal_macd_;
//parameters read in at runtime
double short_, long_, sig_, period_;

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
  ros::init(argc, argv, "sell_signal_node");
  ros::NodeHandle n;//global
  ros::NodeHandle nh; //local

  ros::Subscriber ticker_sub = n.subscribe("ticker",1,ticker_callback);
  
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
