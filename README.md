btce_bot
========

BTC-E trading bot written in C++/Python using ROS. All BTC-e API was taken from [the following repo](https://github.com/alanmcintyre/btce-api). This bot uses multiple MACD filters along with other hueristics to identify optimal sell and buy times and execute trades using BTC-e's API. Can also be used to collect raw ticker data from BTC-e for post processing. Each of the packages in the [src](src) directory are explained in detail inside their respective directories.

###Install

-You'll first have to install the python BTC-e API (link to repo above)  
-Install [ROS](http://www.ros.org/install/)  
  - Project was written on Ubuntu 12.04 using ROS Hydro  
-cd to btce_bot directory  
  - Run "catkin_make" command
  - Run "source devel/setup.bash" command
