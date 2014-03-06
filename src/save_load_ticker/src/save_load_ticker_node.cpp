#include <ros/ros.h>
#include <ros/time.h>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <ticker_publisher/ticker.h>
#include <save_load_ticker/history.h>


std::vector<std::string> tickers_;
std::vector<ros::Subscriber> subs_;
std::map<std::string, std::fstream*> file_handles_;//have to keep pointers to ofstream unfortunately
std::map<std::string, std::string> trade_pair_files_;//this will track what file is associated with what pair
std::string base_file_;

//TODO: make this more sophisticated, i.e. if there aren't enough points in current file, go back to previous days file
void retreive_hist_from_file(std::string &trade_pair, int &num_points_req, int &num_points, int &valid_number, std::vector<ticker_publisher::ticker> &points){
  std::map<std::string, std::string>::const_iterator it = trade_pair_files_.find(trade_pair);
  if(it == trade_pair_files_.end()){
    num_points = 0;
    valid_number = 0;
  }else{
    num_points = 0;
    valid_number = 0;
    //open file for input
    std::ifstream input_file;
    input_file.open(trade_pair_files_[trade_pair].c_str());
    std::string temp;
    while(std::getline(input_file,temp)){
      //make sure we have the right number of commas
      if(std::count(temp.begin(), temp.end(), ',') == 9){
	//TODO: might need to do some checking here for partial lines
	std::stringstream ss(temp);
	std::string item;
	int counter = 0;

	ticker_publisher::ticker tmp;
	std::getline(ss, item, ',');
	tmp.high = atof(item.c_str());
	std::getline(ss, item, ',');
	tmp.low = atof(item.c_str());
	std::getline(ss, item, ',');
	tmp.avg = atof(item.c_str());
	std::getline(ss, item, ',');
	tmp.vol = atof(item.c_str());
	std::getline(ss, item, ',');
	tmp.vol_cur = atof(item.c_str());
	std::getline(ss, item, ',');
	tmp.last = atof(item.c_str());
	std::getline(ss, item, ',');
	tmp.buy = atof(item.c_str());
	std::getline(ss, item, ',');
	tmp.sell = atof(item.c_str());
	std::getline(ss, item, ',');
	tmp.updated = item;
	std::getline(ss, item, ',');
	tmp.server_time = item;
	
	points.push_back(tmp);
	
	valid_number++;
	num_points++;
      }
    }
    //TODO: make this more efficient
    while(points.size() > num_points_req){
      points.erase(points.begin());
      valid_number--;
      num_points--;
    }
  }
}

bool request_history(save_load_ticker::history::Request &req,
		     save_load_ticker::history::Response &res){
  //std::string pair = req.trade_pair;
  //int num_points = req.num_points;
 
  retreive_hist_from_file(req.trade_pair, req.num_points, res.num_points, res.valid_number, res.ticker);
 
  return true;
}

void ticker_callback(const ticker_publisher::ticker::ConstPtr &message){
  ticker_publisher::ticker msg = *message;
  
  //construct filename
  std::string filename = base_file_ + msg.trade_pair + "/" + 
    msg.trade_pair + "_" + msg.server_time.substr(0,10) + ".tkr";
  std::replace(filename.begin(), filename.end(), '-', '_');//replace hyphens with underscores
  
  //create file handle, check
  std::fstream *file_handle;
  std::map<std::string, std::fstream*>::const_iterator it = file_handles_.find(filename);
  if(it == file_handles_.end()){//need to create new file handle
    file_handle = new std::fstream(filename.c_str(),std::ios::app | std::ios::out);
    file_handles_[filename] = file_handle;
    trade_pair_files_[msg.trade_pair] = filename;
    //TODO: close old file, linux will get pissed once i have 1023 files
    ROS_INFO("New file handle created for pair %s: %s", msg.trade_pair.c_str(), filename.c_str());
  }else{//use old file handle
    file_handle = (file_handles_[filename]);
  }

  //save data to file
  (*file_handle) << msg.high << "," << msg.low << "," << msg.avg << "," << msg.vol << "," <<
    msg.vol_cur << "," << msg.last << "," << msg.buy << "," << msg.sell << "," << msg.updated << "," <<
    msg.server_time << "\n";
  file_handle->flush();//write to file each time (slower but better for when data is read later)
  
  
}

void check_for_new_tickers(ros::NodeHandle &n){
  //call rostopic list
  std::string cmd = "rostopic list";
  FILE * cmd_output = popen(cmd.c_str(), "r");

  //put this crap in a vector
  std::vector<std::string> topics;
  char line[128];
  while( fgets (line, sizeof line, cmd_output)){
    topics.push_back(line);
  }
  //loop through and subscribe to all of the ticker topics
  for(int i = 0; i < topics.size(); i++){
    topics[i].erase(0, topics[i].find_first_not_of('\n'));
    topics[i].erase(topics[i].find_last_not_of('\n')+1); 
    
    //all ticker topics will begin with "/ticker_"
    if(topics[i].substr(0,8) == "/ticker_"){

      //if not subscribed to this one, subscribe to it
      if(std::find(tickers_.begin(), tickers_.end(), topics[i]) == tickers_.end()){
	ROS_INFO("subscribing to %s", topics[i].c_str());
	subs_.push_back(n.subscribe(topics[i], 1, ticker_callback));
	tickers_.push_back(topics[i]);
     
	//now data will be saved from this topic, so offer service to retreieve historical data
	std::string historical_ticker_service = topics[i] + "_historical";
	ros::ServiceServer service = n.advertiseService(historical_ticker_service, request_history);
      }
    }
  }
}

int main(int argc, char** argv){
  //Node setup
  ros::init(argc, argv, "save_load_ticker_node");
  ros::NodeHandle n;//global
  ros::NodeHandle nh("~"); //local

  //TODO:make this a param
  base_file_ = "/home/bob/Documents/btce_bot/ticker_data/";

  //Subscribers
  //check to see what tickers are running, populate vector
  check_for_new_tickers(n);

  //go into loop
  ros::Rate rate(1);
  //only use one thread for callbacks for now
  ros::AsyncSpinner spinner(1);
  spinner.start();
  while(ros::ok()){
    check_for_new_tickers(n);
    rate.sleep();
  }
  spinner.stop();


}
