#include "ros/ros.h"
#include "mecanum_steady/location.h"
#include "std_msgs/Float64.h"
#include "std_msgs/Bool.h"
#include <algorithm>

#define ERROR_TOR 0.032
#define p_VALUE 1.7
#define q_VALUE 1.7
#define MAX_SPEED 5.0
#define START_CONST 0.8

double start_x, start_y, start_z;
double target_x, target_y, target_z;
double now_x, now_y, now_z;
void dist_cb(const mecanum_steady::location::ConstPtr& msg) {
    // start_x = msg->start_x;
    // start_y = msg->start_y;
    // start_z = msg->start_z;
    target_x = msg->x;
    target_y = msg->y;
    target_z = msg->z;
}
bool in_error() {
    if (target_x-now_x == 0 && target_y-now_y == 0 && target_z-now_z == 0) return 0;
    bool able_to_stop = true;
    able_to_stop &= (abs(target_x - now_x) < ERROR_TOR);
    able_to_stop &= (abs(target_y - now_y) < ERROR_TOR);
    able_to_stop &= (abs(target_z - now_z) < ERROR_TOR);
    if (able_to_stop) {
        start_x = now_x;
        start_y = now_y;
        start_z = now_z;
    }
    return able_to_stop;
}
void state_cb(const mecanum_steady::location::ConstPtr& msg) {
    now_x = msg->x;
    now_y = msg->y;
    now_z = msg->z;
}
double get_vel(double start, double now, double target) {
    double ret_vel;
    double p = abs(now-start)*p_VALUE + START_CONST;
    double q = abs(target-now)*q_VALUE;
    ret_vel = std::min(p, q);
    ret_vel = std::min(ret_vel, MAX_SPEED);
    if (target - now < 0) ret_vel *= (-1);
    return ret_vel;
}
int main(int argc, char** argv) {
    ros::init(argc, argv, "control");
    ros::NodeHandle nh;

    ros::Subscriber sub_dist = nh.subscribe("/setpoint", 10, dist_cb);
    ros::Subscriber sub_state = nh.subscribe("/state", 10, state_cb);
    ros::Publisher pub_next_ctl = nh.advertise<std_msgs::Bool>("/next_ctl", 10);
    ros::Publisher pub_control = nh.advertise<mecanum_steady::location>("/control_effort", 10);


    std_msgs::Bool go_next;
    mecanum_steady::location vel; 

    go_next.data = 0;
    while (ros::ok()) {
        ros::spinOnce();
        if (in_error()) {
            /* ROS_INFO("in error"); */
            go_next.data = 1;
            vel.x = 0;
            vel.y = 0;
            vel.z = 0;
            ROS_INFO("now_x %lf", now_x);
        }
        else {
            go_next.data = 0;
            /* ROS_INFO("control %lf %lf %lf", start_x, now_x, target_x); */
            /* ROS_INFO("expect %lf, %lf", (now_x-start_x)*p_VALUE, (target_x-now_x)*q_VALUE); */
            vel.x = get_vel(start_x, now_x, target_x);
            /* ROS_INFO("vel %lf", vel.x); */
            vel.y = get_vel(start_y, now_y, target_y);
            vel.z = get_vel(start_z, now_z, target_z);
        }
        pub_next_ctl.publish(go_next);
        pub_control.publish(vel);
    }
    return 0;
}
