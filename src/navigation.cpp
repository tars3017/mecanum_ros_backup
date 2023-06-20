#include "ros/ros.h"
// #include "mecanum_steady/location.h"
#include "std_msgs/Float64.h"
#include "std_msgs/Bool.h"
#include "geometry_msgs/Twist.h"
#include "geometry_msgs/Point.h"
#include <algorithm>



ros::Subscriber sub_dist;
ros::Publisher pub_next_ctl;

ros::Subscriber sub_odom;
// connect to STM32
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
double min_zz_vel;

double kp_xy;
double kp_zz;

double x_slow_dist, y_slow_dist, z_slow_dist;
// param

double x_err, y_err, z_err;


geometry_msgs::Twist vel;

double base_now_x_vel, base_now_y_vel, base_now_z_vel;
double base_last_x_vel = 0, base_last_y_vel = 0, base_last_z_vel = 0;

bool test_mode, debug_send_vel, debug_get_vel;

void odom_cb(const geometry_msgs::Point::ConstPtr& msg) {
    now_x_pos = msg->x;
    now_y_pos = msg->y;
    now_z_pos = msg->z;
}

void dist_cb(const geometry_msgs::Point::ConstPtr& msg) {
    // start_x = msg->start_x;
    // start_y = msg->start_y;
    // start_z = msg->start_z;
    target_x = msg->x;
    target_y = msg->y;
    target_z = msg->z;

    total_x = target_x - start_x;
    total_y = target_y - start_y;
    total_z = target_z - start_z;

    // ROS_INFO("target %lf now %lf", target_x, now_x_pos);


    // temporary use for turtlesim
    // base_now_x_vel = vel.linear.x;
    // base_now_y_vel = vel.linear.y;
    // base_now_z_vel = vel.angular.z;
    // ROS_INFO("now assign %lf", vel.linear.x);
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
    nh.getParam("min_zz_vel", min_zz_vel);
    nh.getParam("kp_xy", kp_xy);
    nh.getParam("kp_zz", kp_zz);
    
    nh.getParam("test_mode", test_mode);
    nh.getParam("debug_send_vel", debug_send_vel);
    nh.getParam("debug_get_vel", debug_get_vel);

    nh.getParam("x_slow_dist", x_slow_dist);
    nh.getParam("y_slow_dist", y_slow_dist);
    nh.getParam("z_slow_dist", z_slow_dist);
}


