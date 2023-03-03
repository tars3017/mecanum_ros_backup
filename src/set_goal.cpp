#include "ros/ros.h"
#include "mecanum_steady/location.h"
#include "std_msgs/Float64.h"
#include "std_msgs/Bool.h"

#define target_num 10

double target[10][5];
int idx;
void next_ctl_cb(const std_msgs::Bool::ConstPtr& msg) {
    if (msg->data) ++idx;
}
int main(int argc, char** argv) { 
    ros::init(argc, argv, "set_goal");
    ros::NodeHandle nh;

    ros::Publisher pub_dest = nh.advertise<mecanum_steady::location>("/setpoint", 10);
    ros::Subscriber sub = nh.subscribe("/next_ctl", 10, next_ctl_cb);

    mecanum_steady::location now_goal;
    while (idx < target_num && ros::ok()) {
        ros::spinOnce(); 

        now_goal.x = target[idx][0];
        now_goal.y = target[idx][1];
        now_goal.z = target[idx][2];
        pub_dest.publish(now_goal); 
    }
    return 0;
}
