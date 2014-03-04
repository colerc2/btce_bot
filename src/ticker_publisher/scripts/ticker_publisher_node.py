#!/usr/bin/env python
import rospy
import std_msgs.msg
from ticker_publisher.msg import ticker
from collections import deque
import btceapi
import csv
import time

def pub_ticker():
    rospy.init_node('ticker_publisher_node')

    trade_pair = rospy.get_param('~trade_pair', 'btc_usd')
    topic_name = 'ticker_' + trade_pair
    pub = rospy.Publisher(topic_name, ticker)
        
    
    #attrs = ('high', 'low', 'avg', 'vol', 'vol_cur', 'last',
     #        'buy', 'sell', 'updated', 'server_time')
    
    #make persistent connection so it doesn't have to every request
    connection = btceapi.BTCEConnection()

    #create empty ticker message
    msg = ticker()

    #while ROs is still running, publish ticker data
    while not rospy.is_shutdown():
        try:
            ticker_ = btceapi.getTicker(trade_pair, connection)
        except:
            rospy.sleep(1.0)
            connection = btceapi.BTCEConnection()
            pass

        msg.header.stamp = rospy.Time.now()
        msg.trade_pair = trade_pair
        msg.high = ticker_.high
        msg.low = ticker_.low
        msg.avg = ticker_.avg
        msg.vol = ticker_.vol
        msg.vol_cur = ticker_.vol_cur
        msg.last = ticker_.last
        msg.buy = ticker_.buy
        msg.sell = ticker_.sell
        msg.updated = "%s" % ticker_.updated
        msg.server_time = "%s" % ticker_.server_time
        pub.publish(msg)
        rospy.sleep(1.0)


if __name__ == '__main__':
    try:
        pub_ticker()
    except rospy.ROSInterruptException:
        pass
