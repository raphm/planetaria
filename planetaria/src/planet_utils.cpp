#include "novas_utils.h"

#include "planet_utils.h"

n_json planet_utils::get_current_planetary_positions(astro_time lookup_time, std::vector<novas_planet> const &planets)
{

    n_json rv;

    for (auto planet : planets)
    {

        sky_pos sp = novas_utils::load_planet_geocentric_equatorial(lookup_time, planet);
        auto [elon, elat] = novas_wrapper::w_equ2ecl(lookup_time, novas_constants::coord_equ, novas_constants::accuracy, sp.ra, sp.dec);

        double ecliptic_long = normalize_degrees(elon);

        auto ra_hms = hms_decimal_to_hms(sp.ra);
        auto dec_dms = deg_to_dms(sp.dec);

        n_json ra;
        ra["hours"] = std::get<0>(ra_hms);
        ra["minutes"] = std::get<1>(ra_hms);
        ra["seconds"] = std::get<2>(ra_hms);

        n_json dec;
        dec["degrees"] = std::get<0>(dec_dms);
        dec["minutes"] = std::get<1>(dec_dms);
        dec["seconds"] = std::get<2>(dec_dms);

        rv.push_back({{"planet", planet.name},
                      {"right_ascension", ra},
                      {"declination", dec},
                      {"ecliptic_longitude", ecliptic_long},
                      {"ecliptic_latitude", elat}});
    }

    return rv;
}

n_json planet_utils::get_moon_phase_events(astro_time begin_time, astro_time end_time)
{

    n_json rv;

    // MOON EVENTS
    std::vector<astro_time> moon_times = novas_utils::find_new_and_full_moons(begin_time.as_utc(), end_time.as_utc());

    std::sort(moon_times.begin(), moon_times.end(), [](auto &a, auto &b) {
        return a.as_utc() < b.as_utc();
    });

    for (astro_time &t_event : moon_times)
    {

        auto [phase_angle, phase_lon, phase_lat, pct_illum, phase] = novas_utils::get_moon_phase(t_event);

        n_json evt_obj;
        evt_obj["time"] = t_event.as_iso8601_str();
        evt_obj["phase_lon"] = phase_lon;
        evt_obj["phase_lat"] = phase_lat;
        evt_obj["pct_illum"] = pct_illum;
        evt_obj["phase"] = moon_phase_str(phase);
        evt_obj["phase_angle"] = phase_angle;

        rv.push_back(evt_obj);
    }

    return rv;
}

n_json planet_utils::get_rise_and_set_times(astro_time begin_time, astro_time end_time, novas_planet planet, double observer_lat, double observer_lon)
{

    n_json rv;

    on_surface geo_loc;

    make_on_surface(observer_lat, observer_lon, 10, 14, 1200, &geo_loc);

    std::vector<planetary_event> v_pe = novas_utils::find_planetary_events(begin_time.as_utc(), end_time.as_utc(), planet, geo_loc);

    for (auto const &evt : v_pe)
    {

        astro_time t_event = astro_time::from_utc(evt.event_time);

        const double dist_km = au_to_km(evt.pos.dis);
        const double app_size = 2.0 * to_degrees(std::atan(planet.diameter_km / (2 * dist_km)));

        n_json obj;

        if (novas_constants::MOON.id == planet.id && evt.event == planet_event_type::upper_culmination)
        {
            auto at = astro_time::from_utc(evt.event_time);
            auto [phase_angle, phase_lon, phase_lat, pct_illum, phase] = novas_utils::get_moon_phase(at);
            obj["phase"] = moon_phase_str(phase);
            obj["pct_illum"] = pct_illum;
        }

        obj["time"] = t_event.as_iso8601_str();
        obj["event_type"] = planet_event_type_str(evt.event);
        obj["ra"] = evt.hc.rar;
        obj["el"] = evt.hc.zd;
        obj["az"] = evt.hc.az;
        obj["dist"] = dist_km;
        obj["app_sz"] = app_size;

        rv.push_back(obj);
    }

    return rv;
}
