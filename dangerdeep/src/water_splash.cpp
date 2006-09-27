/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// water splash (C)+(W) 2006 Thorsten Jordan

#include "water_splash.h"
#include "global_data.h"
#include "texture.h"
#include "game.h"

void water_splash::render_cylinder(double radius_bottom, double radius_top, double height,
				   double alpha, double u_scal, unsigned nr_segs)
{
	glBegin(GL_QUAD_STRIP);
	glColor4f(1, 1, 1, alpha);
	double us = u_scal / nr_segs;
	//fixme: use different alpha for bottom? like always full alpha?
	for (unsigned i = 0; i <= nr_segs; ++i) {
		double a = -2*M_PI*i/nr_segs;
		double sa = sin(a);
		double ca = cos(a);
		glColor4f(1,1,1,0.5+0.5*alpha);
		glTexCoord2f(i * us, 1);
		// compensate tide! so set z_lower to -1.5
		glVertex3f(radius_bottom * ca, radius_bottom * sa, -1.5);
		glColor4f(1, 1, 1, alpha);
		glTexCoord2f(i * us, 0);
		glVertex3f(radius_top * ca, radius_top * sa, height);
	}
	glEnd();
}



double water_splash::compute_height(double t) const
{
	if (t < risetime) {
		double x = (risetime - t)/risetime;
		return riseheight * (1.0 - x*x);
	} else {
		double x = (t - risetime)/falltime;
		return riseheight * (1.0 - x*x);
	}
}



water_splash::water_splash(game& gm_, const vector3& pos)
	: sea_object(gm_, "gun_shell.3ds" /* hack */)
{
	position = pos;
	std::vector<double> p(6);
	p[0] = 5.0;
	p[1] = 5.0;
	p[2] = 6.0;
	p[3] = 7.0;
	p[4] = 8.0;
	p[5] = 9.0;
	bradius_top.reset(new bspline(3, p));
	p[0] = 5.0;
	p[1] = 5.0;
	p[2] = 5.2;
	p[3] = 5.4;
	p[4] = 5.6;
	p[5] = 5.8;
	bradius_bottom.reset(new bspline(3, p));
	p[0] = 1.0;
	p[1] = 1.0;
	p[2] = 0.75;
	p[3] = 0.5;
	p[4] = 0.25;
	p[5] = 0.0;
	balpha.reset(new bspline(3, p));

	risetime = 0.4;
	riseheight = 25.0;
	falltime = sqrt(riseheight * 2.0 / GRAVITY);
	lifetime = risetime + falltime;
	resttime = lifetime;
}



void water_splash::simulate(double delta_time)
{
	sea_object::simulate(delta_time);
	if (is_defunct()) return;

	resttime -= delta_time;
	if (resttime <= -0.5)
		kill();
}



void water_splash::display() const
{
	// fixme: crude hack
	texturecache.ref("splashring.png")->set_gl_texture();

	glDisable(GL_LIGHTING);
	//glTranslate
	//render two cylinders...
	//alpha 0% at end, increase radius on fade, widen more
	try {
		if (lifetime - resttime > 0.5) {
			double t = (lifetime - resttime - 0.5)/lifetime;
			double rt = bradius_top->value(t) * 0.8;
			double rb = bradius_bottom->value(t) * 0.8;
			double a = balpha->value(t);
			render_cylinder(rb, rt, compute_height(lifetime - resttime - 0.5) * 1.2, a);
		}
		if (resttime > 0) {
			double t = (lifetime - resttime)/lifetime;
			double rt = bradius_top->value(t);
			double rb = bradius_bottom->value(t);
			double a = balpha->value(t);
			render_cylinder(rb, rt, compute_height(lifetime - resttime), a);
		}
	}
	catch (std::exception& e) {
		// do nothing.
	}
	glEnable(GL_LIGHTING);
}



void water_splash::display_mirror_clip() const
{
	display();
}