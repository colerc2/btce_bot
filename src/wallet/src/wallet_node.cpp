#include <ros/ros.h>
#include <ros/time.h>
#include <string>
#include <vector>
#include <map>
#include <ticker_publisher/ticker.h>
#include <macd_sell_signal/macd.h>
#include <macd_sell_signal/sell.h>


struct wallet {
  double btc_balance;
  double ltc_balance;
  double usd_balance;
  
  int active_orders;
};
ros::Subscriber sell_subscriber_;

void sell_callback(const macd_sell_signal::sell::ConstPtr &msg){
  
}

int main(int argc, char** argv){
  //Node setup
  ros::init(argc, argv, "node name");
  ros::NodeHandle n;//global
  ros::NodeHandle nh("~");//local
  
  std::string some_param;
  int some_other_param;

  //Services
  //ros::ServiceServer service = n.advertiseService("service_name", service_request);

  //TODO: routine that syncs this wallet with BTC-e wallet
  
  //Subscribers
  std::string sell_topic = "sell";
  global_sub_ = n.subscribe(sell_topic, 1, sell_callback);
  
  //Publishers
  //global_pub_ = n.advertise<example_node::some_msg>("topic_name", 10);

  ros::Rate rate(1);
  //use a single thread for callbacks
  ros::AsyncSpinner spinner(1);
  spinner.start();
  while(ros::ok()){
    //every few seconds, check to make sure our wallet is synced with BTC-e wallet
    rate.sleep();
  }
  spinner.stop();

  return 0;
}
