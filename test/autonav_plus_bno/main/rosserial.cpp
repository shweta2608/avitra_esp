#include "ros.h"
#include "auto_nav/velocity_msg.h"
#include "auto_nav/ticks_msg.h"
#include "std_msgs/Float32.h"
#include "rosserial.h"

ros::NodeHandle nh;

auto_nav::velocity_msg velocity;
auto_nav::velocity_msg rpm;
auto_nav::ticks_msg ticks;
std_msgs::Float32 angle;
ros::Publisher espPub_ticks("ticks", &ticks);
ros::Publisher espPub_yaw("yaw", &angle);

void callback_teleop(const auto_nav::velocity_msg& msg){
  velocity.motor_F = msg.motor_F;
  velocity.motor_B = msg.motor_B;
  velocity.motor_L = msg.motor_L;
  velocity.motor_R = msg.motor_R;
  velocity.command = msg.command;
}
ros::Subscriber<auto_nav::velocity_msg> espSub_teleop("teleop", &callback_teleop);

void callback_rpm(const auto_nav::velocity_msg& msg){
  rpm.motor_L = msg.motor_L;
  rpm.motor_R = msg.motor_R;
}
ros::Subscriber<auto_nav::velocity_msg> espSub_rpm("rpm", &callback_rpm);

void rosserial_setup(){  // Initialize ROS
  nh.initNode();
  nh.advertise(espPub_ticks);
  nh.advertise(espPub_yaw);
  nh.subscribe(espSub_teleop);
  nh.subscribe(espSub_rpm);
  velocity.motor_F = 0;
  velocity.motor_B = 0;
  velocity.motor_L = 0;
  velocity.motor_R = 0;
  velocity.command = 0;
  rpm.motor_L = 0;
  rpm.motor_R = 0;
}

/*here*/
void rosserial_publish(volatile int32_t* ticks_L, volatile int32_t* ticks_R, float* yaw){
  ticks.motor_L = *ticks_L;
  ticks.motor_R = *ticks_R;
  ticks.motor_R *= -1;    //since the right motor moves forward in negative rpm, negative ticks are published
  angle.data = *yaw;
  espPub_ticks.publish(&ticks);  // publish the msg
  espPub_yaw.publish(&angle);  // publish the msg
  nh.spinOnce();
}

void rosserial_subscribe_teleop(float* duty_F, float* duty_B, float* duty_L, float* duty_R){
  *duty_F = velocity.motor_F;
  *duty_B = velocity.motor_B;
  *duty_L = velocity.motor_L;
  *duty_R = velocity.motor_R;
  nh.spinOnce();
}

void rosserial_subscribe_command(int* command){
  *command = velocity.command;
  nh.spinOnce();
}

void rosserial_subscribe_rpm(float* rpm_L, float* rpm_R){    
  *rpm_L = rpm.motor_L;
  *rpm_R = rpm.motor_R;
  *rpm_R *= -1;   //since the right motor moves forward with negative rpm
  nh.spinOnce();
}