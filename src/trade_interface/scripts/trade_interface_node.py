#!/usr/bin/python
import rospy
import roslib
import std_msgs.msg
import btceapi
import csv
import time
import sys
from trade_interface.msg import get_info
from trade_interface.msg import trans_history
from trade_interface.msg import active_orders
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
        trans_history_service = rospy.Service('trans_history_service', trans_history_all, self.call_trans_history)
        active_orders_service = rospy.Service('active_orders_service', active_orders_all, self.call_active_orders)
        cancel_order_service = rospy.Service('cancel_order_service', cancel_order, self.call_cancel_order)
        make_trade_service = rospy.Service('make_trade_service', make_trade, self.call_make_trade)
        #trade_history_service = rospy.Service('trade_history_service', trade_history_all, self.call_trade_history)
        rospy.spin()
        
        #while not rospy.is_shutdown():
         #   self.call_get_info()
            #TODO:make next call a service
            #self.call_trans_history()
            #TODO:do i even need to use the next call?
            #self.call_trade_history()
         #   self.call_order_list()
         #   rospy.sleep(5.0)

    def call_make_trade(self, req):
        res = make_tradeResponse()
        for key in self.handler.getKeys():
            t = btceapi.TradeAPI(key, self.handler)
            try:
                r = t.trade(req.pair, req.buy_or_sell, req.price, req.amount, self.conn)
                res.order_id = r.order_id
                res.received = r.received
            except Exception as e:
                print "  An error occurred: %s" % e
                rospy.sleep(5.0)
                
                self.conn = btceapi.BTCEConnection()
                pass
        return res
        
    def call_cancel_order(self, req):
        res = cancel_orderResponse()
        for key in self.handler.getKeys():
            t = btceapi.TradeAPI(key, self.handler)
            try:
                t.cancelOrder(req.order_id);
                res.completed = True
            except Exception as e:
                res.completed = False
                print "  An error occurred: %s" % e
                rospy.sleep(5.0)
                
                self.conn = btceapi.BTCEConnection()
                pass
        return res
            

    def call_active_orders(self, req):
        res = active_orders_allResponse()
        for key in self.handler.getKeys():
            t = btceapi.TradeAPI(key, self.handler)
            try:
                orders = t.activeOrders(connection = self.conn)
                if orders:
                    for o in orders:
                        msg = active_orders()
                        msg.order_id = int(o.order_id)
                        msg.type = o.type.encode('ascii')
                        msg.pair = o.pair.encode('ascii')
                        msg.rate = float(o.rate)
                        msg.amount = float(o.amount)
                        msg.timestamp_created = o.timestamp_created.strftime("%Y-%m-%d %H:%M:%S")
                        msg.status = int(o.status)
                        
                        res.orders.append(msg)
                else:
                    print "\t\tno orders"
            except Exception as e:
                print "  An error occurred: %s" % e
                rospy.sleep(5.0)
                
                self.conn = btceapi.BTCEConnection()
                pass
        return res
            
    

    def call_trade_history(self, pair):
        history = btceapi.getTradeHistory(pair)
        
        print len(history)
        
        pylab.plot([t.date for t in history if t.trade_type == u'ask'],
                   [t.price for t in history if t.trade_type == u'ask'], 'ro')
        
        pylab.plot([t.date for t in history if t.trade_type == u'bid'],
                   [t.price for t in history if t.trade_type == u'bid'], 'go')
        
        pylab.grid()          
        pylab.show()

            
    def call_trans_history(self, req):
        res = trans_history_allResponse()
        
        for key in self.handler.getKeys():
            t = btceapi.TradeAPI(key, handler=self.handler)
            
            try:
                th = t.transHistory()
                for h in th:
                    msg = trans_history()
                    msg.transaction_id = int(h.transaction_id)
                    msg.type = int(h.type)
                    msg.amount = float(h.amount)
                    msg.currency = h.currency.encode('ascii')
                    msg.desc = h.desc.encode('ascii', 'ignore')
                    msg.status = int(h.status)
                    msg.timestamp = h.timestamp.strftime("%Y-%m-%d %H:%M:%S")
                    res.trans_hist_array.append(msg)

            except Exception as e:
                print "  An error occurred: %s" % e
                print h.currency
                print h.desc
                rospy.sleep(5.0)
                
                #self.conn = btceapi.BTCEConnection()
                pass
        return res
                
    def call_get_info(self, req):
        res = get_info_allResponse()

        for key in self.handler.getKeys():
            # NOTE: In future versions, the handler argument will be required.
            t = btceapi.TradeAPI(key, handler=self.handler)
                
            try:
                r = t.getInfo(connection = self.conn)
                res.transaction_count = r.transaction_count
                res.open_orders = r.open_orders
                res.server_time = r.server_time.strftime("%Y-%m-%d %H:%M:%S")
                counter = 0
                for currency in btceapi.all_currencies:
                    msg = get_info()
                    if (self.topic_names_init == False):
                        topic_name = 'trade_interface/get_info/' + currency
                        self.topics.append(rospy.Publisher(topic_name, get_info))
                        
                    balance = getattr(r, "balance_" + currency)
                    msg.coin = currency
                    msg.balance = balance
                    #msg.server_time = r.server_time.strftime("%Y-%m-%d %H:%M:%S")
                    res.info.append(msg)
                    counter = counter + 1
                        
                self.topic_names_init = True
                                             
            except Exception as e:
                print "  An error occurred: %s" % e
                rospy.sleep(5.0)

                self.conn = btceapi.BTCEConnection()
                pass
        return res

                  
if __name__ == '__main__':
    #intialize node and name her
    rospy.init_node('test_interface_node')

    #go to class functions
    try:
        interface_node = InterfaceNode()
    except rospy.ROSInterruptException:
        pass
