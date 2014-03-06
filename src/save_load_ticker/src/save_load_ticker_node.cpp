#include <ros/ros.h>
#include <ros/time.h>
#include <vector>
#include <string>
#include <ticker_publisher/ticker.h>


int main(int argc, char** argv){
  //Node setup
  ros::init(argc, argv, "save_load_ticker_node");
  ros::NodeHandle n;//global
  ros::NodeHandle nh("~"); //local
  
  ros::Rate rate(100);
  //only use one thread for callbacks for now
  ros::AsyncSpinner spinner(1);
  spinner.start();
  while(ros::ok()){
    rate.sleep();
  }
  spinner.stop();


}
