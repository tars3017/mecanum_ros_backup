#include "ros/ros.h"
#include "mecanum_steady/location.h"
#include "std_msgs/Float64.h"
#include "std_msgs/Bool.h"

double last_x_vel, last_y_vel, last_z_vel;
double cur_x_vel, cur_y_vel, cur_z_vel;

double x_pos, y_pos, z_pos;
void control_cb(const std_msgs::Float64::ConstPtr& msg) {
    cur_x_vel = msg->x;
    cur_y_vel = msg->y;
    cur_z_vel = msg->z;
}
void encoder_cb(const mecanum_steady::location::ConstPtr& msg) {
    // TODO
}
int main(int argc, char** argv) {
    ros::init(argc, argv, "drive");
    ros::NodeHandle nh;

    ros::Subscriber sub_control = nh.subscribe("/control_effort", 10, control_cb);
    ros::Publisher pub_state = nh.advertise<mecanum_steady::location>("/state", 10);
    
    // below 2 lines are for temporary usage
    ros::Subscriber sub_encoder = nh.subscribe("/encoder", 10, encoder_cb);
    ros::Publisher pub_motor = nh.advertise<mecanum_steady::location>("/motor_speed", 10);

    last_x_vel = 0, last_y_vel = 0, last_z_vel = 0;
    cur_x_vel = 0, cur_y_vel = 0, cur_z_vel = 0;
    x_pos = 0, y_pos = 0, z_pos = 0;
    ros::Time last_time, now_time;

    last_time = ros::Time::now();
    mecanum_steady::locatino output_vel;
    while (ros::ok()) {
        ros::spinOnce();

        /* temporary for Rviz */
        last_x_vel = cur_x_vel;
        last_y_vel = cur_y_vel;
        last_z_vel = cur_z_vel;
        /* temporary for Rviz */

        now_time = ros::Time::now();
        x_pos += (last_x_vel) * (now_time - last_time); 
        y_pos += (last_y_vel) * (now_time - last_time); 
        z_pos += (last_z_vel) * (now_time - last_time); 

        
        output_vel.x = cur_x_vel;
        output_vel.y = cur_y_vel;
        output_vel.z = cur_z_vel;
        pub_motor.publish(output_vel);
        last_time = ros::Time::now();
    }
    return 0;
}
