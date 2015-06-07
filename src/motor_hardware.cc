/**
Copyright (c) 2015, Ubiquity Robotics
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of ubiquity_motor nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/
#include <boost/assign.hpp>
#include <ubiquity_motor/motor_hardware.h>
#include <ubiquity_motor/motor_command.h>

#include <boost/math/special_functions/round.hpp>

#define SENSOR_DISTANCE 0.00247525941

MotorHardware::MotorHardware(){
	ros::V_string joint_names = boost::assign::list_of("left_wheel")("right_wheel");

	for (unsigned int i = 0; i < joint_names.size(); i++) {
		hardware_interface::JointStateHandle joint_state_handle(joint_names[i],
		    &joints_[i].position, &joints_[i].velocity, &joints_[i].effort);
		joint_state_interface_.registerHandle(joint_state_handle);

		hardware_interface::JointHandle joint_handle(
		    joint_state_handle, &joints_[i].velocity_command);
		velocity_joint_interface_.registerHandle(joint_handle);
	}
	registerInterface(&joint_state_interface_);
	registerInterface(&velocity_joint_interface_);



	std::string sPort;
	int sBaud;

	if (!n.getParam("ubiquity_motor/serial_port", sPort))
	{
		sPort.assign("/dev/ttyS0");
		n.setParam("ubiquity_motor/serial_port", sPort);
	}

	if (!n.getParam("ubiquity_motor/serial_baud", sPort))
	{
		sBaud = 9600;
		n.setParam("ubiquity_motor/serial_baud", sBaud);
	}

	motor_serial_ = new MotorSerial(sPort,sBaud);
}

void MotorHardware::readInputs(){
	while(motor_serial_->commandAvailable()){
		MotorCommand mc;
		mc = motor_serial_-> receiveCommand();
		if(mc.getType() == MotorCommand::TYPE_RESPONSE){
			switch(mc.getRegister()){
				case MotorCommand::REG_LEFT_ODOM:
					joints_[0].position += mc.getData()*SENSOR_DISTANCE;
					break;
				case MotorCommand::REG_RIGHT_ODOM:
					joints_[1].position += mc.getData()*SENSOR_DISTANCE;
					break;
			}
		}
	}
}

void MotorHardware::writeSpeeds(){
	MotorCommand left;
	left.setRegister(MotorCommand::REG_LEFT_SPEED_SET);
	left.setType(MotorCommand::TYPE_WRITE);
	left.setData(boost::math::iround(joints_[0].velocity_command/SENSOR_DISTANCE));
	motor_serial_->transmitCommand(left);
	MotorCommand right;
	right.setRegister(MotorCommand::REG_RIGHT_SPEED_SET);
	right.setType(MotorCommand::TYPE_WRITE);
	right.setData(boost::math::iround(joints_[1].velocity_command/SENSOR_DISTANCE));
	//ROS_ERROR("RIGHT %i", right.getData());
	motor_serial_->transmitCommand(right);
	//ROS_ERROR("SPEEDS %x %x", left.serialize()[7], right.serialize().data()[7]);
}

void MotorHardware::requestOdometry(){

}