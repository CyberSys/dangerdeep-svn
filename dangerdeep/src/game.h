// game
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef GAME_H
#define GAME_H

#define PINGREMAINTIME 1.0	// seconds
#define PINGANGLE 15		// angle
#define PINGLENGTH 1000		// meters. for drawing
#define ASDICRANGE 1500.0	// meters fixme: historic values?
#define MAX_ACUSTIC_CONTACTS 5	// max. nr of simultaneous trackable acustic contacts

#ifdef WIN32
#pragma warning (disable : 4786)
#endif

#include <list>
#include <vector>
using namespace std;

// use forward declarations to avoid unneccessary compile dependencies
class ship;
class airplane;
class torpedo;
class depth_charge;
class gun_shell;
class model;
class global_data;
class parser;
class sea_object;
class water_splash;

#include "submarine.h"
#include "convoy.h"
#include "angle.h"
#include "date.h"
#include "vector2.h"
#include "vector3.h"

class game	// our "world" with physics etc.
{
	friend void show_results_for_game(const game* );
	friend void check_for_highscore(const game* );
	
public:
	struct ping {
		vector2 pos;
		angle dir;
		double time;
		double range;
		angle ping_angle;
		ping(const vector2& p, angle d, double t, const double& range,
			const angle& ping_angle_ ) :
			pos(p), dir(d), time(t), range ( range ), ping_angle ( ping_angle_ )
			{}
		~ping() {}
		ping(istream& in);
		void save(ostream& out) const;
	};

	struct sink_record {
		date dat;
		string descr;	// fixme: store type, use a static ship function to retrieve a matching description
		unsigned tons;
		sink_record(date d, const string& s, unsigned t) : dat(d), descr(s), tons(t) {}
		sink_record(const sink_record& s) : dat(s.dat), descr(s.descr), tons(s.tons) {}
		~sink_record() {}
		sink_record& operator= (const sink_record& s) { dat = s.dat; descr = s.descr; tons = s.tons; return *this; }
		sink_record(istream& in);
		void save(ostream& out) const;
	};
	
	struct job {
		job() {}
		virtual void run(void) = 0;
		virtual double get_period(void) const = 0;
		virtual ~job() {}
	};
	
protected:
	list<ship*> ships;
	list<submarine*> submarines;
	list<airplane*> airplanes;
	list<torpedo*> torpedoes;
	list<depth_charge*> depth_charges;
	list<gun_shell*> gun_shells;
	list<convoy*> convoys;
	list<water_splash*> water_splashs;
	bool running;	// if this is false, the player was killed
	bool stopexec;	// if this is true, execution stops and the menu is displayed
	
	list<pair<double, job*> > jobs;	// generated by interface construction, no gameplay data
	
	// the player and matching ui (note that playing is not limited to submarines!)
	sea_object* player;
	class user_interface* ui;

	list<sink_record> sunken_ships;
	
	double time;	// global time (in seconds since 1.1.1939, 0:0 hrs)
	double last_trail_time;	// for position trail recording
	
	enum weathers { sunny, clouded, raining, storm };//fixme
	double max_view_dist;	// maximum visibility according to weather conditions
	
	list<ping> pings;
	
	game& operator= (const game& other);
	game(const game& other);
	
	unsigned listsizes(unsigned n) const;	// counts # of list elemens over n lists above

public:
	// expects: size small,medium,large, escort size none,small,medium,large,
	// time of day [0,4) night,dawn,day,dusk
	game(submarine::types subtype, unsigned cvsize, unsigned cvesc, unsigned timeofday);
	game(parser& p);
	virtual ~game();

	// game types: mission, career, multiplayer mission, multiplayer career/patrol (?)
	// fixme: add partial_load flag for network vs. savegame (full save / variable save)
	// maybe only needed for loading (build structure or not)
	virtual void save(const string& savefilename, const string& description) const;
	game(const string& savefilename);	// load a game
	static string read_description_of_savegame(const string& filename);
	virtual void save_to_stream(ostream& out) const;
	virtual void load_from_stream(istream& in);

	void stop(void) { stopexec = true; }

	void compute_max_view_dist(void);
	void simulate(double delta_t);

