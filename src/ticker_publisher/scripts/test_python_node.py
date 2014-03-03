#!/usr/bin/env python
import rospy
from std_msgs.msg import String
from ticker_publisher.msg import ticker
from collections import deque
import btceapi
import csv
import time

def pub_ticker():
    pub = rospy.Publisher('ticker', ticker)
    rospy.init_node('ticker_node')
    
    attrs = ('high', 'low', 'avg', 'vol', 'vol_cur', 'last',
             'buy', 'sell', 'updated', 'server_time')
    
    connection = btceapi.BTCEConnection()
    
    ticker_ = btceapi.getTicker("btc_usd", connection)
    last = ticker_

    msg = ticker()
    while not rospy.is_shutdown():
        try:
            ticker_ = btceapi.getTicker("btc_usd", connection)
        except:
            connection = btce_api.BTCEConnection()
            pass

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
