#include "ros/ros.h"
#include "mecanum_steady/location.h"
#include "std_msgs/Float64.h"
#include "std_msgs/Bool.h"

void dist_cb(const mecanum_stead::location::ConstPtr& msg) {

}
void state_cb(const mecanum_stead::location::ConstPtr& msg) {

}
int main(int argc, char** argv) {
    ros::init(argc, argv, "control");
    ros::NodeHandle nh;

    ros::Subscriber sub_dist = nh.subscribe("/setpoint", 10, dist_cb);
    ros::Subscriber sub_state = nh.subscribe("/state", 10, state_cb);
    ros::Publisher pub_next_ctl = nh.advertise<std_msgs::Bool>("/next_ctl", 10);
    ros::Publisher pub_control = nh.advertise<std_msgs::Float64>("/control_effort", 10);

    while (ros::ok()) {
        // TODO
    }
    return 0;
}
