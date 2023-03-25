#include "ros/ros.h"
// #include "mecanum_steady/location.h"
#include "std_msgs/Float64.h"
#include "std_msgs/Bool.h"
#include "geometry_msgs/Twist.h"
#include "geometry_msgs/Point.h"
#include <algorithm>



ros::Subscriber sub_dist;
ros::Publisher pub_next_ctl;

// connect to STM32
ros::Subscriber sub_base_vel;
ros::Publisher pub_base_vel;

    // ros::Publisher pub_control = nh.advertise<mecanum_steady::location>("/control_effort", 1);

double start_x, start_y, start_z;
double target_x, target_y, target_z;
double now_x_pos, now_y_pos, now_z_pos;
double total_x, total_y, total_z;

// param
double x_tol;
double y_tol;
double z_tol;

double acc_xy;
double acc_zz;

double acc_x_frac;
double acc_y_frac;
double acc_z_frac;

double max_xy_vel;
double min_xy_vel;
double max_zz_vel;

double kp_xy;
double kp_zz;
// param

double x_err, y_err, z_err;


geometry_msgs::Twist vel;
double last_time, now_time;
double base_now_x_vel, base_now_y_vel, base_now_z_vel;
double base_last_x_vel = 0, base_last_y_vel = 0, base_last_z_vel = 0;

void dist_cb(const geometry_msgs::Point::ConstPtr& msg) {
    // start_x = msg->start_x;
    // start_y = msg->start_y;
    // start_z = msg->start_z;
    target_x = msg->x;
    target_y = msg->y;
    target_z = msg->z;

    total_x = target_x - now_x_pos;
    total_y = target_y - now_y_pos;
    total_z = target_z - now_z_pos;
    /* ROS_INFO("target %lf total %lf", target_x, total_x); */


    // temporary use for turtlesim
    base_now_x_vel = vel.linear.x;
    base_now_y_vel = vel.linear.y;
    base_now_z_vel = vel.angular.z;
    ROS_INFO("now assign %lf", vel.linear.x);
}
bool in_error() {
    if (x_err == 0 && y_err == 0 && z_err == 0) return 0;
    bool able_to_stop = true;
    able_to_stop &= (fabs(x_err) < x_tol);
    able_to_stop &= (fabs(y_err) < y_tol);
    able_to_stop &= (fabs(z_err) < z_tol);
    /* ROS_INFO("err1 %lf %lf", fabs(x_err), x_tol); */
    /* ROS_INFO("err2 %lf %lf", fabs(y_err), y_tol); */
    /* ROS_INFO("err3 %lf %lf", fabs(z_err), z_tol); */
    if (able_to_stop) {
        start_x = now_x_pos;
        start_y = now_y_pos;
        start_z = now_z_pos;
    }
    return able_to_stop;
}
// void state_cb(const mecanum_steady::location::ConstPtr& msg) {
//     now_x = msg->x;
//     now_y = msg->y;
//     now_z = msg->z;
// }


// mecanum_steady::location vel; 

void base_cb(const geometry_msgs::Twist::ConstPtr& msg) {
    base_now_x_vel = msg->linear.x;
    base_now_y_vel = msg->linear.y;
    base_now_z_vel = msg->angular.z;

}

void cal_pose() {
    now_time = ros::Time::now().toSec();
    now_x_pos += (base_last_x_vel + base_now_x_vel) / 2 * (now_time - last_time);
    now_y_pos += (base_last_y_vel + base_now_y_vel) / 2 * (now_time - last_time);
    now_z_pos += (base_last_z_vel + base_now_z_vel) / 2 * (now_time - last_time);
    
    last_time = ros::Time::now().toSec();
    base_last_x_vel = base_now_x_vel;
    base_last_y_vel = base_now_y_vel;
    base_last_z_vel = base_now_z_vel;

}
void get_param(ros::NodeHandle nh) {
    nh.getParam("x_tol", x_tol);
    nh.getParam("y_tol", y_tol);
    nh.getParam("z_tol", z_tol);
    nh.getParam("acc_xy", acc_xy);
    nh.getParam("acc_zz", acc_zz);
    nh.getParam("acc_x_frac", acc_x_frac);
    nh.getParam("acc_y_frac", acc_y_frac);

    nh.getParam("acc_z_frac", acc_z_frac);
    nh.getParam("max_xy_vel", max_xy_vel);
    nh.getParam("min_xy_vel", min_xy_vel);
    nh.getParam("max_zz_vel", max_zz_vel);
    nh.getParam("kp_xy", kp_xy);
    nh.getParam("kp_zz", kp_zz);
}


