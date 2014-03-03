#include <ros/ros.h>
#include <ros/time.h>
#include "ticker_publisher/ticker.h"

void ticker_callback(const ticker_publisher::ticker::ConstPtr &msg){
  ROS_INFO("\nhigh:\t%f\nlow:\t%f\navg\t%f\nvol\t%f\nvol_cur\t%f\nlast\t%f\nbuy\t%f\nsell\t%f",
	   msg->high, msg->low, msg->avg, msg->vol, msg->vol_cur, msg->last, msg->buy, msg->sell);
}

int main(int argc, char** argv){
  //Node setup
  ros::init(argc, argv, "sell_signal_node");
  ros::NodeHandle n;//global
  ros::NodeHandle nh; //local

  ros::Subscriber ticker_sub = n.subscribe("ticker",1,ticker_callback);
 
 
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
