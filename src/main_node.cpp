#include "ros/ros.h"
// #include "mecanum_steady/location.h"
#include "std_msgs/Float64.h"
#include "std_msgs/Bool.h"
#include "geometry_msgs/Point.h"

#define target_num 8

double target[10][5] { {4, 0, 0}, {4, 4, 0} , {0, 4, 0}, {0, 0, 0},
                       {4, 0, 0}, {4, 4, 0}, {0, 4, 0}, {0, 0, 0}};
int idx;
bool last_go_next;
bool auto_mode;
geometry_msgs::Point now_goal;
void next_ctl_cb(const std_msgs::Bool::ConstPtr& msg) {
    if (msg->data) {
        if (last_go_next) return ; 
        last_go_next = msg->data;
        if (auto_mode) {
            now_goal.x = target[idx][0];
            now_goal.y = target[idx][1];
            now_goal.z = target[idx][2];
            ++idx;
        }
        else {
            std::cout << "Reached the goal (" << now_goal.x
            << ", " << now_goal.y << ", " << now_goal.z << ")\nInput next goal:";
            std::cin >> now_goal.x >> now_goal.y >> now_goal.z;
            bool check;
            if (now_goal.z > 3.14) {
                std::cout << "Notice the unit! Are you sure?\n";
                std::cin >> check;
            }
        }
        
        
        // ROS_INFO("now %d", idx);
    }
    else {
        last_go_next = 0;
    }
    return ;
}
int main(int argc, char** argv) { 
    ros::init(argc, argv, "main_node");
    ros::NodeHandle nh;

    ros::Publisher pub_dest = nh.advertise<geometry_msgs::Point>("/base_goal", 1);
    ros::Subscriber sub = nh.subscribe("/reached_status", 1, next_ctl_cb);

    std::cout << "Auto Mode ? ";
    std::cin >> auto_mode;
    
    if (auto_mode) {
        now_goal.x = target[idx][0];
        now_goal.y = target[idx][1];
        now_goal.z = target[idx][2];
        ++idx;
    }
    else {
        std::cout << "First target: ";
        std::cin >> now_goal.x >> now_goal.y >> now_goal.z;
    }
    
    while (ros::ok()) {
        ros::spinOnce(); 
        if (idx > target_num) break;
        // now_goal.x = target[idx][0];
        // now_goal.y = target[idx][1];
        // now_goal.z = target[idx][2];

        /* ROS_INFO("idx %d", idx); */
        /* std::cout << "now_goal " << now_goal.x << ' ' << now_goal.y << ' ' << now_goal.z << std::endl; */    
        pub_dest.publish(now_goal); 
    }
    return 0;
}
