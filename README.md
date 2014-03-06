btce_bot
========

BTC-E trading bot written in C++/Python using ROS. All BTC-e API was taken from [the following repo](https://github.com/alanmcintyre/btce-api). This bot uses multiple MACD filters along with other hueristics to identify optimal sell and buy times and execute trades using BTC-e's API. Can also be used to collect raw ticker data from BTC-e for post processing. Each of the packages in the [src](src) directory are explained in detail inside their respective directories.

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
  rosrun macd_sell_signal macd_sell_signal_node _trade_pair:="btc_usd" _short:="11" _long:="25" _sig:="8" 
  _period:="10" _num_old_periods:="15" _spread_window:="40" _spread_value:="1.25"
   
