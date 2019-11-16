#ifndef NOVAS_WRAPPER_H
#define NOVAS_WRAPPER_H


extern "C"
{
#include "eph_manager.h"
#include "novas.h"
}

#include <tuple>

#include "astro_time.h"

// See novas.c make_object() definition for an explanation of these constants
enum class novas_planet_id {
	MERCURY = 1,
	VENUS = 2,
	EARTH = 3,
	MARS = 4,
	JUPITER = 5,
	SATURN = 6,
	URANUS = 7,
	NEPTUNE = 8,
	PLUTO = 9,
	SUN = 10,
	MOON = 11
};

struct novas_planet {
	novas_planet_id id;
	std::string name;
	double diameter_km;
};

namespace novas_constants {

	const novas_planet MERCURY = { novas_planet_id::MERCURY,   "Mercury", 4879.0 };
	const novas_planet VENUS = { novas_planet_id::VENUS,     "Venus",   12104.0 };
	const novas_planet EARTH = { novas_planet_id::EARTH,     "Earth",   (12713.6 + 12756.2) / 2.0 };
	const novas_planet MARS = { novas_planet_id::MARS,      "Mars",    (6752.4 + 6792.4) / 2.0 };
	const novas_planet JUPITER = { novas_planet_id::JUPITER,   "Jupiter", (133708 + 142984) / 2.0 };
	const novas_planet SATURN = { novas_planet_id::SATURN,    "Saturn",  (108728 + 120536) / 2.0 };
	const novas_planet URANUS = { novas_planet_id::URANUS,    "Uranus",  (49946 + 51118) / 2.0 };
	const novas_planet NEPTUNE = { novas_planet_id::NEPTUNE,   "Neptune", (48682 + 49528) / 2.0 };
	const novas_planet PLUTO = { novas_planet_id::PLUTO,     "Pluto",   2370.0 };
	const novas_planet SUN = { novas_planet_id::SUN,      "Sun",     1391978.0 };
	const novas_planet MOON = { novas_planet_id::MOON,     "Moon",    3474.0 };

	static const std::vector<novas_planet> all_planets = {
		novas_constants::SUN,
		novas_constants::MOON,
		novas_constants::MERCURY,
		novas_constants::VENUS,
		novas_constants::MARS,
		novas_constants::JUPITER,
		novas_constants::SATURN,
		novas_constants::URANUS,
		novas_constants::NEPTUNE,
		novas_constants::PLUTO
	};

	const short accuracy = 1;     // 0 ... full accuracy, 1 ... reduced accuracy

	const short coord_gcrs = 0;   // 0 ... GCRS or "local GCRS"
	const short coord_equ = 1;    // 1 ... true equator and equinox of date
	const short coord_cio = 2;    // 2 ... true equator and CIO of date
	const short coord_astro = 3;  // 3 ... astrometric coordinates, i.e., without light deflection or aberration.

	const short refraction = 1;   	// 0: no refraction; 1: include refraction, using 'standard' atmospheric conditions; 2: include refraction, using atmospheric parameters input in the 'location' structure.

}

namespace novas_wrapper {

	// w_cel2ter: rotates a vector from the celestial to the terrestrial system. Specifically, it transforms a vector 
	// in the GCRS (a local space-fixed system) to the ITRS (a rotating earth-fixed system) by applying rotations for
	// the GCRS-to-dynamical frame tie, precession, nutation, Earth rotation, and polar motion.
	//
	// INPUT:
	//   lookup_time (astro_time):            time at which to perform rotation
	//   method (short):                      rotation method:
	//                                           0: CIO-based method
	//                                           1: equinox-based method
	//   accuracy (short):                    relative accuracy of the output position: 
	//                                           0: full accuracy
	//                                           1: reduced accuracy
	//   option (short):                      option for rotation:
	//                                           0: input vector is referred to GCRS axes
	//                                           1: input vector is produced with respect to the equator and equinox of date
	//   xp (double):                         conventionally-defined X coordinate of celestial intermediate pole with respect to ITRS pole, in arcseconds
	//   yp (double):                         conventionally-defined Y coordinate of celestial intermediate pole with respect to ITRS pole, in arcseconds
	//   vec1[3] (double):                    position vector, geocentric equatorial rectangular coordinates, referred to GCRS axes (celestial system) or 
	//                                           equator/equinox of date, depending on 'option'
	//
	// OUTPUT:
	//   std::tuple<double, double, double>:  x/y/z position vector, geocentric equatorial rectangular coordinates, referred to ITRS axes(terrestrial system)

	std::tuple<double, double, double> w_cel2ter (astro_time& lookup_time, short method, short accuracy, short option, double xp, double yp, double vec1[3]);
	
