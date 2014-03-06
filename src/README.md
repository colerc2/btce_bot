A brief description of each of the packages, for more information, please check the README.md in the package.

#### [ticker_publisher](ticker_publisher) 
This package grabs ticker data from the btc-e site and publishes to a topic based on what the trade pair is. For example, the trade pair BTC/USD will publish on the topic /ticker_btc_usd.
- Publishers  
  - [/ticker_trade_pair](ticker_publisher/msg/ticker.msg) - e.g. /ticker_btc_usd
- Subscribers - None
    
#### [macd_sell_signal](macd_sell_signal)
This package subscribes to a ticker topic and publishes a sell signal which is a boolean along with macd values at the current time(TODO). It utilizes a C++ object called [macd_indicator](macd_sell_signal/src/macd_indicator.cpp). This object is used to handle the MACD calculations. This package also provides a service that allows a node to ask for previous macd values. This is mostly used so the plotting node can ask for historical data and plot at it's leisure instead of publishing all of this data to a topic.
- Publishers  
  - [/macd_short_long_sig_xperiod/macd](macd_sell_signal/msg/macd.msg)   -   e.g. macd_12_26_9_x10/macd
  - [/macd_short_long_sig_xperiod/sell](macd_sell_signal/msg/sell.msg)   -   e.g. macd_12_26_9_x10/sell
- Subscribers
  - [/ticker_trade_pair](ticker_publisher/msg/ticker.msg)
- Services
  - [/macd_short_long_sig_xperiod/macd_array](macd_sell_signal/srv/macd_array.srv) -   e.g. macd_12_26_9_x10/macd_array
