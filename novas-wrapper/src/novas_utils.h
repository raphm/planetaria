#ifndef NOVAS_UTILS_H
#define NOVAS_UTILS_H

#include <tuple>
#include <string>
#include <limits>

#include "astro_time.h"
#include "novas_wrapper.h"

template <typename E>
constexpr auto to_underlying (E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}

enum class moon_phase {
    full_moon = 0,
    waxing_gibbous = 1,
    first_quarter = 2,
    waxing_crescent = 3,
    new_moon = 4,
    waning_crescent = 5,
    last_quarter = 6,
    waning_gibbous = 7
};

static std::string moon_phase_str (moon_phase mp) {
    switch (mp) {
        case moon_phase::full_moon:
            return "FULL";
        case moon_phase::waxing_gibbous:
            return "WAXING_GIBBOUS";
        case moon_phase::first_quarter:
            return "FIRST_QUARTER";
        case moon_phase::waxing_crescent:
            return "WAXING_CRESCENT";
        case moon_phase::new_moon:
            return "NEW";
        case moon_phase::waning_crescent:
            return "WANING_CRESCENT";
        case moon_phase::last_quarter:
            return "LAST_QUARTER";
        case moon_phase::waning_gibbous:
            return "WANING_GIBBOUS";
    }
    return "UNKNOWN";
}

enum class planet_event_type {
    rise,
    upper_culmination, // "Transit", i.e., sun at noon; if el is negative, no rise that day
    set,
    lower_culmination  // "Transit below pole", i.e., sun at midnight; if el is positive, no set that day
};

static std::string planet_event_type_str (planet_event_type pe) {
    switch (pe) {
        case planet_event_type::rise:
            return "RISE";
        case planet_event_type::set:
            return "SET";
        case planet_event_type::upper_culmination:
            return "UPPER_CULMINATION";
        case planet_event_type::lower_culmination:
            return "LOWER_CULMINATION";
    }
    return "UNKNOWN";
}

struct planetary_event {
    double event_time;
    novas_wrapper::horizon_coords hc;
    sky_pos pos;
    planet_event_type event;
};

namespace novas_utils {
		
    /*
    * Load a planet's equatorial coordinates; useful for getting apparent geocentric
    * position of planet as right ascension, declination, and distance. RA and declination
    * can be converted to ecliptic coordinates using equ2ecl; resulting ecliptic
    * coordinates can be used for obtaining zodiac sign of planets.
    */
    sky_pos load_planet_geocentric_equatorial (astro_time & lookup_time, novas_planet planet);

    /*
    * Load planet's astrometric coordinates (no light deflection, no abberation)
    */
    sky_pos load_planet_astro (astro_time & lookup_time, novas_planet planet);

    /*
    * Calculate the equatorial spherical coordinates of the solar transit point with
    * respect to the center of the Earth-facing surface of the Moon.
    * Because of tidal locking, the Moon always points the same face to Earth.
    * The center of this face is the phase origin. When the phase is {0,0}, it
    * means that the Earth and Sun transit points on the Moon are coincident.
    * Positive phase latitudes mean the Sun transit point is north of the Earth
    * transit point. Positive phase longitudes mean the Sun transit point is east
    * of the Sun transit point.
    *
    * Input arguments:
    *   tp - pointer to the time_parameters_t structure containing the Julian Date
    *        and TT for the moment of interest.
    *   sun - pointer to the object structure initialized for the Sun
    *   moon - pointer to the object structure initialized for the Moon
    *   accuracy - Selection for accuracy (0=full, 1=reduced)
    * Output arguments:
    *   phlat - pointer to a double that will be assigned the phase latitude.
    *           This is the number of degrees north of the Earth transit point
    *           on the Moon.
    *   phlon - pointer to a double that will be assigned the phase longitude.
    *           This is the number of degrees east of the Earth transit point
    *           on the Moon.
    *   phindex - pointer to an int that will be assigned a number from 0 to 7,
    *           corresponding to one of the common names of the Moon phases.
    *           Use moon_phase_names[phindex] to obtain a string name for the
    *           Moon phase.
    */

    struct moon_information {
        double sun_earth_angle;
        double sun_earth_angle_long;
        double sun_earth_angle_lat;
        double percent_illumination;
        moon_phase phase;
    };

    moon_information get_moon_phase (astro_time & lookup_time);

    /*
    * Return an object to use in novas_wrapper::w_place or other calls that take an object.
    */
    object build_planet_object (novas_planet planet);

    /*
    * Return local apparent sidereal time
    */
    double get_local_apparent_sidereal_time (astro_time & lookup_time, double longitude);

   /*
    * Determines rise, upper_culmination, set, or lower_culmination based on:
    *   hc (which should be adjusted for object diameter thusly: (hc.zd + moon_app_diameter/2.0)
    *   event_time
    *   location of observer on surface
    */
    planet_event_type determine_planetary_event_type (novas_wrapper::horizon_coords hc, astro_time event_time, on_surface geo_loc);

    /*
    * Find planet events (rise, set, transit, etc.)
    */
    std::vector<planetary_event> find_planetary_events (const double julian_utc_begin, const double julian_utc_end, novas_planet planet, on_surface geo_loc);

    /*
    * Find new and full moons
    */
    std::vector<astro_time> find_new_and_full_moons (double jd_utc_beg, double jd_utc_end);

};

#endif