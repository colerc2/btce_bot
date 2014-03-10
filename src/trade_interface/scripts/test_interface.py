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
        key_file = "/home/bob/Documents/btce_bot/trade_api.key"
        handler = btceapi.KeyHandler(key_file, resaveOnDeletion=True)
        conn = btceapi.BTCEConnection()
        topic_names_init = False
        topics = deque([])
        msg = get_info()

        while not rospy.is_shutdown():
            rospy.sleep(5.0)
            for key in handler.getKeys():
                print "Printing info for key %s" % key
                
            # NOTE: In future versions, the handler argument will be required.
                t = btceapi.TradeAPI(key, handler=handler)
                
                try:
                    r = t.getInfo(connection = conn)
                    
                    counter = 0
                    for currency in btceapi.all_currencies:
                        if (topic_names_init == False):
                            topic_name = 'get_info/' + currency.upper()
                            topics.append(rospy.Publisher(topic_name, get_info))
                                          
                        balance = getattr(r, "balance_" + currency)
                        msg.coin = currency.upper()
                        msg.balance = balance
                        test_string = "%r" % r.server_time
                        msg.server_time = r.server_time.strftime("%Y-%m-%d %H:%M:%S")
                        topics[counter].publish(msg)
                        counter = counter + 1
                                          #print "\t%s balance: %s" % (currency.upper(), balance)
                        #print "\tInformation rights: %r" % r.info_rights
                        #print "\tTrading rights: %r" % r.trade_rights
                        #print "\tWithrawal rights: %r" % r.withdraw_rights
                        #print "\tServer time: %r" % r.server_time
                        #print "\tItems in transaction history: %r" % r.transaction_count
                        #print "\tNumber of open orders: %r" % r.open_orders
                                          
                    topic_names_init = True
                                             
                except Exception as e:
                    print "  An error occurred: %s" % e
                    rospy.sleep(5.0)

                    conn = btceapi.BTCEConnection()
                    pass
                                          
#raise e
                

if __name__ == '__main__':
    #intialize node and name her
    rospy.init_node('test_interface_node')

    #go to class functions
    try:
        interface_node = InterfaceNode()
    except rospy.ROSInterruptException:
        pass
