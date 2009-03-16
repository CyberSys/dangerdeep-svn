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

// game
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef GAME_H
#define GAME_H

#define PINGREMAINTIME 1.0	// seconds
#define PINGANGLE 15		// angle
#define PINGLENGTH 1000		// meters. for drawing
#define ASDICRANGE 1500.0	// meters fixme: historic values?
#define MAX_ACUSTIC_CONTACTS 5	// max. nr of simultaneous trackable acustic contacts
#define TERRAIN_NR_LEVELS	7
#define TERRAIN_RESOLUTION_N	6

#include <list>
#include <vector>
#include "thread.h"
#include "mutex.h"
#include "condvar.h"

// use forward declarations to avoid unneccessary compile dependencies
class ship;
class submarine;
class airplane;
class torpedo;
class depth_charge;
class gun_shell;
class model;
class global_data;
class sea_object;
class network_connection;
class water_splash;
class particle;
class convoy;
class water;
class height_generator;

#include "angle.h"
#include "date.h"
#include "vector2.h"
#include "vector3.h"
#include "color.h"
#include "logbook.h"
#include "ptrset.h"
#include "xml.h"
#include "sensors.h"
#include "sonar.h"
#include "event.h"
#include "ptrlist.h"

// Note! do NOT include user_interface here, class game MUST NOT call any method
// of class user_interface or its heirs.

// network messages
#define MSG_length	16
#define MSG_cancel	"DFTD-cancel!    "
#define MSG_ask		"DFTD-ask?       "
#define MSG_offer	"DFTD-offer!     "
#define MSG_join	"DFTD-join?      "
#define MSG_joined	"DFTD-joined!    "
#define MSG_initgame	"DFTD-init!      "
#define MSG_ready	"DFTD-ready!     "
#define MSG_start	"DFTD-start!     "
#define MSG_gamestate	"DFTD-gamestate: "
#define MSG_command	"DFTD-command:   "


///\brief Central object of the game world with physics simulation etc.
class game
{
public:
	// fixme: may be redundant with event_ping !
	struct ping {
		vector2 pos;
		angle dir;
		double time;
		double range;
		angle ping_angle;
		ping(const vector2& p, angle d, double t, double range,
			const angle& ping_angle_ ) :
			pos(p), dir(d), time(t), range ( range ), ping_angle ( ping_angle_ )
			{}
		~ping() {}
		ping(const xml_elem& parent);
		void save(xml_elem& parent) const;
	};

	struct sink_record {
		date dat;
		std::string descr;	// fixme: store type, use a static ship function to retrieve a matching description, via specfilename!
		std::string mdlname;	// model file name string
		std::string specfilename; // spec file name (base model name)
		std::string layoutname; // model skin
		unsigned tons;
		sink_record(date d, const std::string& s, const std::string& m,
			    const std::string& sn, const std::string& ln, unsigned t)
			: dat(d), descr(s), mdlname(m), specfilename(sn), layoutname(ln), tons(t) {}
		sink_record(const xml_elem& parent);
		void save(xml_elem& parent) const;
	};
	
	struct job {
		job() {}
		virtual void run() = 0;
		virtual double get_period() const = 0;
		virtual ~job() {}
	};

	struct player_info {
		std::string name;
		unsigned flotilla;
		std::string submarineid;
		std::string photo;
		
		std::string soldbuch_nr;
		std::string gasmask_size;
		std::string bloodgroup;
		std::string marine_roll;
		std::string marine_group;
		/* 'cause the career list is linear we do not need to store 
		 * ranks or paygroups. a list of the dates should be enough
		 */
		std::list<std::string> career;
		
		player_info();
		player_info(const xml_elem& parent);
		void save(xml_elem& parent) const;
	};

	// in which state is the game
	// normal mode (running), or stop on next cycle (reason given by value)
	enum run_state { running, player_killed, mission_complete, contact_lost };
	
	// time between records of trail positions
	static const double TRAIL_TIME;

protected:
	// begin [SAVE]
	ptrset<ship> ships;
	ptrset<submarine> submarines;
	ptrset<airplane> airplanes;
	ptrset<torpedo> torpedoes;
	ptrset<depth_charge> depth_charges;
	ptrset<gun_shell> gun_shells;
	ptrset<water_splash> water_splashes;
	ptrset<convoy> convoys;
	ptrset<particle> particles;
	// end [SAVE]
	run_state my_run_state;

	ptrlist<event> events;
	
	std::list<std::pair<double, job*> > jobs;	// generated by interface construction, no gameplay data
	
	// the player (note that playing is not limited to submarines!)
	sea_object* player;	// [SAVE]

	std::list<sink_record> sunken_ships;	// [SAVE]

	logbook players_logbook;	// [SAVE]
	
	double time;	// global time (in seconds since 1.1.1939, 0:0 hrs) (universal time!) [SAVE]
	double last_trail_time;	// for position trail recording	[SAVE]

	date equipment_date;	// date that equipment was created. used for torpedo loading
	
