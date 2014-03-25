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
#include <trade_interface/trans_history.h>
#include <trade_interface/trans_history_all.h>
#include <trade_interface/active_orders.h>
#include <trade_interface/active_orders_all.h>
#include <trade_interface/cancel_order.h>
#include <trade_interface/make_trade.h>

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

void handle_active_orders_res(trade_interface::active_orders_all::Response &res){
  for(unsigned int i = 0; i < res.orders.size(); i++){
    std::cout << "Order id     : " << res.orders[i].order_id << std::endl;
    std::cout << "Type         : " << res.orders[i].type << std::endl;
    std::cout << "Pair         : " << res.orders[i].pair << std::endl;
    std::cout << "Rate         : " << res.orders[i].rate << std::endl;
    std::cout << "Amount       : " << res.orders[i].amount << std::endl;
    std::cout << "Time created : " << res.orders[i].timestamp_created << std::endl;
    std::cout << "Status       : " << res.orders[i].status << std::endl;
    std::cout << "----------------------------" << std::endl;
  }
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

void handle_trans_history_res(trade_interface::trans_history_all::Response &res){
  for(unsigned int i = 0; i < res.trans_hist_array.size(); i++){
    std::cout << "Transaction id   : " << res.trans_hist_array[i].transaction_id << std::endl;
    std::cout << "Type             : " << res.trans_hist_array[i].type << std::endl;
    std::cout << "Amount           : " << res.trans_hist_array[i].amount << std::endl;
    std::cout << "Currency         : " << res.trans_hist_array[i].currency << std::endl;
    std::cout << "Description      : " << res.trans_hist_array[i].desc << std::endl;
    std::cout << "Status           : " << res.trans_hist_array[i].status << std::endl;
    std::cout << "Timestamp        : " << res.trans_hist_array[i].timestamp << std::endl;
  }
}

int main(int argc, char** argv){
  //Node setup
  ros::init(argc, argv, "wallet_node");
  ros::NodeHandle n;//global
  ros::NodeHandle nh("~");//local

  //get_info service
  ros::ServiceClient get_info_client = n.serviceClient<trade_interface::get_info_all>("get_info_service");
  trade_interface::get_info_all get_info_srv;
  //trans_history service
  ros::ServiceClient trans_history_client = n.serviceClient<trade_interface::trans_history_all>("trans_history_service");
  trade_interface::trans_history_all trans_history_srv;
  //active orders service
  ros::ServiceClient active_orders_client = n.serviceClient<trade_interface::active_orders_all>("active_orders_service");
  trade_interface::active_orders_all active_orders_srv;
  //cancel order service
  ros::ServiceClient cancel_order_client = n.serviceClient<trade_interface::cancel_order>("cancel_order_service");
  trade_interface::cancel_order cancel_order_srv;
  //make_trade 
  ros::ServiceClient make_trade_client = n.serviceClient<trade_interface::make_trade>("make_trade_service");
  trade_interface::make_trade make_trade_srv;

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
    if(get_info_client.call(get_info_srv)){
      //handle_get_info_res(get_info_srv.response);
    }else{
      ROS_ERROR("Failed to call service /get_info_service");
      return 1;
    }
    //call trans_history_all service
    if(trans_history_client.call(trans_history_srv)){
      //handle_trans_history_res(trans_history_srv.response);
    }else{
      ROS_ERROR("Failed to call service /trans_history_service");
      return 1;
    }
    //call active orders service
    if(active_orders_client.call(active_orders_srv)){
      handle_active_orders_res(active_orders_srv.response);
    }else{
      ROS_ERROR("Failed to call service /active_orders_service");
      return 1;
    }
    //cancel order service
    cancel_order_srv.request.order_id = 0;
    if(cancel_order_client.call(cancel_order_srv)){
      ROS_INFO("Cancel order %ld success: %d", cancel_order_srv.request.order_id, cancel_order_srv.response.completed);
    }else{
      ROS_ERROR("Failed to call service /cancel_order_service");
      return 1;
    }
    //call trade service
    make_trade_srv.request.pair = "ltc_usd";
    make_trade_srv.request.buy_or_sell = "sell";
    make_trade_srv.request.price = 20.6343543232;
    make_trade_srv.request.amount = 1;
    if(make_trade_client.call(make_trade_srv)){
      ROS_INFO("Trade order success ahhhhhhh");
    }else{
      ROS_ERROR("Failed to call service /make_trade_service");
      return 1;
    }
    
    

    rate.sleep();
  }
  spinner.stop();

  return 0;
}
