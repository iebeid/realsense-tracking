
#include "vrpn_tracker_rs.h"

int main(int argc, char **argv){
	vrpn_Connection_IP* m_Connection = new vrpn_Connection_IP();
	myTracker* serverTracker = new myTracker(m_Connection);
	cout << "Created VRPN server." << endl;
	while (true)
	{
		serverTracker->mainloop();
		m_Connection->mainloop();
		SleepEx(1, FALSE);
	}
}