	enum weathers { sunny, clouded, raining, storm };//fixme
	double max_view_dist;	// maximum visibility according to weather conditions, fixme recomputed or save?
	
	std::list<ping> pings;	// [SAVE]
	
	// network game type (0 = single player, 1 = server, 2 = client)
	unsigned networktype;	// [SAVE] later!
	// the connection to the server (zero if this is the server)
	network_connection* servercon;	// [SAVE] later!
	// the connections to the clients (at least one if this is the server, else empty)
	std::vector<network_connection*> clientcons;	// [SAVE] later!

	// time in milliseconds that game is paused between simulation steps.
	// for small pauses to compensate long image loading times
	unsigned freezetime, freezetime_start;

	// water height data, and everything around it.
	std::auto_ptr<water> mywater;

	// terrain height data
	std::auto_ptr<height_generator> myheightgen;

	// multi-threading helper for simulation
	void simulate_objects_mt(double delta_t, unsigned idxoff, unsigned idxmod, bool record,
				 double& nearest_contact);

	class simulate_worker : public thread
	{
		mutex mtx;
		condvar cond;
		condvar condfini;
		game& gm;
		double delta_t;
		unsigned idxoff;
		unsigned idxmod;
		bool record;
		double nearest_contact;
		bool done;
	public:
		simulate_worker(game& gm_);
		void loop();
		void request_abort();
		void work(double dt, unsigned io, unsigned im, bool record);
		double sync();
	};

	thread::auto_ptr<simulate_worker> myworker;

	player_info playerinfo;

	/// check objects collide with any other object
	void check_collisions();
	bool check_collision_bboxes(const sea_object& a, const sea_object& b, vector3& collision_pos);
	void collision_response(sea_object& a, sea_object& b, const vector3& collision_pos);

	game();	
	game& operator= (const game& other);
	game(const game& other);

public:
	// create new custom mission
	// expects: size small,medium,large, escort size none,small,medium,large,
	// time of day [0,4) night,dawn,day,dusk
	game(const std::string& subtype, unsigned cvsize, unsigned cvesc, unsigned timeofday,
	     const date& timeperioddate, const player_info& pi = player_info()/*fixme - must be always given*/, unsigned nr_of_players = 1);

	// create from mission file or savegame (xml file)
	game(const std::string& filename);

	virtual ~game();

	virtual void save(const std::string& savefilename, const std::string& description) const;
	static std::string read_description_of_savegame(const std::string& filename);

	void compute_max_view_dist();	// fixme - public?
	virtual void simulate(double delta_t);

	const std::list<sink_record>& get_sunken_ships() const { return sunken_ships; };
	const logbook& get_players_logbook() const { return players_logbook; }
	void add_logbook_entry(const std::string& s);
	double get_time() const { return time; };
	date get_date() const { return date(unsigned(time)); };
	date get_equipment_date() const { return equipment_date; }
	double get_max_view_distance() const { return max_view_dist; }
	/**
		This method is needed to verify for day and night mode for the
		display methods within the user interfaces.
		@return true when day mode, false when night mode
	*/
	bool is_day_mode () const;
	/**
		This method calculates a depth depending factor. A deep diving
		submarine is harder to detect with ASDIC than a submarine at
		periscope depth.
		@param sub location vector of submarine
		@return depth factor
	*/
	virtual double get_depth_factor ( const vector3& sub ) const;
	
	sea_object* get_player() const { return player; }

	double get_last_trail_record_time() const { return last_trail_time; }

	// compute visibility data
	//fixme: remove the single functions, they're always called together
	//by visible_sea/visible_surface objects
	//they all map to the same function.
	//if certain objects should not be reported, unmask them with extra-parameter.
	virtual std::vector<ship*> visible_ships(const sea_object* o) const;
	virtual std::vector<submarine*> visible_submarines(const sea_object* o) const;
	virtual std::vector<airplane*> visible_airplanes(const sea_object* o) const;
	virtual std::vector<torpedo*> visible_torpedoes(const sea_object* o) const;
	virtual std::vector<depth_charge*> visible_depth_charges(const sea_object* o) const;
	virtual std::vector<gun_shell*> visible_gun_shells(const sea_object* o) const;
	virtual std::vector<water_splash*> visible_water_splashes(const sea_object* o) const;
	virtual std::vector<particle*> visible_particles (const sea_object* o ) const;
	// computes visible ships, submarines (surfaced) and airplanes
	virtual std::vector<sea_object*> visible_surface_objects(const sea_object* o) const;
	// computes ships, subs (surfaced), airplanes, torpedoes. But not fast moving objects
	// like shells/DCs, because they need to be detected more often, and this function
	// is called once per second normally.
	virtual std::vector<sea_object*> visible_sea_objects(const sea_object* o) const;

