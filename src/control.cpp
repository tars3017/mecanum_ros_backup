#include "ros/ros.h"
#include "mecanum_steady/location.h"
#include "std_msgs/Float64.h"
#include "std_msgs/Bool.h"

#define ERROR_TOR 0.05
#define p_VALUE 0.3
#define q_VALUE 0.3
#define MAX_SPEED 70

double start_x, start_y, start_z;
double target_x, target_y, target_z;
double now_x, now_y, now_z;
void dist_cb(const mecanum_stead::location::ConstPtr& msg) {
    start_x = msg->x;
    start_y = msg->y;
    start_z = msg->z;
    target_x = msg->target_x;
    target_y = msg->target_y;
    target_z = msg->target_z;
}
bool error() {
    bool able_to_stop = true;
    able_to_stop &= (abs(target_x - now_x) < ERROR_TOR);
    able_to_stop &= (abs(target_y - now_y) < ERROR_TOR);
    able_to_stop &= (abs(target_z - now_z) < ERROR_TOR);
    return able_to_stop;
}
void state_cb(const mecanum_stead::location::ConstPtr& msg) {
    now_x = msg->x;
    now_y = msg->y;
    now_z = msg->z;
}
double get_vel(double start, double now, double target) {
    double ret_vel;
    if (abs(now-start)*p_VALUE < abs(target-now)*q_VALUE) {
        ret_vel = (now - start) * p_VALUE;
    }
    else {
        ret_vel = (target - now) * q_VALUE;
    }

    if (abs(ret_vel) > MAX_SPEED) {
        ret_vel = (ret_vel > 0 ? MAX_SPEED : -MAX_SPEED);
    }
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
    mecanum::steady::location vel; 

    go_next.data = 0;
    while (ros::ok()) {
        ros::spinOnce();
        if (in_error()) {
            go_next.data = 1;
            vel.x = 0;
            vel.y = 0;
            vel.z = 0;
        }
        else {
            go_next.data = 0;
            vel.x = get_vel(start_x, now_x, target_x);
            vel.y = get_vel(start_y, now_y, target_y);
            vel.z = get_vel(start_z, now_z, target_z);
        }
        pub_next_ctl.publish(go_next);
        pub_control.publish(vel);
    }
    return 0;
}
