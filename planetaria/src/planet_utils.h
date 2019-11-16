#pragma once

#include <json.hpp>
using n_json = nlohmann::json;

#include "astro_calc.h"
#include "astro_time.h"

#include "novas_wrapper.h"

namespace planet_utils {
    n_json get_current_planetary_positions ( astro_time lookup_time, std::vector<novas_planet> const & planets);
    n_json get_moon_phase_events (astro_time begin_time, astro_time end_time);
    n_json get_rise_and_set_times (astro_time begin_time, astro_time end_time, novas_planet planet, double observer_lat, double observer_lon);
};

