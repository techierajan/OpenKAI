#ifndef OPENKAI_SRC_INTERFACE_MAVLINK_H_
#define OPENKAI_SRC_INTERFACE_MAVLINK_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../include/mavlink/common/mavlink.h"
#include "../include/mavlink/mavlink_conversions.h"
#include "../IO/SerialPort.h"
#include "../UI/Window.h"

#define NUM_MSG_HANDLE 100

namespace kai
{

struct Time_Stamps
{
	Time_Stamps()
	{
		reset_timestamps();
	}

	uint64_t heartbeat;
	uint64_t sys_status;
	uint64_t battery_status;
	uint64_t radio_status;
	uint64_t local_position_ned;
	uint64_t global_position_int;
	uint64_t position_target_local_ned;
	uint64_t position_target_global_int;
	uint64_t highres_imu;
	uint64_t attitude;

	void reset_timestamps()
	{
		heartbeat = 0;
		sys_status = 0;
		battery_status = 0;
		radio_status = 0;
		local_position_ned = 0;
		global_position_int = 0;
		position_target_local_ned = 0;
		position_target_global_int = 0;
		highres_imu = 0;
		attitude = 0;
	}

};

// Struct containing information on the MAV we are currently connected to
struct Mavlink_Messages
{
	int sysid;
	int compid;

	mavlink_heartbeat_t heartbeat;
	mavlink_command_ack_t command_ack;
	mavlink_sys_status_t sys_status;
	mavlink_battery_status_t battery_status;
	mavlink_radio_status_t radio_status;
	mavlink_local_position_ned_t local_position_ned;
	mavlink_global_position_int_t global_position_int;
	mavlink_position_target_local_ned_t position_target_local_ned;
	mavlink_position_target_global_int_t position_target_global_int;
	mavlink_highres_imu_t highres_imu;
	mavlink_attitude_t attitude;
	Time_Stamps time_stamps;

	void reset_timestamps()
	{
		time_stamps.reset_timestamps();
	}

};


class _Mavlink: public _ThreadBase
{
public:
	_Mavlink();
	~_Mavlink();

	bool init(void* pKiss);
	bool link(void);

	void close(void);
	bool start(void);
	bool draw(void);

	//Receive
	void handleMessages();
	bool readMessage(mavlink_message_t &message);

	//Send
	int  writeMessage(mavlink_message_t message);
	void requestDataStream(uint8_t stream_id, int rate);
	void sendHeartbeat(void);

	//Commands
	void gps_input(mavlink_gps_input_t* pGPSinput);
	void set_attitude_target(float* pAttitude, float* pRate, float thrust, uint8_t mask);
	void landing_target(uint8_t stream_id, uint8_t frame, float angle_x, float angle_y, float distance, float size_x, float size_y);
	void command_long_doSetMode(int mode);
	void command_long_doSetPositionYawThrust(float steer, float thrust);
	void distance_sensor(uint8_t type, uint8_t orientation, uint16_t max, uint16_t min, uint16_t v);


public:
	SerialPort* m_pSerialPort;

	int m_systemID;
	int m_componentID;
	int m_type;
	int m_targetComponentID;

	Mavlink_Messages m_msg;
	mavlink_set_position_target_local_ned_t m_initPos;
	mavlink_status_t m_status;

	//Read Thread
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Mavlink *) This)->update();
		return NULL;
	}


};

}

#endif

/*
I believe this is the one
The board itself is 1m.  The outter edge of the red circle is 93cm diameter
each red ring is 7cm wide
the central red circle is 9cm diameter

MAVLink command sequence..
1. OpenKai should send a heartbeat at 1hz.  http://mavlink.org/messages/common#HEARTBEAT
Inside the HEARTBEAT message (and any other message), openkai needs to specify it's sys-id and component-id.
ideally the sys-id should match the vehicle's sysid.
By default the vehicle's sys-id is "1" so you could just hardcode that for now.
Eventually openKai should instead listen for heartbeat messages on it's serial port and should look for the first heartbeat messages with a
type != MAV_TYPE_GCS ("6") and grab that system id and use it.

the component id can be anything except 0.
there's a list of suggested component-ids on this page (search for MAV_COMPONENT) http://mavlink.org/messages/common.  but it doesn't really matter.  Maybe we could use MAV_COMP_ID_PATHPLANNER ("195")

The Heartbeat's "type" should be MAV_TYPE_ONBOARD_CONTROLLER (18)
"base_mode" can be anything.
maybe we will eventually assign values for when OpenKai is providing precision-landing updates vs follow-me updates.. but for now just make it "0" I think
"autopilot" should be "0"
"custom_mode" can be "0"
"system_status" should be MAV_STATE_ACTIVE ("4")

Next, when openkai sees the target is should send LANDING_TARGET messages maybe as quickly as it can.
10hz would be nice.  over 50hz is too fast.
http://mavlink.org/messages/common#LANDING_TARGET
	"time_usec" is just the TX1's system time in sec * 1,000,000.  We don't use this anyway
	"target_num" = "0" (for now anyway)
	"frame" = MAV_FRAME_BODY_NED ("8")
	"angle_x" : the horizontal angle to the target in radians.
		0 = the target is right in the middle of the screen.
		negative means the target is to the left of center,
		positive means it's to the right of center.
		OpenKai will need to know the field-of-view of the camera.
		So for example, I measured the field of view of the Logitech 920c and horizontall it's 70.42degrees).
		So if the center of the target was on the left most edge of the image we would send "angle_x" as -0.61 radians (= -35degrees)
	"angle_y" : the vertical angle to the target in radians.
		0 = middle.  to be honest, I'm not exactly sure if negative is up or down.
	"distance" - we don't use this now.
		It can be estimated even using non-stereo cameras if you know the actual size of the object.
		As you probably know, the apparent diameter of the object halves if it's distance is doubled.
		Anyway, we don't use it anyway for now
	"size_x", "size_y" - the horizontal and vertical size of the object in radians.  Also not used.
 */


