#!/usr/bin/python
import rospy
import roslib
import std_msgs.msg
import btceapi
import csv
import time
import sys

class InterfaceNode():
    def __init__(self):
        #get params
        #TODO: make this a param
        key_file = "/home/bob/Documents/btce_bot/trade_api.key"
        handler = btceapi.KeyHandler(key_file, resaveOnDeletion=True)
        conn = btceapi.BTCEConnection()

        while not rospy.is_shutdown():
            rospy.sleep(2.0)
            for key in handler.getKeys():
                print "Printing info for key %s" % key
                
            # NOTE: In future versions, the handler argument will be required.
                t = btceapi.TradeAPI(key, handler=handler)
                
                try:
                    r = t.getInfo(connection = conn)
                    
                    for currency in btceapi.all_currencies:
                        balance = getattr(r, "balance_" + currency)
                        print "\t%s balance: %s" % (currency.upper(), balance)
                        print "\tInformation rights: %r" % r.info_rights
                        print "\tTrading rights: %r" % r.trade_rights
                        print "\tWithrawal rights: %r" % r.withdraw_rights
                        print "\tServer time: %r" % r.server_time
                        print "\tItems in transaction history: %r" % r.transaction_count
                        print "\tNumber of open orders: %r" % r.open_orders
                        
                except Exception as e:
                    print "  An error occurred: %s" % e
                    rospy.sleep(2.0)

                    conn = btceapt.BTCEConnection()
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
