#!/usr/bin/env python
import rospy
from std_msgs.msg import String
from collections import deque
import btceapi
import csv
import time

def talker():
    pub = rospy.Publisher('chatter', String)
    rospy.init_node('talker')
    
    attrs = ('high', 'low', 'avg', 'vol', 'vol_cur', 'last',
             'buy', 'sell', 'updated', 'server_time')
    
    connection = btceapi.BTCEConnection()
    
    ticker = btceapi.getTicker("btc_usd", connection)
    last = ticker

    while not rospy.is_shutdown():
        try:
            ticker = btceapi.getTicker("btc_usd", connection)
        except:
            connection = btce_api.BTCEConnection()
            pass

        str = "%s" % ticker.last
        #str = "%s" % rospy.get_time()
        rospy.loginfo(str)
        pub.publish(String(str))
        rospy.sleep(1.0)


if __name__ == '__main__':
    try:
        talker()
    except rospy.ROSInterruptException:
        pass
