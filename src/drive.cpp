#include "ros/ros.h"
#include "mecanum_steady/location.h"
#include "std_msgs/Float64.h"
#include "std_msgs/Bool.h"
#include "geometry_msgs/Twist.h"

double last_x_vel, last_y_vel, last_z_vel;
double cur_x_vel, cur_y_vel, cur_z_vel;

double x_pos, y_pos, z_pos;
void control_cb(const mecanum_steady::location::ConstPtr& msg) {
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


    /* for turtlesim */
    ros::Publisher pub_turtle = nh.advertise<geometry_msgs::Twist>("/turtle1/cmd_vel", 10);
    geometry_msgs::Twist turtle_vel;
    /* for turtlesim */

    last_x_vel = 0, last_y_vel = 0, last_z_vel = 0;
    cur_x_vel = 0, cur_y_vel = 0, cur_z_vel = 0;
    x_pos = 0, y_pos = 0, z_pos = 0;
    double last_time, now_time;

    last_time = ros::Time::now().toSec();
    mecanum_steady::location now_state;
    mecanum_steady::location output_vel;
    while (ros::ok()) {
        ros::spinOnce();

        /* temporary for Rviz */
        last_x_vel = cur_x_vel;
        last_y_vel = cur_y_vel;
        last_z_vel = cur_z_vel;
        /* temporary for Rviz */

        now_time = ros::Time::now().toSec();
        now_state.x += (last_x_vel) * (now_time - last_time); 
        now_state.y += (last_y_vel) * (now_time - last_time); 
        now_state.z += (last_z_vel) * (now_time - last_time); 
        last_time = ros::Time::now().toSec();
        pub_state.publish(now_state);

        
        output_vel.x = cur_x_vel;
        output_vel.y = cur_y_vel;
        output_vel.z = cur_z_vel;
        pub_motor.publish(output_vel);

        /* for turtlesim */
        turtle_vel.linear.x = cur_x_vel;
        turtle_vel.linear.y = cur_y_vel;
        turtle_vel.angular.z = cur_z_vel;
        pub_turtle.publish(turtle_vel);
        /* for turtlesim */
    }
    return 0;
}