	// fixme: maybe we should distuingish between passivly and activly detected objects...
	// passivly detected objects should store their noise source as position and not their
	// geometric center position!
	virtual std::vector<sonar_contact> sonar_ships(const sea_object* o) const;
	virtual std::vector<sonar_contact> sonar_submarines(const sea_object* o) const;
	virtual std::vector<sonar_contact> sonar_sea_objects(const sea_object* o) const;
	// fixme: return sonar_contact here (when the noise_pos fix is done...)
	virtual ship* sonar_acoustical_torpedo_target(const torpedo* o) const;
	
	// std::list<*> radardetected_ships(...);	// later!
	virtual std::vector<submarine*> radar_submarines(const sea_object* o) const;
	virtual std::vector<ship*> radar_ships(const sea_object* o) const;
	//virtual std::vector<airplane*> radar_airplanes(const sea_object* o) const;
	virtual std::vector<sea_object*> radar_sea_objects(const sea_object* o) const;

	///\brief compute sound strengths caused by all ships
	/** @param	listener		object that listens via passive sonar
	    @passive	listening_direction	direction for listening
	    @return	absolute freq. strength in dB and noise struct of received noise frequencies (in dB)
	*/    
	std::pair<double, noise> sonar_listen_ships(const ship* listener, angle listening_direction) const;

	// append objects to vector
	template<class T>
	static void append_vec(std::vector<sea_object*>& vec, const std::vector<T*>& vec2) {
		for (unsigned i = 0; i < vec2.size(); ++i)
			vec.push_back(vec2[i]);
	}

	std::vector<vector2> convoy_positions() const;	// fixme
	
	// create new objects
	void spawn_ship(ship* s);
	void spawn_submarine(submarine* u);
	void spawn_airplane(airplane* a);
	void spawn_torpedo(torpedo* t);
	void spawn_gun_shell(gun_shell* s, const double &calibre);
	void spawn_depth_charge(depth_charge* dc);
	void spawn_water_splash(water_splash* ws);
	void spawn_convoy(convoy* cv);
	void spawn_particle(particle* pt);

	// simulation events
	void dc_explosion(const depth_charge& dc);	// depth charge exploding
	void torp_explode(const torpedo *t);	// torpedo explosion/impact
	void ship_sunk( const ship* s );	// a ship sinks

	// simulation actions, fixme send something over net for them, fixme : maybe vector not list?
	virtual void ping_ASDIC(std::list<vector3>& contacts, sea_object* d,
		const bool& move_sensor, const angle& dir = angle ( 0.0f ) );

	// various functions (fixme: sort, cleanup)
	void register_job(job* j);	// insert/remove job in job list
	void unregister_job(job* j);
	const std::list<ping>& get_pings() const { return pings; };	// fixme: maybe vector not list

	template<class C> ship* check_units ( torpedo* t, const ptrset<C>& units );

	// fixme why is this not const? if it changes game, it must be send over network, and
	// then it can't be a function!
	bool check_torpedo_hit(torpedo* t, bool runlengthfailure, bool failure);

	// dito, see check_torpedo_hit-comment
	sea_object* contact_in_direction(const sea_object* o, const angle& direction) const;
	ship* ship_in_direction_from_pos(const sea_object* o, const angle& direction) const;
	submarine* sub_in_direction_from_pos(const sea_object* o, const angle& direction) const;

	const torpedo* get_torpedo_for_camera_track(unsigned nr) const;

	bool is_collision(const sea_object* s1, const sea_object* s2) const;
	bool is_collision(const sea_object* s, const vector2& pos) const;

	// is editor?
	virtual bool is_editor() const { return false; }

	// sun/moon and light color/brightness
	double compute_light_brightness(const vector3& viewpos, vector3& sundir) const;	// depends on sun/moon
	colorf compute_light_color(const vector3& viewpos) const;	// depends on sun/moon
	vector3 compute_sun_pos(const vector3& viewpos) const;
	vector3 compute_moon_pos(const vector3& viewpos) const;

	/// compute height of water at given world space position.
	double compute_water_height(const vector2& pos) const;

	// Translate pointers to numbers and vice versa. Used for load/save
	sea_object* load_ptr(unsigned nr) const;
	ship* load_ship_ptr(unsigned nr) const;
	convoy* load_convoy_ptr(unsigned nr) const;
	unsigned save_ptr(const sea_object* s) const;
	unsigned save_ptr(const convoy* c) const;

	void freeze_time();
	void unfreeze_time();

	void add_event(event* e) { events.push_back(e); }
	const ptrlist<event>& get_events() const { return events; }
	run_state get_run_state() const { return my_run_state; }
	unsigned get_freezetime() const { return freezetime; }
	unsigned get_freezetime_start() const { return freezetime_start; }
	unsigned process_freezetime() { unsigned f = freezetime; freezetime = 0; return f; }

	water& get_water() { return *mywater.get(); }
	const water& get_water() const { return *mywater.get(); }

	height_generator& get_height_gen() { return *myheightgen.get(); }
	const height_generator& get_height_gen() const { return *myheightgen.get(); }

	/// get pointers to all ships for collision tests.
	std::vector<ship*> get_all_ships() const;

	virtual const player_info& get_player_info() const { return playerinfo; }
};

#endif
