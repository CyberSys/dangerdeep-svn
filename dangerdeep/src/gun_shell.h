// gun shells
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef GUN_SHELL_H
#define GUN_SHELL_H

#include "sea_object.h"

#define GUN_SHELL_HITPOINTS 1	// fixme
#define AIR_RESISTANCE 2.0	// m/sec^2
#define GRAVITY 9.806
#define GUN_SHELL_INITIAL_VELOCITY	400	// m/sec

class gun_shell : public sea_object
{
protected:
	double v0;	// inititial velocity
	double t;	// flying time
	angle a;	// elevation angle

	gun_shell();
	gun_shell& operator=(const gun_shell& other);
	gun_shell(const gun_shell& other);
public:
	virtual ~gun_shell() {};
	gun_shell(const sea_object& parent, angle direction, angle elevation,
		double initial_velocity = GUN_SHELL_INITIAL_VELOCITY);
	virtual void simulate(class game& gm, double delta_time);
	virtual void display(void) const;
};

#endif