int main(int argc, char** argv) {

    ros::init(argc, argv, "navigation");
    ros::NodeHandle nh;
    get_param(nh);

    std::cout << "run navigation" << std::endl;
    if (test_mode) std::cout << "===== test mode =====" << std::endl; 

    sub_dist = nh.subscribe("/base_goal", 1, dist_cb);
    pub_next_ctl = nh.advertise<std_msgs::Bool>("/reached_status", 1);

    // connect to STM32
    sub_odom = nh.subscribe("/odom", 1, odom_cb);
    pub_base_vel = nh.advertise<geometry_msgs::Twist>("/cmd_vel", 1);
    

    // temporary use for turtlesim
    // pub_base_vel = nh.advertise<geometry_msgs::Twist>("turtle1/cmd_vel", 1);


    ros::Rate loop_rate(10);

    std_msgs::Bool go_next;
    double vel_x = 0, vel_y = 0, vel_z = 0;

    go_next.data = 0;
    double real_x_max = 0, real_y_max = 0, real_z_max = 0;
    while (ros::ok()) {
        // ROS_INFO("her1");
        ros::spinOnce();
        // ROS_INFO("here2");
        // ROS_INFO("now x pos %lf %lf %lf", now_x_pos, now_y_pos, now_z_pos);

        x_err = target_x - now_x_pos;
        y_err = target_y - now_y_pos;
        z_err = target_z - now_z_pos;
        // ROS_INFO("now x pos %lf %lf %lf", x_err, y_err, z_err);
        
        if (in_error()) {
            // ROS_INFO("in error");
            go_next.data = 1;
            vel_x = 0, vel_y = 0, vel_z = 0;
        }
        else {
            go_next.data = 0;
            // std::cout << "total_x " << total_x << " " << acc_x_frac << ' ' << x_err << "\n";
            if ( fabs(x_err) > x_tol && fabs(x_err) > fabs(total_x*(1-acc_x_frac)) ) {
                // std::cout << "a" << std::endl;
                vel_x += (x_err > 0 ? acc_xy : -acc_xy);
                if (vel_x > max_xy_vel) vel_x = max_xy_vel;
                else if (vel_x < -max_xy_vel) vel_x = -max_xy_vel;
                real_x_max = std::max(real_x_max, fabs(vel_x));
            }
            else if (fabs(x_err) > x_tol) {
                // std::cout << "b" << std::endl;
                vel_x = kp_xy * x_err;
                if (vel_x > std::min(max_xy_vel, real_x_max)) vel_x = std::min(max_xy_vel, real_x_max);
                else if (vel_x < -std::min(max_xy_vel, real_x_max)) vel_x = -std::min(max_xy_vel, real_x_max);
                else if (vel_x > 0 && vel_x < min_xy_vel) vel_x = min_xy_vel;
                else if (vel_x < 0 && vel_x > -min_xy_vel) vel_x = -min_xy_vel;
            }
            else {
                vel_x = 0;
            }

            if ( fabs(y_err) > y_tol && fabs(y_err) > fabs(total_y*(1-acc_y_frac)) ) {
                vel_y += (y_err > 0 ? acc_xy : -acc_xy);
                if (vel_y > max_xy_vel) vel_y = max_xy_vel;
                else if (vel_y < -max_xy_vel) vel_y = -max_xy_vel;
                real_y_max = std::max(real_y_max, fabs(vel_y));
            }
            else if (fabs(y_err) > y_tol) {
                vel_y = kp_xy * y_err;
                if (vel_y > std::min(max_xy_vel, real_y_max)) vel_y = std::min(max_xy_vel, real_y_max);
                else if (vel_y < -std::min(max_xy_vel, real_y_max)) vel_y = -std::min(max_xy_vel, real_y_max);
                else if (vel_y > 0 && vel_y < min_xy_vel) vel_y = min_xy_vel;
                else if (vel_y < 0 && vel_y > -min_xy_vel) vel_y = -min_xy_vel;
            }
            else {
                vel_y = 0;
            }
            // ROS_INFO("vel_y %lf", vel_y);

            if ( fabs(z_err) > z_tol && fabs(z_err) > fabs(total_z*(1-acc_z_frac))) {
                vel_z += (z_err > 0 ? acc_zz : -acc_zz);
                if (vel_z > max_zz_vel) vel_z = max_zz_vel;
                else if (vel_z < -max_zz_vel) vel_z = -max_zz_vel;
                real_z_max = std::max(real_z_max, vel_z);
            }
            else if (fabs(z_err) > z_tol) {
                vel_z = kp_zz * z_err;
                if (vel_z > std::min(max_zz_vel, real_z_max)) vel_z = std::min(max_zz_vel, real_z_max);
                else if (vel_z < -std::min(max_zz_vel, real_z_max)) vel_z = -std::min(max_zz_vel, real_z_max);
                else if (vel_z > 0 && vel_z < min_zz_vel) vel_z = min_zz_vel;
                else if (vel_z < 0 && vel_z > -min_zz_vel) vel_z = -min_zz_vel;
            }
            else {
                vel_z = 0;
            }
        }
        vel.linear.x = vel_x, vel.linear.y = vel_y, vel.angular.z = vel_z; 

        if (debug_send_vel && (vel_x != 0 || vel_y != 0 || vel_z != 0)) {
            ROS_INFO("Output speed %lf %lf %lf", vel_x, vel_y, vel_z);
        }
        pub_next_ctl.publish(go_next);
        pub_base_vel.publish(vel);
        /* if (vel_x > 0 ) */ 
        /*     ROS_INFO("published %lf %lf %lf", vel_x, vel_y, vel_z); */
        // ROS_INFO("in the loop");
        loop_rate.sleep();
        // ROS_INFO("sleep done");
    }
    ROS_INFO("navigation end");
    return 0;
}
