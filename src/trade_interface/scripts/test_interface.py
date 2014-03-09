import rospy
import std_msgs.msg
import btceapi
import csv
import time
import sys

class InterfaceNode():
    def __init__(self):
        #get params
        
    

if __name__ == '__main__':
    #intialize node and name her
    rospy.init_node('test_interface_node')

    #go to class functions
    try:
        interface_node = InterfaceNode()
    except rospy.ROSInterruptException:
        pass
