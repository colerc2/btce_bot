btce_bot
========

BTC-E trading bot written in C++/Python using ROS

All BTC-e API was taken from https://github.com/alanmcintyre/btce-api

A list of each of the packages in /src and a brief description

    publish_ticker - This package grabs ticker data from the btc-e site and publishes to a topic based on what the trade pair is. For example, the trade pair BTC/USD will publish on the topic /ticker_btc_usd. The message published is a custom message defined in /src/publish_ticker/ticker.msg.