	double get_time(void) const { return time; };
	double get_max_view_distance(void) const { return max_view_dist; }
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

	// compute visibility data
	// fixme: gcc3.2+ optimizes return values that are complex data types.
	// hence change signature to list<xxx*> visible_xxxs(const...);
	virtual void visible_ships(list<ship*>& result, const sea_object* o);
	virtual void visible_submarines(list<submarine*>& result, const sea_object* o);
	virtual void visible_airplanes(list<airplane*>& result, const sea_object* o);
	virtual void visible_torpedoes(list<torpedo*>& result, const sea_object* o);
	virtual void visible_depth_charges(list<depth_charge*>& result, const sea_object* o);
	virtual void visible_gun_shells(list<gun_shell*>& result, const sea_object* o);
	virtual void visible_water_splashes ( list<water_splash*>& result, const sea_object* o );

	virtual void sonar_ships ( list<ship*>& result, const sea_object* o );
	virtual void sonar_submarines ( list<submarine*>& result, const sea_object* o );
	virtual ship* sonar_acoustical_torpedo_target ( const torpedo* o );
	
	// list<*> radardetected_ships(...);	// later!

	void convoy_positions(list<vector2>& result) const;	// fixme
	
//	bool can_see(const sea_object* watcher, const submarine* sub) const;

	// create new objects
	void spawn_ship(ship* s);
	void spawn_submarine(submarine* u);
	void spawn_airplane(airplane* a);
	void spawn_torpedo(torpedo* t);
	void spawn_gun_shell(gun_shell* s);
	void spawn_depth_charge(depth_charge* dc);
	void spawn_convoy(convoy* cv);
	void spawn_water_splash ( water_splash* ws );

	// simulation events
//fixme: send messages about them to ui (remove sys-console printing in torpedo.cpp etc)
	void dc_explosion(const depth_charge& dc);	// depth charge exploding
	bool gs_impact(const vector3& pos);	// gun shell impact
	void torp_explode(const vector3& pos);	// torpedo explosion/impact
	void ship_sunk( const ship* s );	// a ship sinks

	// simulation actions
	virtual void ping_ASDIC(list<vector3>& contacts, sea_object* d,
		const bool& move_sensor, const angle& dir = angle ( 0.0f ) );

	// various functions (fixme: sort, cleanup)
	void register_job(job* j);	// insert/remove job in job list
	void unregister_job(job* j);
	const list<ping>& get_pings(void) const { return pings; };

	template<class _C>
	ship* check_unit_list ( torpedo* t, list<_C>& unit_list );

	bool check_torpedo_hit(torpedo* t, bool runlengthfailure, bool failure);

	sea_object* contact_in_direction(const sea_object* o, const angle& direction);
	ship* ship_in_direction_from_pos(const sea_object* o, const angle& direction);
	submarine* sub_in_direction_from_pos(const sea_object* o, const angle& direction);

	bool is_collision(const sea_object* s1, const sea_object* s2) const;
	bool is_collision(const sea_object* s, const vector2& pos) const;

	double water_depth(const vector2& pos) const;
	
	// returns 0 for show menu, 1 for exit game
	unsigned exec(void);

	// Translate pointers to numbers and vice versa. Used for load/save
	void write(ostream& out, const ship* s) const;
	void write(ostream& out, const submarine* s) const;
	void write(ostream& out, const airplane* s) const;
	void write(ostream& out, const torpedo* s) const;
	void write(ostream& out, const depth_charge* s) const;
	void write(ostream& out, const gun_shell* s) const;
	void write(ostream& out, const convoy* s) const;
	void write(ostream& out, const water_splash* s) const;
	void write(ostream& out, const sea_object* s) const;
	ship* read_ship(istream& in) const;
	submarine* read_submarine(istream& in) const;
	airplane* read_airplane(istream& in) const;
	torpedo* read_torpedo(istream& in) const;
	depth_charge* read_depth_charge(istream& in) const;
	gun_shell* read_gun_shell(istream& in) const;
	convoy* read_convoy(istream& in) const;
	water_splash* read_water_splash(istream& in) const;
	sea_object* read_sea_object(istream& in) const;
};

#endif
