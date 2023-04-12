#include "ros/ros.h"
// #include "mecanum_steady/location.h"
#include "std_msgs/Float64.h"
#include "std_msgs/Bool.h"
#include "geometry_msgs/Twist.h"
#include "geometry_msgs/Point.h"
#include <algorithm>

// connect to STM32
ros::Subscriber sub_base_vel;
ros::Publisher pub_base_vel;

double base_now_x_vel;
double base_now_y_vel;
double base_now_z_vel;

void base_cb(const geometry_msgs::Twist::ConstPtr& msg) {
    base_now_x_vel = msg->linear.x;
    base_now_y_vel = msg->linear.y;
    base_now_z_vel = msg->angular.z;
    ROS_INFO("get base speed! %lf %lf %lf", base_now_x_vel, base_now_y_vel, base_now_z_vel);

}



int main(int argc, char** argv) {
    std::cout << "run control" << std::endl;
    ros::init(argc, argv, "test_send");
    ros::NodeHandle nh;


    // connect to STM32
    sub_base_vel = nh.subscribe("/base_speed", 1, base_cb);
    pub_base_vel = nh.advertise<geometry_msgs::Twist>("/cmd_vel", 1);


    double send_vel_x, send_vel_y, send_vel_z;
    geometry_msgs::Twist send_vel;
    while (std::cin >> send_vel_x >> send_vel_y >> send_vel_z) {
        send_vel.linear.x = send_vel_x;
        send_vel.linear.y = send_vel_y;
        send_vel.angular.z = send_vel_z;
        pub_base_vel.publish(send_vel);
        ros::spinOnce();
    }
    return 0;
}
