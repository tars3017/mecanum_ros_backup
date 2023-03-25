#include "ros/ros.h"
#include "mecanum_steady/location.h"
#include "std_msgs/Float64.h"
#include "std_msgs/Bool.h"
#include <algorithm>


double start_x, start_y, start_z;
double target_x, target_y, target_z;
double now_x, now_y, now_z;
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
void dist_cb(const mecanum_steady::location::ConstPtr& msg) {
    // start_x = msg->start_x;
    // start_y = msg->start_y;
    // start_z = msg->start_z;
    target_x = msg->x;
    target_y = msg->y;
    target_z = msg->z;

    total_x = target_x - now_x;
    total_y = target_y - now_y;
    total_z = target_z - now_z;
    /* ROS_INFO("target %lf total %lf", target_x, total_x); */
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
int main(int argc, char** argv) {
    ros::init(argc, argv, "control");
    ros::NodeHandle nh;

    ros::Subscriber sub_dist = nh.subscribe("/setpoint", 10, dist_cb);
    ros::Subscriber sub_state = nh.subscribe("/state", 10, state_cb);
    ros::Publisher pub_next_ctl = nh.advertise<std_msgs::Bool>("/next_ctl", 10);
    ros::Publisher pub_control = nh.advertise<mecanum_steady::location>("/control_effort", 10);

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


    std_msgs::Bool go_next;
    mecanum_steady::location vel; 
    double pub_x, pub_y, pub_z;
    double vel_x, vel_y, vel_z;

    go_next.data = 0;
    while (ros::ok()) {
        ros::spinOnce();

        x_err = target_x - now_x;
        y_err = target_y - now_y;
        z_err = target_z - now_z;

        if (in_error()) {
            ROS_INFO("in error");
            go_next.data = 1;
            vel_x = 0, vel_y = 0, vel_z = 0;
            pub_x = 0, pub_y = 0, pub_z = 0;
            ROS_INFO("now_x %lf %lf %lf", start_x, now_x, target_x);
        }
        else {
            go_next.data = 0;
            if (fabs(x_err) > x_tol && fabs(x_err) < fabs(total_x*acc_x_frac)) {
                vel_x += (x_err > 0 ? acc_xy : -acc_xy);
                if (vel_x > max_xy_vel) vel_x = max_xy_vel;
                else if (vel_x < -max_xy_vel) vel_x = -max_xy_vel;
                pub_x = vel_x;            
            }
            else if (fabs(x_err) > x_tol) {
                vel_x = kp_xy * x_err;
                if (vel_x > max_xy_vel) vel_x = max_xy_vel;
                else if (vel_x < -max_xy_vel) vel_x = -max_xy_vel;
                else if (vel_x > 0 && vel_x < min_xy_vel) vel_x = min_xy_vel;
                else if (vel_x < 0 && vel_x > -min_xy_vel) vel_x = -min_xy_vel;
                pub_x = vel_x;
            }

            if (fabs(y_err) > y_tol && fabs(y_err) < fabs(total_y*acc_y_frac)) {
                vel_y += (y_err > 0 ? acc_xy : -acc_xy);
                if (vel_y > max_xy_vel) vel_y = max_xy_vel;
                else if (vel_y < -max_xy_vel) vel_y = -max_xy_vel;
                pub_y = vel_y;            
            }
            else if (fabs(y_err) > y_tol) {
                vel_y = kp_xy * y_err;
                if (vel_y > max_xy_vel) vel_y = max_xy_vel;
                else if (vel_y < -max_xy_vel) vel_y = -max_xy_vel;
                else if (vel_y > 0 && vel_y < min_xy_vel) vel_y = min_xy_vel;
                else if (vel_y < 0 && vel_y > -min_xy_vel) vel_y = -min_xy_vel;
                pub_y = vel_y;
            }
            // ROS_INFO("vel_y %lf", vel_y);

            if (fabs(z_err) > z_tol && fabs(z_err) < fabs(total_z*acc_z_frac)) {
                vel_z += (z_err > 0 ? acc_zz : -acc_zz);
                if (vel_z > max_zz_vel) vel_z = max_zz_vel;
                else if (vel_z < -max_zz_vel) vel_z = -max_zz_vel;
                pub_z = vel_z;            
            }
            else if (fabs(z_err) > z_tol) {
                vel_z = kp_zz * z_err;
                if (vel_z > max_zz_vel) vel_z = max_zz_vel;
                else if (vel_z < -max_zz_vel) vel_z = -max_zz_vel;
                pub_z = vel_z;
            }
        }
        vel.x = pub_x, vel.y = pub_y, vel.z = pub_z; 
        pub_next_ctl.publish(go_next);
        pub_control.publish(vel);
    }
    return 0;
}
