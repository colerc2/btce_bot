btce_bot
========

BTC-E trading bot written in C++/Python using ROS. All BTC-e API was taken from [the following repo](https://github.com/alanmcintyre/btce-api). This bot uses multiple MACD filters along with other hueristics to identify optimal sell and buy times and execute trades using BTC-e's API. While it's running, it also saves ticker data to a csv file at a rate of ~1Hz.

###Install

-You'll first have to install the python BTC-e API (link to repo above).
-Install [ROS](http://www.ros.org/install/)
  - Project was written on Ubuntu 12.04 using ROS Hydro
-cd to btce_bot directory
  - Run "catkin_make" command
  - Run "source devel/setup.bash" command
