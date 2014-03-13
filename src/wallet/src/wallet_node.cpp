#include <ros/ros.h>
#include <ros/time.h>
#include <string>
#include <vector>
#include <map>
#include <ticker_publisher/ticker.h>
#include <macd_sell_signal/macd.h>
#include <macd_sell_signal/sell.h>
#include <trade_interface/get_info.h>
#include <trade_interface/get_info_all.h>

#define LOCAL 0
#define REMOTE 1

struct wallet {
  double btc_balance[2];
  double ltc_balance[2];
  double usd_balance[2];
  
  int active_orders;
};
ros::Subscriber sell_subscriber_;

void sell_callback(const macd_sell_signal::sell::ConstPtr &msg){
  
}

void handle_get_info_res(trade_interface::get_info_all::Response &res){
  std::cout << "Transaction count: " << res.transaction_count << std::endl;
  std::cout << "Open orders: " << res.open_orders << std::endl;
  std::cout << "Server Time: " << res.server_time << std::endl;
  for(unsigned int i = 0; i < res.info.size(); i++){
    std::cout << "Coin   : " << res.info[i].coin << std::endl;
    std::cout << "Balance: " << res.info[i].balance << std::endl;
    std::cout << "------------------------" << std::endl;
  }
}

int main(int argc, char** argv){
  //Node setup
  ros::init(argc, argv, "wallet_node");
  ros::NodeHandle n;//global
  ros::NodeHandle nh("~");//local
  
  //std::string some_param;
  //int some_other_param;

  //Services
  //ros::ServiceServer service = n.advertiseService("service_name", service_request);
  ros::ServiceClient get_info_client = n.serviceClient<trade_interface::get_info_all>("get_info_service");
  trade_interface::get_info_all srv;

  //TODO: routine that syncs this wallet with BTC-e wallet
  
  //Subscribers
  //std::string sell_topic = "sell";
  //global_sub_ = n.subscribe(sell_topic, 1, sell_callback);
  
  //Publishers
  //global_pub_ = n.advertise<example_node::some_msg>("topic_name", 10);

  ros::Rate rate(5);
  //use a single thread for callbacks
  ros::AsyncSpinner spinner(1);
  spinner.start();
  while(ros::ok()){
    //every few seconds, check to make sure our wallet is synced with BTC-e wallet
    if(get_info_client.call(srv)){
      handle_get_info_res(srv.response);
    }else{
      ROS_ERROR("Failed to call service /get_info_service");
      return 1;
    }
    rate.sleep();
  }
  spinner.stop();

  return 0;
}
