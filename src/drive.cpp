#include "ros/ros.h"
#include "mecanum_steady/location.h"
#include "std_msgs/Float64.h"
#include "std_msgs/Bool.h"

void control_cb(const std_msgs::Float64::ConstPtr& msg) {

}
int main(int argc, char** argv) {
    ros::init(argc, argv, "drive");
    ros::NodeHandle nh;

    ros::Subscriber sub_control = nh.subscribe("/control_effort", 10, control_cb);
    ros::Publisher pub_state = nh.advertise<mecanum_steady::location>("/state", 10);

    while (ros::ok()) {
        // TODO
    }
    return 0;
}