int main(int argc, char** argv) {
    ros::init(argc, argv, "control");
    ros::NodeHandle nh;

    sub_dist = nh.subscribe("/setpoint", 1, dist_cb);
    pub_next_ctl = nh.advertise<std_msgs::Bool>("/next_ctl", 1);

    // connect to STM32
    sub_base_vel = nh.subscribe("/base_speed", 1, base_cb);
    // pub_base_vel = nh.advertise<mecanum_steady::location>("/cmd_vel", 1);


    // temporary use for turtlesim
    pub_base_vel = nh.advertise<geometry_msgs::Twist>("turtle1/cmd_vel", 1);



    get_param(nh);

    std_msgs::Bool go_next;
    double vel_x, vel_y, vel_z;

    go_next.data = 0;
    while (ros::ok()) {
        ros::spinOnce();
        cal_pose();
        // ROS_INFO("now x pos %lf %lf %lf", now_x_pos, now_y_pos, now_z_pos);

        x_err = target_x - now_x_pos;
        y_err = target_y - now_y_pos;
        z_err = target_z - now_z_pos;

        if (in_error()) {
            ROS_INFO("in error");
            go_next.data = 1;
            vel_x = 0, vel_y = 0, vel_z = 0;
        }
        else {
            go_next.data = 0;
            if (fabs(x_err) > x_tol && fabs(x_err) < fabs(total_x*acc_x_frac)) {
                vel_x += (x_err > 0 ? acc_xy : -acc_xy);
                if (vel_x > max_xy_vel) vel_x = max_xy_vel;
                else if (vel_x < -max_xy_vel) vel_x = -max_xy_vel;
            }
            else if (fabs(x_err) > x_tol) {
                vel_x = kp_xy * x_err;
                if (vel_x > max_xy_vel) vel_x = max_xy_vel;
                else if (vel_x < -max_xy_vel) vel_x = -max_xy_vel;
                else if (vel_x > 0 && vel_x < min_xy_vel) vel_x = min_xy_vel;
                else if (vel_x < 0 && vel_x > -min_xy_vel) vel_x = -min_xy_vel;
            }

            if (fabs(y_err) > y_tol && fabs(y_err) < fabs(total_y*acc_y_frac)) {
                vel_y += (y_err > 0 ? acc_xy : -acc_xy);
                if (vel_y > max_xy_vel) vel_y = max_xy_vel;
                else if (vel_y < -max_xy_vel) vel_y = -max_xy_vel;
            }
            else if (fabs(y_err) > y_tol) {
                vel_y = kp_xy * y_err;
                if (vel_y > max_xy_vel) vel_y = max_xy_vel;
                else if (vel_y < -max_xy_vel) vel_y = -max_xy_vel;
                else if (vel_y > 0 && vel_y < min_xy_vel) vel_y = min_xy_vel;
                else if (vel_y < 0 && vel_y > -min_xy_vel) vel_y = -min_xy_vel;
            }
            // ROS_INFO("vel_y %lf", vel_y);

            if (fabs(z_err) > z_tol && fabs(z_err) < fabs(total_z*acc_z_frac)) {
                vel_z += (z_err > 0 ? acc_zz : -acc_zz);
                if (vel_z > max_zz_vel) vel_z = max_zz_vel;
                else if (vel_z < -max_zz_vel) vel_z = -max_zz_vel;
            }
            else if (fabs(z_err) > z_tol) {
                vel_z = kp_zz * z_err;
                if (vel_z > max_zz_vel) vel_z = max_zz_vel;
                else if (vel_z < -max_zz_vel) vel_z = -max_zz_vel;
            }
        }
        vel.linear.x = vel_x, vel.linear.y = vel_y, vel.angular.z = vel_z; 

        pub_next_ctl.publish(go_next);
        pub_base_vel.publish(vel);

    }
    return 0;
}
