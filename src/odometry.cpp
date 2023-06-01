#include "ros/ros.h"
// #include "mecanum_steady/location.h"
#include "std_msgs/Float64.h"
#include "std_msgs/Bool.h"
#include "geometry_msgs/Twist.h"
#include "geometry_msgs/Point.h"
#include <algorithm>
#include "geometry_msgs/PoseStamped.h"
#include "tf/transform_broadcaster.h"
#include <random>

double last_time, now_time;
bool test_mode, debug_get_vel;
double now_x_pos, now_y_pos, now_z_pos;
double base_now_x_vel, base_now_y_vel, base_now_z_vel;
double base_last_x_vel, base_last_y_vel, base_last_z_vel;

ros::Subscriber sub_base_vel;
ros::Publisher pub_odom;

// for Rviz
ros::Publisher pub_pose;

// for test_mode use
ros::Subscriber sub_navi_vel;
geometry_msgs::Point now_pt;
geometry_msgs::Twist vel;

// for rqt_plot real vel
geometry_msgs::Twist base_vel_for_test;
ros::Publisher pub_base_vel_for_test;


double get_random() {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> negative(0, 1);
    std::uniform_int_distribution<std::mt19937::result_type> x(0, 1000);
    double num = x(dev) * 1.0;
    if (negative(dev)) {num *= -1;}
    return num / 1000000000;
}

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

        base_vel_for_test.linear.x = msg->linear.x;
        base_vel_for_test.linear.y = msg->linear.y;
        base_vel_for_test.linear.z = msg->linear.z;
    }
    else {
        base_now_x_vel = vel.linear.x + get_random();
        base_now_y_vel = vel.linear.y + get_random();
        base_now_z_vel = vel.angular.z + get_random();

        base_vel_for_test.linear.x = base_now_x_vel;
        base_vel_for_test.linear.y = base_now_y_vel;
        base_vel_for_test.linear.z = base_now_z_vel;
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
    pub_base_vel_for_test = nh.advertise<geometry_msgs::Twist>("base_speed_test", 1);
    
    // for Rviz
    pub_pose = nh.advertise<geometry_msgs::PoseStamped>("/rviz_pose", 1);
    // for test_mode use
    sub_navi_vel = nh.subscribe("/cmd_vel", 1, navi_cb);
    ros::Rate loop_rate(10);
    last_time = ros::Time::now().toSec();
    while (ros::ok()) {
        ros::spinOnce();
        if (test_mode) {
            base_cb();
        }
        cal_pose();
        pub_odom.publish(now_pt);

        // for rqt_plot
        pub_base_vel_for_test.publish(base_vel_for_test);

        // for Rviz
        geometry_msgs::PoseStamped ps;
        ps.header.stamp = ros::Time::now();
        ps.header.frame_id = "map";
        ps.pose.position.x = now_pt.x;
        ps.pose.position.y = now_pt.y;
        geometry_msgs::Quaternion goal_quat = tf::createQuaternionMsgFromYaw(now_pt.z);
        ps.pose.orientation.w = goal_quat.w;
        ps.pose.orientation.x = goal_quat.x;
        ps.pose.orientation.y = goal_quat.y;
        ps.pose.orientation.z = goal_quat.z;
        pub_pose.publish(ps);

        loop_rate.sleep();
    }
    return 0;
}
