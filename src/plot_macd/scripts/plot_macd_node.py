#!/usr/bin/env python
import rospy
import std_msgs.msg
from ticker_publisher.msg import ticker
from macd_sell_signal.msg import macd
import numpy as np
import matplotlib
matplotlib.use('GTK')
import matplotlib.pyplot as plt
from collections import deque
import time

buffer_short_emaf = deque([])
fig=plt.figure()

def callback(data):
    #rospy.loginfo(rospy.get_name() + "I heard %f", data.short_emaf)
    global buffer_short_emaf
    if (len(buffer_short_emaf) > 1000):
        buffer_short_emaf.popleft()
    buffer_short_emaf.append(data.short_emaf)
    #print buffer_short_emaf
    if (len(buffer_short_emaf) > 1):
        plt.clf()
        x_axis = range(len(buffer_short_emaf))
        min_x = min(x_axis)
        max_x = max(x_axis)
        min_y = min(buffer_short_emaf)-0.5
        max_y = max(buffer_short_emaf)+0.5
        plt.plot(x_axis,buffer_short_emaf,linewidth=2.0)
        plt.axis([min_x, max_x, min_y, max_y])
        plt.draw()
        time.sleep(0.1)


def plot():
    plt.ion()
    plt.show()
    #get params
    macd_topic = rospy.get_param('~macd_topic', 'error')
    #create subscriber
    rospy.Subscriber(macd_topic, macd, callback)
    #wait for new messages
    rospy.spin()
    


if __name__ == '__main__':
    #Initialize node and name it
    rospy.init_node('macd_plot_node')
    #Go to main loop
    plot()
