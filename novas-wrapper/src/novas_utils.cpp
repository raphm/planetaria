#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>

#include "zbrent.h"
#include "novas_wrapper.h"
#include "astro_calc.h"
#include "vec3.h"

#include "novas_utils.h"

const double finder_tolerance = std::numeric_limits<double>::epsilon() * 100;

object novas_utils::build_planet_object(novas_planet planet)
{

    char star_name[SIZE_OF_OBJ_NAME];
    char catalog[SIZE_OF_CAT_NAME];
    strcpy(star_name, "DUMMY");
    strcpy(catalog, "xxx");

    cat_entry dummy_star;
    const short planet_type = 0; // Type 0 = major planet, Pluto, Sun, or Moon
    make_cat_entry(star_name, catalog, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, &dummy_star);

    return novas_wrapper::w_make_object(planet_type, planet.id, planet.name, dummy_star);
}

sky_pos novas_utils::load_planet_astro(astro_time &lookup_time, novas_planet planet)
{

    // Makes a structure of type 'observer' specifying an observer at the geocenter.
    observer at_geocenter;
    make_observer_at_geocenter(&at_geocenter);

    char star_name[SIZE_OF_OBJ_NAME];
    char catalog[SIZE_OF_CAT_NAME];
    strcpy(star_name, "DUMMY");
    strcpy(catalog, "xxx");

    cat_entry dummy_star;
    const short planet_type = 0; // Type 0 = major planet, Pluto, Sun, or Moon
    object planet_obj = novas_wrapper::w_make_object(planet_type, planet.id, planet.name, dummy_star);

    make_cat_entry(star_name, catalog, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, &dummy_star);

    return novas_wrapper::w_place(lookup_time, planet_obj, at_geocenter, novas_constants::coord_astro, novas_constants::accuracy);
}

sky_pos novas_utils::load_planet_geocentric_equatorial(astro_time &lookup_time, novas_planet planet)
{

    // Makes a structure of type 'observer' specifying an observer at the geocenter.
    observer at_geocenter;
    make_observer_at_geocenter(&at_geocenter);

    char star_name[SIZE_OF_OBJ_NAME];
    char catalog[SIZE_OF_CAT_NAME];
    strcpy(star_name, "DUMMY");
    strcpy(catalog, "xxx");

    cat_entry dummy_star;
    const short planet_type = 0; // Type 0 = major planet, Pluto, Sun, or Moon
    object planet_obj = novas_wrapper::w_make_object(planet_type, planet.id, planet.name, dummy_star);

    make_cat_entry(star_name, catalog, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, &dummy_star);

    return novas_wrapper::w_place(lookup_time, planet_obj, at_geocenter, novas_constants::coord_equ, novas_constants::accuracy);
}

// Calculate the equatorial spherical coordinates of the solar transit point with
// respect to the center of the Earth-facing surface of the Moon.

