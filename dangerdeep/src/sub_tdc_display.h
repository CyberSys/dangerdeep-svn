// user display: submarine's tdc
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_TDC_DISPLAY_H
#define SUB_TDC_DISPLAY_H

#include "user_display.h"
#include <vector>
using namespace std;

class sub_tdc_display : public user_display
{
	class rotat_tex {
	public:
		rotat_tex();
		~rotat_tex();
		texture* tex;
		int left, top, centerx, centery;
		void draw(double angle) const;
		void set(const char* filename, int left, int top, int centerx, int centery);
	protected:
		rotat_tex(const rotat_tex& );
		rotat_tex& operator= (const rotat_tex& );
	};

	class scheme {
	public:
		class image* background;
		rotat_tex clockbig;
		rotat_tex clocksml;
		rotat_tex targetcourse;
		rotat_tex targetrange;
		rotat_tex targetspeed;
		rotat_tex spreadangle;
		rotat_tex targetpos;
		// everything that does not rotate could also be an "image"...
		// but only when this doesn't trash the image cache
		texture* tubelight[6];
		texture* tubeswitch[6];
		texture* firebutton;
		texture* automode[2];	// on/off
		rotat_tex gyro_a;
		rotat_tex gyro_v;
		texture* firesolutionquality;
		rotat_tex torpspeed;
		scheme();
		~scheme();
	protected:
		scheme(const scheme& );
		scheme& operator= (const scheme& );
	};

	scheme normallight, nightlight;

	unsigned selected_tube;	// 0-5
	unsigned selected_mode;	// 0-1 (automatic on / off)

public:
	sub_tdc_display(class user_interface& ui_);
	virtual ~sub_tdc_display();

	//overload for zoom key handling ('y') and TDC input
	virtual void process_input(class game& gm, const SDL_Event& event);
	virtual void display(class game& gm) const;
};

#endif