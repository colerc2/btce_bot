#!/usr/bin/python
import rospy
import roslib
import std_msgs.msg
import btceapi
import csv
import time
import sys
import smtplib
#from trade_interface.msg import trans_history
#from trade_interface.msg import active_orders
#from trade_interface.srv import *
#from cell_phone_interface.msg import send_text_msg
from cell_phone_interface.srv import *
from collections import deque
import pylab

class CellInterfaceNode():
    def __init__(self):
        #get params
        self.email = rospy.get_param('~email', 'error')
        password_file = rospy.get_param('~password_file', 'error')
        self.phone_number = rospy.get_param('~phone_number', 'error')
        f = open(password_file, 'r')
        self.password = f.readline()
        
        self.email_server = smtplib.SMTP( "smtp.gmail.com", 587)
        self.email_server.starttls()
        self.email_server.login( self.email, self.password)
        
        
        #intiate a bunch of services
        send_text_service = rospy.Service('send_text_service', send_text_msg, self.send_text)
        rospy.spin()
        
    def send_text(self, req):
        res = send_text_msgResponse()
        #for i in range(50):
        self.email_server.sendmail(self.email, self.phone_number, req.text_msg)
         #   time.sleep(0.2)
                                   #print self.email

        return res

if __name__ == '__main__':
    #intialize node and name her
    rospy.init_node('cell_phone_interface_node')

    #go to class functions
    try:
        interface_node = CellInterfaceNode()
    except rospy.ROSInterruptException:
        pass