novas_utils::moon_information novas_utils::get_moon_phase(astro_time &lookup_time)
{

    auto sun = build_planet_object(novas_constants::SUN);

    auto moon = build_planet_object(novas_constants::MOON);

    observer geo_ctr;

    double earth_sun[3];
    double moon_earth[3];
    double moon_sun[3];

    make_observer(0, NULL, NULL, &geo_ctr);

    const bool use_astro = false;
    const short coord_sys = (use_astro ? novas_constants::coord_astro : novas_constants::coord_equ);
    const short ecl_coord_sys = (use_astro ? 2 : 1);
    // coord_sys (short int)
    //    Coordinate system selection.
    //    = 0 ... mean equator and equinox of date 'jd_tt'
    //    = 1 ... true equator and equinox of date 'jd_tt'
    //    = 2 ... ICRS
    //    (ecliptic is always the mean plane)

    sky_pos sun_place = novas_wrapper::w_place(lookup_time, sun, geo_ctr, coord_sys, novas_constants::accuracy);
    radec2vector(sun_place.ra, sun_place.dec, sun_place.dis, earth_sun);
    sky_pos moon_place = novas_wrapper::w_place(lookup_time, moon, geo_ctr, coord_sys, novas_constants::accuracy);
    radec2vector(moon_place.ra, moon_place.dec, moon_place.dis, moon_earth);

    /* The vector points from Earth to Moon. Reverse it. */
    moon_earth[0] *= -1.0;
    moon_earth[1] *= -1.0;
    moon_earth[2] *= -1.0;

    /* Calculate the position vector of the Sun w/r/t Moon */
    moon_sun[0] = earth_sun[0] + moon_earth[0];
    moon_sun[1] = earth_sun[1] + moon_earth[1];
    moon_sun[2] = earth_sun[2] + moon_earth[2];

    vec3 me;
    me.x = moon_earth[0];
    me.y = moon_earth[1];
    me.z = moon_earth[2];

    vec3 ms;
    ms.x = moon_sun[0];
    ms.y = moon_sun[1];
    ms.z = moon_sun[2];

    double ems_angle = to_degrees(vec3::angle(me, ms));

    double lon_earth, lat_earth;
    vector2radec(moon_earth, &lon_earth, &lat_earth);
    auto [earth_ecl_lon, earth_ecl_lat] = novas_wrapper::w_equ2ecl(lookup_time, ecl_coord_sys, novas_constants::accuracy, lon_earth, lat_earth);
    if (earth_ecl_lon > 180.0)
    {
        earth_ecl_lon -= 360.0;
    }
    /* {earth_ecl_lon, earth_ecl_lat} is now the equatorial spherical coordinates, in degrees, of the Earth transit point on the Moon. */

    double lon_sun, lat_sun;
    vector2radec(moon_sun, &lon_sun, &lat_sun);
    auto [sun_ecl_lon, sun_ecl_lat] = novas_wrapper::w_equ2ecl(lookup_time, ecl_coord_sys, novas_constants::accuracy, lon_sun, lat_sun);
    if (sun_ecl_lon > 180.0)
    {
        sun_ecl_lon -= 360.0;
    }
    /* {sun_ecl_lon, sun_ecl_lat} is now the equatorial spherical coordinates, in degrees, of the Solar transit point on the Moon. */

    double phlon;
    double phlat;
    int phindex;

    phlon = normalize(sun_ecl_lon - earth_ecl_lon, 360.0);
    phindex = (int)floor(normalize(sun_ecl_lon - earth_ecl_lon + 22.5, 360.0) / 45.0);
    if (phlon > 180.0)
    {
        phlon -= 360.0;
    }
    phlat = sun_ecl_lat - earth_ecl_lat;

    if (phindex < 0 || phindex > 7)
    {
        throw std::runtime_error("moon phase index (" + std::to_string(phindex) + ") is out of bounds.");
    }
    auto phase = static_cast<moon_phase>(phindex);

    // https://the-moon.wikispaces.com/phase
    // phase_angle is the angle between the Sun and the observer's location on Earth as seen from the Moon's center
    // phase = (1 + Cosine(phase_angle))/2
    // To obtain the ("theoretical") percent illumination (PI), multiply P by 100.
    // PI = 50 x(1 + Cosine(PA))

    double phase_angle = ems_angle; // phase_lon is longitude delta on *moon* between sun vector and earth vector
    double pct_illum = (1 + std::cos(to_radians(phase_angle))) / 2.0;

    return {ems_angle, phlon, phlat, pct_illum, phase};
}

double novas_utils::get_local_apparent_sidereal_time(astro_time &lookup_time, double longitude)
{

    double greenwich_apparent_sidereal_time = 0, local_apparent_sidereal_time = 0;

    const short gst_type = 1; // Greenwich apparent sidereal time; use 0 for Greenwich mean sidereal time
    const short method = 1;   // equinox-based method

    // output sidereal time is Greenwich apparent sidereal time in hours
    short error = sidereal_time(lookup_time.as_ut1(), 0.0, lookup_time.delta_t(), gst_type, method, novas_constants::accuracy, &greenwich_apparent_sidereal_time);

    // RETURNED VALUE:
    //     = 0         ... everything OK
    //     = 1         ... invalid value of 'accuracy'
    //     = 2         ... invalid value of 'method'
    //     > 10, < 30  ... 10 + error from function 'cio_rai'

    if (error > 0)
    {
        if (error == 1)
        {
            throw std::runtime_error("invalid value of 'accuracy': " + std::to_string(novas_constants::accuracy));
        }
        else if (error == 2)
        {
            throw std::runtime_error("invalid value of 'method': " + std::to_string(method));
        }
        else
        {
            throw std::runtime_error("error from function 'cio_rai': " + (error - 10));
        }
    }

    // To compute local sidereal time (either mean or apparent), add the longitude (east positive) expressed in hours :

    local_apparent_sidereal_time = greenwich_apparent_sidereal_time + (longitude / 15.0);

    if (local_apparent_sidereal_time >= 24.0)
        local_apparent_sidereal_time -= 24.0;
    if (local_apparent_sidereal_time < 0.0)
        local_apparent_sidereal_time += 24.0;

    return local_apparent_sidereal_time;
}

planet_event_type novas_utils::determine_planetary_event_type(novas_wrapper::horizon_coords hc, astro_time event_time, on_surface geo_loc)
{

    const double bracket_range = 0.01; // Arbitrary but reasonable

    const double last = get_local_apparent_sidereal_time(event_time, geo_loc.longitude);
    const double lha = normalize(last - hc.rar, 24.0);

    // For any celestial object, the object’s right ascension plus the object’s current hour angle is equal to the local sidereal time at
    // the observing site. For an object at transit (i.e., H = 0), the local sidereal time equals the object’s right ascension.
    // Therefore:
    // RISE: lha>12.0
    // SET: lha<12.0
    // UPPER CULMINATION: LHA = 0 or 24
    // LOWER CULMINATION: LHA = 12

    if (abs(24.0 - lha) < bracket_range || abs(lha) < bracket_range)
    {
        return planet_event_type::upper_culmination;
    }
    else if (abs(12.0 - lha) < bracket_range)
    {
        return planet_event_type::lower_culmination;
    }
    else
    {
        if (lha > 12.0)
        {
            // object is to the east
            return planet_event_type::rise;
        }
        else
        {
            return planet_event_type::set;
        }
    }
}

