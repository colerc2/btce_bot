btce_bot
========

BTC-E trading bot written in C++/Python using ROS. All BTC-e API was taken from [the following repo](https://github.com/alanmcintyre/btce-api). Below is a list of each of the packages in /src and a brief description.

#### [ticker_publisher](src/ticker_publisher) 
This package grabs ticker data from the btc-e site and publishes to a topic based on what the trade pair is. For example, the trade pair BTC/USD will publish on the topic /ticker_btc_usd. The message published is a custom message defined [here](src/publish_ticker/msg/ticker.msg).
- Publishers  
  - /ticker_trade_pair - e.g. /ticker_btc_usd
- Subscribers - None
    
#### [macd_sell_signal](src/macd_sell_signal)
This package subscribes to a ticker topic and publishes a sell signal which is more or less a boolean along with macd values at the current time. It utilizes a C++ object called macd_indicator. This object is used to handle the MACD calculations. This package also provides a service that allows a node to ask for previous macd values. This is mostly used so the plotting node can ask for historical data and plot at it's leisure instead of publishing all of this data to a topic.
- Publishers  
  - /macd_short_long_sig_xperiod/macd   -   e.g. macd_12_26_9_x10/macd
  - /macd_short_long_sig_xperiod/sell   -   e.g. macd_12_26_9_x10/sell
- Subscribers
  - /ticker_trade_pair
- Services
  - /macd_short_long_sig_xperiod/macd_array -   e.g. macd_12_26_9_x10/macd_array
