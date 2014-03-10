#!/usr/bin/python
import rospy
import roslib
import std_msgs.msg
import btceapi
import csv
import time
import sys
from trade_interface.msg import get_info
from collections import deque

class InterfaceNode():
    def __init__(self):
        #get params
        #TODO: make this a param
        self.key_file = "/home/bob/Documents/btce_bot/trade_api.key"
        self.handler = btceapi.KeyHandler(self.key_file, resaveOnDeletion=True)
        self.conn = btceapi.BTCEConnection()
        self.topic_names_init = False
        self.topics = deque([])
        self.msg = get_info()

        while not rospy.is_shutdown():
            rospy.sleep(5.0)
            self.call_get_info()
            
            
    def call_get_info(self):
        for key in self.handler.getKeys():
            print "Printing info for key %s" % key
                
            # NOTE: In future versions, the handler argument will be required.
            t = btceapi.TradeAPI(key, handler=self.handler)
                
            try:
                r = t.getInfo(connection = self.conn)
                
                counter = 0
                for currency in btceapi.all_currencies:
                    if (self.topic_names_init == False):
                        topic_name = 'trade_interface/get_info/' + currency
                        self.topics.append(rospy.Publisher(topic_name, get_info))
                        
                    balance = getattr(r, "balance_" + currency)
                    self.msg.coin = currency
                    self.msg.balance = balance
                    #test_string = "%r" % r.server_time
                    self.msg.server_time = r.server_time.strftime("%Y-%m-%d %H:%M:%S")
                    self.topics[counter].publish(self.msg)
                    counter = counter + 1
                        
                self.topic_names_init = True
                                             
            except Exception as e:
                print "  An error occurred: %s" % e
                rospy.sleep(5.0)

                self.conn = btceapi.BTCEConnection()
                pass

                  
if __name__ == '__main__':
    #intialize node and name her
    rospy.init_node('test_interface_node')

    #go to class functions
    try:
        interface_node = InterfaceNode()
    except rospy.ROSInterruptException:
        pass
