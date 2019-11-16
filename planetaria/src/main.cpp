#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <tuple>
#include <locale> 
#include <algorithm>
#include <set>

#include <json.hpp>
using n_json = nlohmann::json;

#include "novas_wrapper.h"
#include "finals_data_handler.h"
#include "ephemeris.h"
#include "astro_time.h"
#include "astro_calc.h"

#include "planet_utils.h"

// ARGUMENT PARSING LOGIC is from:
//
// https://stackoverflow.com/questions/865668/how-to-parse-command-line-arguments-in-c
// The first piece of code is mine the second was a edit by someone else. Regarding my
// code there is no licence it is public domain, feel free to use as you like without any warranty.

class input_parser
{
public:
	input_parser (int& argc, char** argv);
	/// @author iain
	const std::string& getCmdOption (const std::string& option) const;
	/// @author iain
	bool cmdOptionExists (const std::string& option) const;

private:
	std::vector<std::string> tokens;
};

inline bool iequals (const std::string& l, const std::string& r)
{
	std::locale const locale;
	return std::equal (l.cbegin (), l.cend (), r.cbegin (), r.cend (), [&](char a, char b) {
		return std::toupper (a, locale) == std::toupper (b, locale);
		});
}

input_parser::input_parser (int& argc, char** argv)
{
	for (int i = 1; i < argc; ++i)
		this->tokens.push_back (std::string (argv[i]));
}

const std::string& input_parser::getCmdOption (const std::string& option) const
{
	std::vector<std::string>::const_iterator itr;
	itr = std::find (this->tokens.begin (), this->tokens.end (), option);
	if (itr != this->tokens.end () && ++itr != this->tokens.end ())
	{
		return *itr;
	}
	static const std::string empty_string ("");
	return empty_string;
}

bool input_parser::cmdOptionExists (const std::string& option) const
{
	return std::find (this->tokens.begin (), this->tokens.end (), option) != this->tokens.end ();
}

// END ARGUMENT PARSING LOGIC

