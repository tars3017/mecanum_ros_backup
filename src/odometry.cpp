#include "ros/ros.h"
// #include "mecanum_steady/location.h"
#include "std_msgs/Float64.h"
#include "std_msgs/Bool.h"
#include "geometry_msgs/Twist.h"
#include "geometry_msgs/Point.h"
#include <algorithm>

double last_time, now_time;
bool test_mode, debug_get_vel;
double now_x_pos, now_y_pos, now_z_pos;
double base_now_x_vel, base_now_y_vel, base_now_z_vel;
double base_last_x_vel, base_last_y_vel, base_last_z_vel;

ros::Subscriber sub_base_vel;
ros::Publisher pub_odom;

// for test_mode use
ros::Subscriber sub_navi_vel;
geometry_msgs::Point now_pt;
geometry_msgs::Twist vel;

void navi_cb(const geometry_msgs::Twist::ConstPtr& msg) {
    vel.linear.x = msg->linear.x;
    vel.linear.y = msg->linear.y;
    vel.angular.z = msg->angular.z;
}

void base_cb(const geometry_msgs::Twist::ConstPtr& msg = nullptr) {
    if (!test_mode && msg != nullptr) {
        base_now_x_vel = msg->linear.x;
        base_now_y_vel = msg->linear.y;
        base_now_z_vel = msg->angular.z;
    }
    else {
        base_now_x_vel = vel.linear.x;
        base_now_y_vel = vel.linear.y;
        base_now_z_vel = vel.angular.z;
    }
    if (debug_get_vel && (base_now_x_vel != 0 || base_now_y_vel != 0 || base_now_z_vel != 0)) {
        ROS_INFO("Get base speed %lf %lf %lf", base_now_x_vel, base_now_y_vel, base_now_z_vel);
    }
}

void cal_pose() {
    now_time = ros::Time::now().toSec();
    now_x_pos += (base_last_x_vel + base_now_x_vel) / 2 * (now_time - last_time);
    now_y_pos += (base_last_y_vel + base_now_y_vel) / 2 * (now_time - last_time);
    now_z_pos += (base_last_z_vel + base_now_z_vel) / 2 * (now_time - last_time);

    now_pt.x = now_x_pos;
    now_pt.y = now_y_pos;
    now_pt.z = now_z_pos;
    
    last_time = ros::Time::now().toSec();
    base_last_x_vel = base_now_x_vel;
    base_last_y_vel = base_now_y_vel;
    base_last_z_vel = base_now_z_vel;

}

int main(int argc, char** argv) {

    ros::init(argc, argv, "odometry");
    ros::NodeHandle nh;
    nh.getParam("debug_get_vel", debug_get_vel);
    nh.getParam("test_mode", test_mode);

    sub_base_vel = nh.subscribe("/base_speed", 1, base_cb);
    pub_odom = nh.advertise<geometry_msgs::Point>("/odom", 1);
    
    // for test_mode use
    sub_navi_vel = nh.subscribe("/cmd_vel", 1, navi_cb);
    
    last_time = ros::Time::now().toSec();
    while (ros::ok()) {
        ros::spinOnce();
        if (test_mode) {
            base_cb();
        }
        cal_pose();
        pub_odom.publish(now_pt);
    }
    return 0;
}
