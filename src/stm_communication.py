#!/usr/bin/env python3
#coding:utf-8

import rospy
from std_msgs.msg import Int64

def publisher():
    rospy.init_node('talker')
    pub = rospy.Publisher('/counting', Int64, queue_size = 1)
    count = 0
    while not rospy.is_shutdown():
            print("publshed", count)
            pub.publish(count)
            rospy.sleep(0.5)
            count+=1
    
if __name__=='__main__':
    try:
        publisher()
    except rospy.ROSInterruptException:
        pass