std::vector<planetary_event> novas_utils::find_planetary_events(const double julian_utc_begin, const double julian_utc_end, novas_planet planet, on_surface geo_loc)
{

    observer surface_loc; // Need to get *topocentric* values for equ2hor to work correctly.
    make_observer_on_surface(geo_loc.latitude, geo_loc.longitude, geo_loc.height, geo_loc.temperature, geo_loc.pressure, &surface_loc);

    object planet_obj = build_planet_object(planet);

    auto hc_at_time_fn = [&](double jd_utc_time) -> auto
    {
        auto at = astro_time::from_utc(jd_utc_time);
        sky_pos t_place = novas_wrapper::w_place(at, planet_obj, surface_loc, novas_constants::coord_equ, novas_constants::accuracy);
        finals_data fd = at.get_finals_data();
        auto hc = novas_wrapper::w_equ2hor(at, t_place, novas_constants::accuracy, fd.pm_x, fd.pm_y, geo_loc, novas_constants::refraction);
        return std::tuple<novas_wrapper::horizon_coords, sky_pos>{hc, t_place};
    };

    auto el_at_time_fn = [&](double jd_utc_time) -> auto
    {

        auto [hc, t_place] = hc_at_time_fn(jd_utc_time);
        double distance_to_obj = au_to_km(t_place.dis);
        double actual_diameter_km = planet.diameter_km;
        double planet_diameter = to_degrees(2 * std::atan(actual_diameter_km / (2 * distance_to_obj)));

        // Moonrise occurs when EL is 90 degrees + Moon's apparent angular radius moon_app_radius, which varies between 0.245 and 0.279.
        // "More" EL means "lower". But we've already subtracted from 90.
        // hc.zd = 90 - hc.zd;

        return hc.zd + planet_diameter / 2.0;
    };

    auto az_at_time_fn = [&](double jd_utc_time) -> auto
    {
        auto [hc, t_place] = hc_at_time_fn(jd_utc_time);
        return hc.az - 180; // give us sign change at azimuth (180.0 or 0/360)
    };

    std::vector<double> event_times;

    std::vector<double> xb1, xb2;
    int nroot = 0;

    const int slices = (int)floor(8 * (julian_utc_end - julian_utc_begin));

    zbrak(el_at_time_fn, julian_utc_begin, julian_utc_end, slices, xb1, xb2, nroot);

    for (int i = 0; i < nroot; ++i)
    {

        auto jd_utc_of_event = zbrent(el_at_time_fn, xb1[i], xb2[i], finder_tolerance);

        event_times.push_back(jd_utc_of_event);
    }

    xb1.clear();
    xb2.clear();
    nroot = 0;

    zbrak(az_at_time_fn, julian_utc_begin, julian_utc_end, 31 * 4, xb1, xb2, nroot);

    for (int i = 0; i < nroot; ++i)
    {

        auto jd_utc_of_event = zbrent(az_at_time_fn, xb1[i], xb2[i], finder_tolerance);

        event_times.push_back(jd_utc_of_event);
    }

    std::sort(event_times.begin(), event_times.end());

    std::vector<planetary_event> events;

    for (auto utc_time : event_times)
    {
        auto [hc, t_place] = hc_at_time_fn(utc_time);

        planet_event_type et = determine_planetary_event_type(hc, astro_time::from_utc(utc_time), geo_loc);

        events.push_back({utc_time, hc, t_place, et});
    }

    return events;
}

std::vector<astro_time> novas_utils::find_new_and_full_moons(double jd_utc_beg, double jd_utc_end)
{

    std::vector<astro_time> rv;

    std::vector<double> xb1, xb2;
    int nroot = 0;

    auto pl_at_time_fn = [&](double jd_utc_time) -> auto
    {
        auto at = astro_time::from_utc(jd_utc_time);
        auto [phase_angle, phase_lon, phase_lat, pct_illum, phase] = get_moon_phase(at);
        return phase_lon;
    };

    zbrak(pl_at_time_fn, jd_utc_beg, jd_utc_end, 120, xb1, xb2, nroot);

    for (int i = 0; i < nroot; ++i)
    {
        auto jd_utc_of_event = zbrent(pl_at_time_fn, xb1[i], xb2[i], finder_tolerance);
        astro_time t_event = astro_time::from_utc(jd_utc_of_event);
        rv.push_back(t_event);
    }

    return rv;
}
