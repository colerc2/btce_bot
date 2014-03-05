#!/usr/bin/env python
import rospy
import std_msgs.msg
from ticker_publisher.msg import ticker
from macd_sell_signal.msg import macd
from macd_sell_signal.srv import macd_array
import numpy as np
import matplotlib
matplotlib.use('GTKAgg')
import matplotlib.pyplot as plt
from pylab import *
rcParams['figure.figsize'] = 15, 10
from collections import deque
import time
import datetime


fig=plt.figure()

def get_data_and_plot(service):
    rospy.wait_for_service(service)
    try:
        #call service
        grab_array = rospy.ServiceProxy(service, macd_array)
        macd_array_ = grab_array()
        
        #populate new arrays each time, this is so god damn inefficient, oh well
        #macd buffers
        buffer_short_emaf = deque([])
        buffer_long_emaf = deque([])
        buffer_macd = deque([])
        buffer_signal = deque([])
        buffer_macd_hist = deque([])
        #ticker buffers
        buffer_last = deque([])
        buffer_server_time = deque([])
        
        for i in range(len(macd_array_.macd_array)):
            macd_msg = macd_array_.macd_array[i]
            buffer_short_emaf.append(macd_msg.short_emaf)
            buffer_long_emaf.append(macd_msg.long_emaf)
            buffer_macd.append(macd_msg.macd)
            buffer_signal.append(macd_msg.signal)
            buffer_macd_hist.append(macd_msg.macd_hist)
            tmp_date = datetime.datetime.strptime(macd_msg.tick.server_time, "%Y-%m-%d %H:%M:%S")
            buffer_server_time.append(tmp_date)
            buffer_last.append(macd_msg.tick.last)

        #plot her
        plt.clf()
        if(len(macd_array_.macd_array) > 0):
            plt.subplot(211)
            p1 = plt.plot(buffer_server_time,buffer_last,'g',linewidth=4.0)
            p2 = plt.plot(buffer_server_time,buffer_short_emaf,'b',linewidth=3.0)
            p3 = plt.plot(buffer_server_time,buffer_long_emaf,'r',linewidth=2.0)
            plt.xlabel('Time')
            plt.ylabel('Price ($)')
            plt.grid(True)
            plt.legend(['Last', '12-period EMAF', '26-period EMAF'], loc=2)
            min_y = min([min(buffer_short_emaf), min(buffer_long_emaf), min(buffer_last)])
            max_y = max([max(buffer_long_emaf), max(buffer_short_emaf), max(buffer_last)])
            #next 3 lines are for pretty
            range_y = 0.1*(max_y-min_y)
            min_y -= range_y
            max_y += range_y
            plt.ylim([min_y, max_y])
            plt.gcf().autofmt_xdate()
            plt.gca().get_yaxis().get_major_formatter().set_useOffset(False)
            plt.subplot(212)
            plt.plot(buffer_server_time,buffer_macd,'b',linewidth=2.0)
            plt.plot(buffer_server_time,buffer_signal,'r',linewidth=2.0)
            plt.plot(buffer_server_time,buffer_macd_hist,'k',linewidth=4.0)
            plt.xlabel('Time');
            plt.ylabel('wat');
            plt.grid(True)
            plt.legend(['MACD', 'Signal', 'Histogram'], loc=2)
            min_y = max([abs(min(buffer_macd)), abs(min(buffer_signal)), abs(min(buffer_macd_hist))])
            max_y = max([max(buffer_macd), max(buffer_signal), max(buffer_macd_hist), min_y])*1.1
            plt.ylim([-max_y, max_y])
            plt.gcf().autofmt_xdate()
            fig = plt.gcf()
            fig.set_size_inches(15,10)
            plt.draw()
            time.sleep(0.1)

            
            #rospy.loginfo(macd_msg)
    except rospy.ServiceException, e:
            print "Service call failed: %s" % e

def plot():
    plt.ion()
    plt.show()
    #get params
    macd_array_service = rospy.get_param('~macd_array_service', 'error')
    while 1:
        get_data_and_plot(macd_array_service)
        time.sleep(5.0)
    


if __name__ == '__main__':
    #Initialize node and name it
    rospy.init_node('macd_plot_node')
    #Go to main loop
    plot()
