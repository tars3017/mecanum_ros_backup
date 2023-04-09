/** stm_communication.cpp **/
#include <iostream>
#include "ros/ros.h"
#include <std_msgs/Int64.h>

using namespace std;

int main(int argc, char** argv){
    ros::init(argc, argv, "stm_communication");
    ros::NodeHandle nh;

    ros::Publisher pub = nh.advertise<std_msgs::Int64>("counting", 1);

    std_msgs::Int64 hello;
    hello.data = 0;

    while(ros::ok()){
        pub.publish(hello);
        hello.data++;
        ros::Duration(0.5).sleep();
	    std::cout << "count=" << hello.data << std::endl;
    }

    ROS_INFO("== Terminate ! ==");
    return 0;
}
