#ifndef HEAD_TRACKER_H
#define HEAD_TRACKER_H

#include <stdio.h>
#include <tchar.h>

#include <math.h>

#include "vrpn_Text.h"
#include "vrpn_Tracker.h"
#include "vrpn_Analog.h"
#include "vrpn_Button.h"
#include "vrpn_Connection.h"

#include <iostream>
using namespace std;

class myTracker : public vrpn_Tracker
{
public:
	myTracker(vrpn_Connection *c /*= 0 */) :
		vrpn_Tracker("Tracker0", c)
	{
	}

	virtual ~myTracker() {};

	virtual void mainloop(){
		vrpn_gettimeofday(&_timestamp, NULL);

		vrpn_Tracker::timestamp = _timestamp;

		// We will just put a fake data in the position of our tracker
		static float angle = 0; angle += 0.001f;

		// the pos array contains the position value of the tracker
		// XXX Set your values here
		pos[0] = this->x;
		pos[1] = this->y;
		pos[2] = this->z;

		// the d_quat array contains the orientation value of the tracker, stored as a quaternion
		// XXX Set your values here
		d_quat[0] = this->qx;
		d_quat[1] = this->qy;
		d_quat[2] = this->qz;
		d_quat[3] = this->qw;

		char msgbuf[1000];

		d_sensor = 0;

		int  len = vrpn_Tracker::encode_to(msgbuf);

		if (d_connection->pack_message(len, _timestamp, position_m_id, d_sender_id, msgbuf,
			vrpn_CONNECTION_LOW_LATENCY))
		{
			fprintf(stderr, "can't write message: tossing\n");
		}

		server_mainloop();
	}

protected:
	struct timeval _timestamp;

public:
	float x, y, z, qx, qy, qz, qw;
};

#endif