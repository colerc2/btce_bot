#include <ros/ros.h>
#include <ros/time.h>
#include <string>
#include <vector>
#include <macd_sell_signal/macd_indicator.h>//macd_indicator object
#include "ticker_publisher/ticker.h"//ticker msg
#include "macd_sell_signal/macd.h"//macd msg
#include "macd_sell_signal/sell.h"
#include "macd_sell_signal/macd_array.h"//macd array service
#include "save_load_ticker/history.h"

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
ros::Publisher sell_pub_;

void publish_sell(){
  macd_sell_signal::sell sell_msg;
  std::vector<macd_sell_signal::macd>::reverse_iterator rit = old_points_.rbegin();
  //header
  sell_msg.header.stamp = ros::Time::now();
  //save MACD params
  sell_msg.short_ema = short_;
  sell_msg.long_ema = long_;
  sell_msg.sig_ema = sig_;
  sell_msg.period = period_;
  sell_msg.spread_window = spread_window_;
  sell_msg.spread_thresh = spread_value_;
  //save MACD/ticker values before and after sell
  sell_msg.current = *rit;
  rit++;
  sell_msg.previous = *rit;
  
  //publish sell
  sell_pub_.publish(sell_msg);
  
}

void check_if_should_sell(){
  if((old_points_.size() > period_*long_) &&
     ((old_points_[old_points_.size()-2]).macd_hist > 0) &&//last point was positive
     ((old_points_[old_points_.size()-1]).macd_hist < 0)){
    //now check the more complicated stuff
    int counter = 0;
    int max = 0;
    bool sell_signal = false;
    std::vector<macd_sell_signal::macd>::reverse_iterator rit = old_points_.rbegin();
    for ( rit = old_points_.rbegin();
	  ((rit != old_points_.rend()) && (counter < spread_window_));
	  rit++){
      if(rit->macd_hist > spread_value_){
	ROS_INFO("We got a sell signal from MACD(%d,%d,%d)x%d,%d,thresh=%lf",short_,long_,sig_,period_,spread_window_,spread_value_);
	sell_signal = true;
      }
      counter++;
    }
    if(sell_signal){
      publish_sell();
    }
  }
  
}

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

  //pop front, push back (always save at least the last hour of data)
  while(old_points_.size() > std::max(num_old_periods_*period_,3600)){
    old_points_.erase(old_points_.begin());
  }
  old_points_.push_back(macd_msg);

  //check if it's time to sell breh
  check_if_should_sell();
}

bool request_array(macd_sell_signal::macd_array::Request &req,
		   macd_sell_signal::macd_array::Response &res){
  res.macd_array = old_points_;

  return true;
}

void read_in_history(std::vector<ticker_publisher::ticker> &ticker){
  macd_sell_signal::macd macd_msg;
  for(int i = 0; i < ticker.size(); i++){
    macd_.update(ticker[i].last, short_emaf_line_, long_emaf_line_, macd_line_, signal_line_, macd_hist_);

    //publish MACD info for current time
    macd_sell_signal::macd macd_msg;
    //header
    macd_msg.header.stamp = ros::Time::now();
    //save ticker values
    macd_msg.tick = ticker[i];
    //macd stuff
    macd_msg.short_emaf = short_emaf_line_;
    macd_msg.long_emaf = long_emaf_line_;
    macd_msg.macd = macd_line_;
    macd_msg.signal = signal_line_;
    macd_msg.macd_hist = macd_hist_;
    
    old_points_.push_back(macd_msg);
  }

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

   //intialize globals
   macd_ = macd_indicator(short_, long_, sig_, period_);

   //Services (servers)
   std::string macd_array_service_name = "macd_" + std::to_string(short_) + 
     "_" + std::to_string(long_) + "_" + std::to_string(sig_) +
     "_x" + std::to_string(period_) + "/macd_array";
   ros::ServiceServer macd_array_service = n.advertiseService(macd_array_service_name, request_array);
   ROS_INFO("Ready to provide macd_array service.");
   //Services (clients)
   std::string history_service = "/ticker_" + trade_pair_ + "_history";
   ros::ServiceClient history_client = n.serviceClient<save_load_ticker::history>(history_service);
   save_load_ticker::history srv;
   srv.request.trade_pair = trade_pair_;
   srv.request.num_points = std::max(num_old_periods_*period_,3600);
   if (history_client.call(srv))
   {
     read_in_history(srv.response.ticker);
   }
   else
   {
     ROS_ERROR("Failed to call service /ticker_%s_history", trade_pair_.c_str());
     return 1;
   }

  //Subscribers
  //construct topic name from trade_pair_ string
  std::string ticker_topic = "ticker_" + trade_pair_;
  ticker_sub_ = n.subscribe(ticker_topic,1,ticker_callback);

  //Publishers
  //macd topic
  std::string macd_topic = "macd_" + std::to_string(short_) + 
    "_" + std::to_string(long_) + "_" + std::to_string(sig_) +
    "_x" + std::to_string(period_) + "/macd";
  macd_pub_ = n.advertise<macd_sell_signal::macd>(macd_topic, 10);
  //sell topic
  std::string sell_topic = "macd_" + std::to_string(short_) + 
    "_" + std::to_string(long_) + "_" + std::to_string(sig_) +
    "_x" + std::to_string(period_) + "/sell";
  sell_pub_ = n.advertise<macd_sell_signal::sell>(sell_topic, 10);



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
