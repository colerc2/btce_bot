btce_bot
========

BTC-E trading bot written in C++/Python using ROS. All BTC-e API was taken from [this repo](https://github.com/alanmcintyre/btce-api). This bot uses multiple MACD filters along with other hueristics to identify optimal sell and buy times and execute trades using BTC-e's API. Can also be used to collect raw ticker data from BTC-e for post processing. Each of the packages in the [src](src) directory are explained in detail inside their respective directories.

###Install

- You'll first have to install the python BTC-e API (link to repo above)  
- Install [ROS](http://www.ros.org/install/)  
  - Project was written on Ubuntu 12.04 using ROS Hydro
- cd to btce_bot directory  
  - Run *catkin_make* command
  - Run *source devel/setup.bash* command

###Examples
- To launch the ticker/data collector run *roslaunch launch/ticker.launch*  
  - This will publish the following trade pairs to topics: btc_usd, btc_ltc, ltc_usd  
  - To change these values simple add another node in the [ticker.launch](launch/ticker.launch) file
- To launch a MACD node do one of the following:  
  - Run the command *roslaunch launch/lots_of_macd.launch*  
    - This will launch a bunch of MACD nodes along with the [sell_signal_filter_node](src/sell_signal_filter). You can get a list of the current nodes by running *rosnode list*.  
  - Manually start a node with rosrun, here's an example:
```
rosrun macd_sell_signal macd_sell_signal_node _trade_pair:="btc_usd" _short:="11" _long:="25" _sig:="8" 
_period:="10" _num_old_periods:="15" _spread_window:="40" _spread_value:="1.25"
```
- To plot MACD/ticker values for a certain MACD node, run the following (change values of macd accordingly):
```
rosrun plot_macd plot_macd_node.py _macd_array_service:="macd_11_25_8_x10/macd_array"
```

####Known Issues
- (Might have fixed it, haven't seen it since March 6th) When ticker_publisher nodes are running and save_load ticker node is running too, after they've been running for a very long time(>1 hour) if a new macd_sell_signal node is started, the service that requests the history will fail, causing the save_load_ticker node to crash although the new MACD node starts up without any issues, it just has no history, so the filter has to settle down 
- (Only saw this happen once, March 8th) When the user_interface_node initializes, it makes a call to the /sell_history service so it can load all the sells and display them to the user. That call to that service failed, still trying to figure this one out.
