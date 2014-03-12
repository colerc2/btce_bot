#!/usr/bin/python
import rospy
import roslib
import std_msgs.msg
import btceapi
import csv
import time
import sys
from trade_interface.msg import get_info
from trade_interface.srv import *
from collections import deque
import pylab

class InterfaceNode():
    def __init__(self):
        #get params
        #TODO: make this a param
        self.key_file = "/home/bob/Documents/btce_bot/trade_api.key"
        self.handler = btceapi.KeyHandler(self.key_file, resaveOnDeletion=True)
        self.conn = btceapi.BTCEConnection()
        self.topic_names_init = False
        self.topics = deque([])
 #       self.msg = get_info()
        
        #intiate a bunch of services
        get_info_service = rospy.Service('get_info_service', get_info_all, self.call_get_info)
        rospy.spin()
        
        #while not rospy.is_shutdown():
         #   self.call_get_info()
            #TODO:make next call a service
            #self.call_trans_history()
            #TODO:do i even need to use the next call?
            #self.call_trade_history()
         #   self.call_order_list()
         #   rospy.sleep(5.0)


    def call_order_list(self):
        for key in self.handler.getKeys():
            t = btceapi.TradeAPI(key, self.handler)
            try:
                orders = t.activeOrders(connection = self.conn)
                if orders:
                    for o in orders:
                        print "\t\t order id: %r" % o.order_id
                        print "\t\t type: %s" % o.type
                        print "\t\t pair: %s" % o.pair
                        print "\t\t rate: %s" % o.rate
                        print "\t\t amount: %s" % o.amount
                        print "\t\t created: %r" % o.timestamp_created
                        print "\t\t status: %r" % o.status
                        print
                else:
                    print "\t\tno orders"
            except Exception as e:
                print "  An error occurred: %s" % e
                rospy.sleep(5.0)
                
                self.conn = btceapi.BTCEConnection()
                pass
            
    

    def call_trade_history(self, pair):
        history = btceapi.getTradeHistory(pair)
        
        print len(history)
        
        pylab.plot([t.date for t in history if t.trade_type == u'ask'],
                   [t.price for t in history if t.trade_type == u'ask'], 'ro')
        
        pylab.plot([t.date for t in history if t.trade_type == u'bid'],
                   [t.price for t in history if t.trade_type == u'bid'], 'go')
        
        pylab.grid()          
        pylab.show()

            
    def call_trans_history(self):
        for key in self.handler.getKeys():
            t = btceapi.TradeAPI(key, handler=self.handler)
            
            try:
                th = t.transHistory()
                for h in th:
                    print "\t\t        id: %r" % h.transaction_id
                    print "\t\t      type: %r" % h.type
                    print "\t\t    amount: %r" % h.amount
                    print "\t\t  currency: %r" % h.currency
                    print "\t\t      desc: %s" % h.desc
                    print "\t\t    status: %r" % h.status
                    print "\t\t timestamp: %r" % h.timestamp
                    print
            except:
                print "  An error occurred: %s" % e
                rospy.sleep(5.0)
                
                self.conn = btceapi.BTCEConnection()
                pass
                
    def call_get_info(self, req):
        res = get_info_allResponse()


#        print get_info_all.info
        print "-----------------------\n"
        #for property, value in vars(res).iteritems():
        #    print property, ": ", value
        #[name for name,thing in res.getmembers([])]
        print "-----------------------\n"

        for key in self.handler.getKeys():
            #print "Printing info for key %s" % key
                
            # NOTE: In future versions, the handler argument will be required.
            t = btceapi.TradeAPI(key, handler=self.handler)
                
            try:
                r = t.getInfo(connection = self.conn)
                
                counter = 0
                for currency in btceapi.all_currencies:
                    msg = get_info()
                    if (self.topic_names_init == False):
                        topic_name = 'trade_interface/get_info/' + currency
                        self.topics.append(rospy.Publisher(topic_name, get_info))
                        
                    balance = getattr(r, "balance_" + currency)
                    msg.coin = currency
                    msg.balance = balance
                    #test_string = "%r" % r.server_time
                    msg.server_time = r.server_time.strftime("%Y-%m-%d %H:%M:%S")
                    res.info.append(msg)
                    #res.append(self.msg)
#self.topics[counter].publish(self.msg)
                    counter = counter + 1
                        
                self.topic_names_init = True
                                             
            except Exception as e:
                print "  An error occurred: %s" % e
                rospy.sleep(5.0)

                self.conn = btceapi.BTCEConnection()
                pass
               #get_info_
            #tester = get_info_allResponse()
            #tester.info = response
        return res

                  
if __name__ == '__main__':
    #intialize node and name her
    rospy.init_node('test_interface_node')

    #go to class functions
    try:
        interface_node = InterfaceNode()
    except rospy.ROSInterruptException:
        pass
