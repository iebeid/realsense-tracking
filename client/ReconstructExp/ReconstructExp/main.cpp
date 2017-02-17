#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <vrpn_Connection.h>
#include <vrpn_Button.h>
#include <vrpn_Tracker.h>
#include <vrpn_Analog.h>

int width = 640;
int heigth = 480;
float c_x = 0, c_y = 0, c_z = 0;
float d_quat_0 = 0, d_quat_1 = 0, d_quat_2 = 0, d_quat_3 = 1;
std::vector<glm::vec3> pos, rot;
vrpn_Tracker_Remote* vrpnTracker = NULL;

using namespace std;

void VRPN_CALLBACK handle_tracker(void* userData, const vrpn_TRACKERCB t)
{
	cout << "Tracker '" << t.sensor << "' : " << t.pos[0] << "," << t.pos[1] << "," << t.pos[2] << endl;
	c_x = t.pos[0];
	c_y = t.pos[1];
	c_z = t.pos[2];
	d_quat_0 = t.quat[0];
	d_quat_1 = t.quat[1];
	d_quat_2 = t.quat[2];
	d_quat_3 = t.quat[3];
	
}

double slope(const std::vector<double>& x, const std::vector<double>& y)
{
	const auto n = x.size();
	const auto s_x = std::accumulate(x.begin(), x.end(), 0.0);
	const auto s_y = std::accumulate(y.begin(), y.end(), 0.0);
	const auto s_xx = std::inner_product(x.begin(), x.end(), x.begin(), 0.0);
	const auto s_xy = std::inner_product(x.begin(), x.end(), y.begin(), 0.0);
	const auto a = (n * s_xy - s_x * s_y) / (n * s_xx - s_x * s_x);
	return a;
}

void drawCube()
{
	glBegin(GL_QUADS);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);

	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);

	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);

	glColor3f(1.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);

	glColor3f(1.0f, 0.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();
}

void render()
{
	glm::mat4 proj;
	glm::mat4 modelview;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	proj = glm::perspective(45.f, (float)width / (float)heigth, .1f, 1000.f);
	glMultMatrixf(glm::value_ptr(proj));
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glViewport(0, 0, width, heigth);
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float mul = 3;
	glTranslatef((-(c_x / 640 * 2) + 1) * mul, ((c_y / 480 * 2) - 1) * mul, c_z);
	glm::quat q(d_quat_0, d_quat_1, d_quat_2, d_quat_3);
	glm::vec3 angles = glm::eulerAngles(q);
	glRotatef(angles.x,1.0,0.0,0.0);
	glRotatef(angles.y, 0.0, 1.0, 0.0);
	glRotatef(angles.z, 0.0, 0.0, 1.0);
	for (int i = 0; i < pos.size(); i++)
	{
		glPushMatrix();
		glTranslatef(pos[i].x * 10, pos[i].y * 10, -pos[i].z * 10);
		glScalef(0.3, 0.3, 0.3);
		drawCube();
		glPopMatrix();
	}
}

bool collinear(int x1, int y1, int x2, int y2, int x3, int y3)
{
	return (y1 - y2) * (x1 - x3) == (y1 - y3) * (x1 - x2);
}

int main(int argc, char* argv[])
{
	vrpnTracker = new vrpn_Tracker_Remote("Tracker0@localhost");
	vrpnTracker->register_change_handler(0, handle_tracker);

	GLFWwindow *win;
	glfwInit();
	win = glfwCreateWindow(width, heigth, "realsense head tracking", NULL, NULL);
	glfwMakeContextCurrent(win);
	glEnable(GL_DEPTH_TEST);

	glfwSetWindowSizeCallback(win, [](GLFWwindow* window, int w, int h)
	{
		width = w;
		heigth = h;
	});

	// init
	for (int i = 0; i < 100; i++)
	{
		float x = std::rand() % 1000 * 0.002 - 1;
		float y = std::rand() % 1000 * 0.002 - 1;
		float z = std::rand() % 1000 * 0.001 + 1;
		pos.push_back(glm::vec3(x, y, z));
	}

	while (!glfwWindowShouldClose(win))
	{
		render();
		vrpnTracker->mainloop();
		glfwSwapBuffers(win);
		glfwPollEvents();
	}
	return 0;
}