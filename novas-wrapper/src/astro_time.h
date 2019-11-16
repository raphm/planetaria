#ifndef ASTRO_TIME_H
#define ASTRO_TIME_H

#include <iostream>
#include <mutex>
#include <vector>

#include "finals_data_handler.h"

class astro_time
{
  public:
	astro_time();
	~astro_time();

	static astro_time from_now();
	static astro_time from_tt(double jd_tt);
	static astro_time from_utc(double jd_utc);
	static astro_time from_ut1(double jd_ut1);
	static astro_time from_tdb(double jd_tdb);
	static astro_time from_iso8601(std::string const &time_str);

	double delta_t();

	double as_tt();
	double as_utc();
	double as_ut1();
	double as_tdb();

	std::string as_tt_str();
	std::string as_utc_str();
	std::string as_ut1_str();
	std::string as_tdb_str();
	std::string as_iso8601_str();

	astro_time prev_year_start();
	astro_time year_start();
	astro_time next_year_start();
	astro_time year_after_next_year_start();

	astro_time prev_month_start();
	astro_time month_start();
	astro_time next_month_start();
	astro_time month_after_next_month_start();
	
	finals_data get_finals_data();

	// Static

	static double julian_date_from_values(int year, int month, int day, int hour, int min, double secs);

	// Friend

	friend std::ostream &operator<<(std::ostream &os, astro_time st);

  private:
	static std::mutex mtx;
	double julian_tt;
	double julian_utc;
	double julian_ut1;
	double julian_tdb;
	finals_data cached_finals_data;
};

#endif