int main (int argc, char* argv[])
{
	std::string const app_name (argv[0]);
	std::string em_path ("./data/jpleph.430");
	std::string finals_path ("./data/finals.data.txt");

	try
	{

		input_parser input (argc, argv);

		if (input.cmdOptionExists ("-h") || !input.cmdOptionExists ("-c"))
		{
			std::cout << (app_name + " [-h : print this message]") << std::endl;
			std::cout << (app_name + " [-e ephemeris_location] : pass location of DE 430 Ephemeris. Defaults to '" + em_path + "'.") << std::endl;
			std::cout << (app_name + " [-f finals-data-location] : pass location of finals data. Defaults to '" + finals_path + "'.") << std::endl;
			std::cout << (app_name + " [-e ephemeris-location] [-f finals-data-location] -c command [parameters]") << std::endl;
			std::cout << std::endl;
			std::cout << "Commands: " << std::endl;
			std::cout << std::endl;
			std::cout << app_name << " -c planets" << std::endl;
			std::cout << app_name << " -c planets [-utc datetime]" << std::endl;
			std::cout << app_name << " -c planets [-utc datetime] [-all]" << std::endl;
			std::cout << app_name << " -c planets [-utc datetime] [-planet planet-name]" << std::endl;
			std::cout << std::endl;
			std::cout << app_name << " -c moon_phases" << std::endl;
			std::cout << app_name << " -c moon_phases [-utcstart datetime] [-utcend datetime]" << std::endl;
			std::cout << "                                " << "-utcstart defaults to the start of the current month" << std::endl;
			std::cout << "                                " << "-utcend defaults to thirty days after utcstart" << std::endl;
			std::cout << std::endl;
			std::cout << app_name << " -c rise_set -lat lat -lon lon" << std::endl;
			std::cout << app_name << " -c rise_set -lat lat -lon lon [-utcstart datetime] [-utcend datetime] [-planet planet-name]" << std::endl;
			std::cout << "                                " << "-utcstart defaults to now" << std::endl;
			std::cout << "                                " << "-utcend defaults to one day after utcstart" << std::endl;
			std::cout << "                                " << "-planet defaults to sun" << std::endl;
			std::cout << std::endl;
			std::cout << "Notes: " << std::endl;
			std::cout << std::endl;
			std::cout << "The 'datetime' values must be UTC, in ISO 8601 format: YYYY-MM, YYYY-MM-DD, or YYYY-MM-DDTHH:MM:SSZ" << std::endl;
			std::cout << "Latitude and longitude are in signed degrees (decimal) format: 41.25 or -122.95." << std::endl;
			std::cout << "Planet name(s) are case-insensitive, and may be one of the following values:" << std::endl;
			std::cout << "Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, Neptune, Pluto, Sun, Moon" << std::endl;
			return (0);
		}

		if (input.cmdOptionExists ("-e")) {
			em_path = input.getCmdOption ("-e");
		}

		if (input.cmdOptionExists ("-f")) {
			finals_path = input.getCmdOption ("-f");
		}

		std::string command = input.getCmdOption ("-c");

		n_json rv;

		auto& em = ephemeris::instance ();
		em.open (em_path);

		n_json eph_obj;

		eph_obj["version"] = em.eph_version ();
		eph_obj["start_julian"] = em.eph_begin ();
		eph_obj["end_julian"] = em.eph_end ();

		rv["ephemeris"] = eph_obj;

		finals_data_handler& fdh = finals_data_handler::instance ();
		fdh.load_finals_data_from_file (finals_path);

		if (iequals (command, "planets"))
		{
			std::set<novas_planet, bool (*)(novas_planet, novas_planet)> planets ([](novas_planet p0, novas_planet p1) { return p0.id < p1.id; });

			astro_time st0 = astro_time::from_now ();

			if (input.cmdOptionExists ("-utc")) {
				st0 = astro_time::from_iso8601 (input.getCmdOption ("-utc"));
			}

			if (input.cmdOptionExists ("-planet")) {
				std::string planet = input.getCmdOption ("-planet");

				for (auto const& p : novas_constants::all_planets)
				{
					if (iequals (p.name, planet))
					{
						planets.insert (p);
					}
				}

			}
			else {
				for (auto const& p : novas_constants::all_planets)
				{
					planets.insert (p);
				}
			}

			std::vector<novas_planet> pvec;

			for (auto const& p : planets)
			{
				pvec.push_back (p);
			}

			rv[command] = planet_utils::get_current_planetary_positions (st0, pvec);
			
			n_json args;

			args["utc"] = st0.as_iso8601_str ();

			n_json arg_planets;

			for (auto const& p : planets)
			{
				arg_planets.push_back (p.name);
			}

			args["planets"] = arg_planets;

			rv["args"] = args;

		}
		else if (iequals (command, "moon_phases")) {
			auto now = astro_time::from_now ();
			auto start = now.month_start ();
			auto end = now.next_month_start ();

			if (input.cmdOptionExists ("-utcstart")) {
				start = astro_time::from_iso8601 (input.getCmdOption ("-utcstart"));
				end = astro_time::from_utc (start.as_utc () + 30);
			}

			if (input.cmdOptionExists ("-utcend")) {
				end = astro_time::from_iso8601 (input.getCmdOption ("-utcend"));
			}

			if (end.as_utc () <= start.as_utc ()) {
				throw std::runtime_error ("start time of " + start.as_iso8601_str () + " must be less than end time of " + end.as_iso8601_str ());
			}

			rv[command] = planet_utils::get_moon_phase_events (start, end);

			n_json args;

			args["utcstart"] = start.as_iso8601_str ();
			args["utcend"] = end.as_iso8601_str ();

			rv["args"] = args;

		}
		else if (iequals (command, "rise_set")) {

			auto now = astro_time::from_now ();
			auto start = astro_time::from_utc (std::floor (now.as_utc ()));
			auto end = astro_time::from_utc (start.as_utc () + 1);

			if (input.cmdOptionExists ("-utcstart")) {
				start = astro_time::from_iso8601 (input.getCmdOption ("-utcstart"));
				end = astro_time::from_utc (start.as_utc () + 1);
			}

			if (input.cmdOptionExists ("-utcend")) {
				end = astro_time::from_iso8601 (input.getCmdOption ("-utcend"));
			}

			if (end.as_utc () <= start.as_utc ()) {
				throw std::runtime_error ("start time of " + start.as_iso8601_str () + " must be less than end time of " + end.as_iso8601_str ());
			}

			novas_planet n_planet = novas_constants::SUN;

			if (input.cmdOptionExists ("-planet")) {
				std::string planet = input.getCmdOption ("-planet");

				for (auto const& p : novas_constants::all_planets)
				{
					if (iequals (p.name, planet))
					{
						n_planet = p;
					}
				}
			}

			double lat = 0.0;
			double lon = 0.0;

			if (!input.cmdOptionExists ("-lat") || !input.cmdOptionExists ("-lon")) {
				throw std::runtime_error ("lat and lon are required for rise_set");
			}

			{
				std::string _lat = input.getCmdOption ("-lat");
				int parsed = sscanf (_lat.c_str (), "%lf", &lat);
				if (parsed != 1) {
					throw std::runtime_error ("invalid latitude value: " + _lat);
				}
			}

			{
				std::string _lon = input.getCmdOption ("-lon");
				int parsed = sscanf (_lon.c_str (), "%lf", &lon);
				if (parsed != 1) {
					throw std::runtime_error ("invalid longitude value: " + _lon);
				}
			}

			rv[command] = planet_utils::get_rise_and_set_times (start, end, n_planet, lat, lon);

			n_json args;

			args["utcstart"] = start.as_iso8601_str ();
			args["utcend"] = end.as_iso8601_str ();
			args["planet"] = n_planet.name;
			args["lat"] = lat;
			args["lon"] = lon;

			rv["args"] = args;

		}
		else {
			throw std::runtime_error ("Unknown command: " + command);
		}

		std::cout << rv.dump () << std::endl;

	}
	catch (std::exception & e)
	{
		n_json rv;

		rv["error"] = e.what ();

		std::cout << rv.dump () << std::endl;

	}
}