	// w_equ2ecl: convert right ascension and declination to ecliptic longitude and latitude
	//
	// INPUT:
	//   lookup_time (astro_time):     Julian date of equator, equinox, and ecliptic used for coordinates
	//   coord_sys (short):            coordinate system selection:
	//                                    0: mean equator and equinox of date 'lookup_time'
	//                                    1: true equator and equinox of date 'lookup_time'
	//                                    2: ICRS
	//                                    (ecliptic is always the mean plane)
	//   accuracy (short):             relative accuracy of the output position: 
	//                                    0: full accuracy
	//                                    1: reduced accuracy
	//   ra (double):                  right ascension in hours, referred to specified equator and equinox of date
	//   dec (double):                 declination in degrees, referred to specified equator and equinox of date
	//
	// OUTPUT:
	//   std::tuple<double, double>:   ecliptic longitude and latitude in degrees, referred to specified ecliptic and equinox of date

	std::tuple<double, double> w_equ2ecl (astro_time& lookup_time, short coord_sys, short accuracy, double ra, double dec);

	// w_equ2hor: 	transforms topocentric right ascension and declination to zenith distance and azimuth. It uses a method that properly
	// accounts for polar motion, which is significant at the sub-arcsecond level. This function can also adjust coordinates for 
	// atmospheric refraction.
	//
	// INPUT:
	//   lookup_time (astro_time):     Julian date of equator, equinox, and ecliptic used for coordinates
	//   accuracy (short):             relative accuracy of the output position: 
	//                                    0: full accuracy
	//                                    1: reduced accuracy
	//   xp (double):                  conventionally-defined X coordinate of celestial intermediate pole with respect to ITRS pole, in arcseconds
	//   yp (double):                  conventionally-defined Y coordinate of celestial intermediate pole with respect to ITRS pole, in arcseconds
	//   location (observer):          location of observer
	//   ra (double):                  topocentric right ascension in hours, referred to true equator and equinox of date
	//   dec (double):                 topocentric declination in degrees, referred to true equator and equinox of date
	//   ref_option (short):           atmosphere refraction to simulate:
	//                                    0: no refraction
	//                                    1: include refraction, using 'standard' atmospheric conditions
	//                                    2: include refraction, using atmospheric parameters input in the 'location' structure.
	//
	// OUTPUT:
	//   

	struct horizon_coords {
		double zd; // Topocentric zenith distance in degrees
		double az; // Topocentric azimuth (measured east from north) in degrees.
		double rar; // Topocentric right ascension of object of interest, in hours
		double decr; // topocentric declination of object of interest, in degrees
	};

	horizon_coords w_equ2hor (astro_time& lookup_time, sky_pos t_place, short accuracy, double x_pole, double y_pole, on_surface& geo_loc, short ref_option);


	// w_place: computes the apparent direction of a star or solar system body at a specified time and in a specified coordinate system.
	//
	// INPUT:
	//   lookup_time (astro_time): time at which to perform lookup
	//   cel_object (object):      celestial object of interest
	//   location (observer):      location of observer
	//   coord_sys (short):        coordinate system of the output position: 
	//                                0: GCRS or "local GCRS"
	//                                1: true equator and equinox of date
	//                                2: true equator and CIO of date
	//                                3: astrometric coordinates (no light deflection/aberration)
	//   accuracy (short):         relative accuracy of the output position: 
	//                                0: full accuracy
	//                                1: reduced accuracy
	//
	// OUTPUT:
	//   sky_pos:                   object's place on the sky at time 'lookup_time', with respect to the specified output coordinate system

	sky_pos w_place (astro_time& lookup_time, object& cel_object, observer& location, short coord_sys, short accuracy);

	// w_make_object: Makes a structure of type 'object' - specifying a celestial object - based on the input parameters.
	//
	// INPUT:
	//   type (short):                     type of object
	//                                        0: major planet, Pluto, Sun, or Moon
	//                                        1: minor planet
	//                                        2: object located outside the solar system (e.g. star, galaxy, nebula, etc.)
	//   number (short):                   Body number
	//                                        For 'type' = 0: Mercury = 1,...,Pluto = 9, Sun = 10, Moon = 11
	//                                        For 'type' = 1: minor planet number
	//                                        For 'type' = 2: set to 0 (zero)
	//   name[SIZE_OF_OBJ_NAME] (char):    Name of the object ((SIZE_OF_OBJ_NAME - 1) characters maximum).
	//   star_data (cat_entry):            Structure containing basic astrometric data for any celestial object located outside the solar system.
	//
	// OUTPUT:
	//   object:                           structure containing the object definition

	object w_make_object (short int type, novas_planet_id number, std::string const& name, cat_entry& star_data);
};

#endif