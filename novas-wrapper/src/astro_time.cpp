#include <iostream>
#include <sstream>
#include <iomanip>
#include <tuple>
#include <chrono>
#include <thread>
#include <mutex>
#include <algorithm>
#include <vector>
#include <ctime>
#include <cmath>
#include <cerrno>

#include "astro_time.h"

extern "C"
{
#include "novas.h"
}

constexpr double secs_per_day = 86400; // This varies in the event of leap seconds.
constexpr double secs_per_hour = 3600;

std::mutex astro_time::mtx;

// Some of this code was taken from: https://github.com/brhamon/astro/blob/master/ephutil/ephutil.c

// From IERS: https://hpiers.obspm.fr/iers/bul/bulc/bulletinc.dat

typedef struct leap_second_t_
{
	int year;
	int month;
	int day;
	double jd;
	double tai_utc;
} leap_second_t;

static const std::vector<leap_second_t> leap_seconds = {
	{1972, 1, 1, 2441317.5, 10.0},
	{1972, 7, 1, 2441499.5, 11.0},
	{1973, 1, 1, 2441683.5, 12.0},
	{1974, 1, 1, 2442048.5, 13.0},
	{1975, 1, 1, 2442413.5, 14.0},
	{1976, 1, 1, 2442778.5, 15.0},
	{1977, 1, 1, 2443144.5, 16.0},
	{1978, 1, 1, 2443509.5, 17.0},
	{1979, 1, 1, 2443874.5, 18.0},
	{1980, 1, 1, 2444239.5, 19.0},
	{1981, 7, 1, 2444786.5, 20.0},
	{1982, 7, 1, 2445151.5, 21.0},
	{1983, 7, 1, 2445516.5, 22.0},
	{1985, 7, 1, 2446247.5, 23.0},
	{1988, 1, 1, 2447161.5, 24.0},
	{1990, 1, 1, 2447892.5, 25.0},
	{1991, 1, 1, 2448257.5, 26.0},
	{1992, 7, 1, 2448804.5, 27.0},
	{1993, 7, 1, 2449169.5, 28.0},
	{1994, 7, 1, 2449534.5, 29.0},
	{1996, 1, 1, 2450083.5, 30.0},
	{1997, 7, 1, 2450630.5, 31.0},
	{1999, 1, 1, 2451179.5, 32.0},
	{2006, 1, 1, 2453736.5, 33.0},
	{2009, 1, 1, 2454832.5, 34.0},
	{2012, 7, 1, 2456109.5, 35.0},
	{2015, 7, 1, 2457204.5, 36.0},
	{2017, 1, 1, 2457754.5, 37.0} };

/*
* Find the number of leap seconds for a given JD UTC.
*/
double leapsec_tai_utc (double jd_utc)
{

	leap_second_t test;
	test.jd = jd_utc;

	std::vector<leap_second_t>::const_iterator it = std::lower_bound (leap_seconds.begin (), leap_seconds.end (), test, [](const leap_second_t& l0, const leap_second_t& l1) -> bool { return l0.jd < l1.jd; });

	if (it == leap_seconds.begin ())
	{
		// A time before 1972? Leap seconds were not invented yet.
		return 0;
	}
	else if (it == leap_seconds.end ())
	{
		// A time after 2017? Apply the latest amount we have.
		return leap_seconds.back ().tai_utc;
	}
	else
	{
		return (*(it - 1)).tai_utc;
	}

	return 0;
}

finals_data astro_time::get_finals_data ()
{
	if (std::isnan (cached_finals_data.julian_utc))
	{
		cached_finals_data = finals_data_handler::instance ().finals_data_for_time (julian_utc);
	}
	return cached_finals_data;
}

double astro_time::julian_date_from_values (int year, int month, int day, int hour, int min, double secs)
{
	double hour_part = (double)hour * 3600 + (double)min * 60 + secs; // as seconds
	hour_part /= secs_per_hour;                       // as hours
	return julian_date (year, month, day, hour_part);
}

double jd_utc_now (std::mutex& scmtx, bool as_utc)
{

	using namespace std;
	using namespace std::chrono;

	typedef duration<int, ratio_multiply<hours::period, ratio<24>>::type> days;

	system_clock::time_point now_tp = system_clock::now ();
	system_clock::duration ntp = now_tp.time_since_epoch ();
	{
		days d = duration_cast<days>(ntp);
		ntp -= d;
		hours h = duration_cast<hours>(ntp);
		ntp -= h;
		minutes m = duration_cast<minutes>(ntp);
		ntp -= m;
		seconds s = duration_cast<seconds>(ntp);
		ntp -= s;
	}

	system_clock::time_point day_tp = std::chrono::floor<days> (now_tp);
	system_clock::duration dtp = day_tp.time_since_epoch ();
	{
		days d = duration_cast<days>(dtp);
		dtp -= d;
		hours h = duration_cast<hours>(dtp);
		dtp -= h;
		minutes m = duration_cast<minutes>(dtp);
		dtp -= m;
		seconds s = duration_cast<seconds>(dtp);
		dtp -= s;
	}

	double fractional_seconds = (ntp.count () - dtp.count ()) * 1.0 * system_clock::duration::period::num / system_clock::duration::period::den;

	time_t tt_now = std::chrono::system_clock::to_time_t (now_tp);
	time_t tt_day = std::chrono::system_clock::to_time_t (day_tp);

	tm now_tm;
	tm day_tm;

	{
		std::lock_guard<std::mutex> lck (scmtx);

		tm* now_tm_ptr = nullptr;
		tm* day_tm_ptr = nullptr;

		if (as_utc)
		{
			now_tm_ptr = gmtime (&tt_now);
			if (now_tm_ptr != nullptr)
				now_tm = *now_tm_ptr;

			day_tm_ptr = gmtime (&tt_day);
			if (day_tm_ptr != nullptr)
				day_tm = *day_tm_ptr;
		}
		else
		{
			now_tm_ptr = localtime (&tt_now);
			if (now_tm_ptr != nullptr)
				now_tm = *now_tm_ptr;

			day_tm_ptr = localtime (&tt_day);
			if (day_tm_ptr != nullptr)
				day_tm = *day_tm_ptr;
		}

		if (now_tm_ptr == nullptr || day_tm_ptr == nullptr)
		{
			throw std::runtime_error ("Error: gmtime/localtime returned nullptr.");
		}
	}

	return astro_time::julian_date_from_values (now_tm.tm_year + 1900,
		now_tm.tm_mon + 1,
		now_tm.tm_mday,
		now_tm.tm_hour,
		now_tm.tm_min,
		now_tm.tm_sec + fractional_seconds);
}

double julian_tdb_to_tt (double julian_tdb)
{
	double julian_tt = 0;
	double tdb_minus_tt_secs = 0;

	tdb2tt (julian_tdb, &julian_tt, &tdb_minus_tt_secs);

	return julian_tt;
}

double julian_tt_to_tdb (double julian_tt)
{
	double jd_tdb = julian_tt;
	double dummy, secdiff;
	tdb2tt (jd_tdb, &dummy, &secdiff);
	jd_tdb = julian_tt + secdiff / secs_per_day;
	return jd_tdb;
}

double julian_utc_to_tt (double julian_utc)
{
	// TAI = UTC + leap_seconds_to_date
	// TT = TAI + 32.184s
	// TT = UTC + leap_seconds_to_date + 32.184
	return julian_utc + (leapsec_tai_utc (julian_utc) + 32.184) / secs_per_day;
}

double julian_tt_to_utc (double julian_tt)
{
	// TAI = UTC + leap_seconds_to_date
	// TT = TAI + 32.184s
	// TT = UTC + leap_seconds_to_date + 32.184
	double julian_tai = julian_tt - (32.184 / secs_per_day);
	// Julian TAI will be UTC + leap_seconds, but... leap_seconds is determined based on UTC, not TAI
	// Either...
	// a) TAI and UTC return the same value of leap seconds:
	//      Determine leap_seconds for TAI or UTC (same value), then subtract from TAI to give UTC
	// b) TAI returns one more leap_second than (TAI-leap_seconds):
	//      Determine leap_seconds for TAI, subtract from TT, determine leap_seconds for that value, subtract that value from TAI to give UTC

	double leap_for_tai = leapsec_tai_utc (julian_tai);
	double leap_for_utc = leapsec_tai_utc (julian_tai - (leap_for_tai / secs_per_day));

	if (leap_for_tai == leap_for_utc)
	{
		return julian_tt - (leap_for_tai + 32.184) / secs_per_day;
	}
	else
	{
		return julian_tt - (leap_for_utc + 32.184) / secs_per_day;
	}
}

astro_time astro_time::from_now ()
{
	return from_utc (jd_utc_now (mtx, true));
}

astro_time astro_time::from_tt (double jd_tt)
{
	astro_time st;
	st.julian_tt = jd_tt;
	st.julian_utc = julian_tt_to_utc (jd_tt);
	st.julian_tdb = julian_tt_to_tdb (jd_tt);
	st.julian_ut1 = finals::DNAN;
	return st;
}

astro_time astro_time::from_utc (double jd_utc)
{
	astro_time st;
	st.julian_tt = julian_utc_to_tt (jd_utc);
	st.julian_utc = jd_utc;
	st.julian_tdb = julian_tt_to_tdb (st.julian_tt);
	st.julian_ut1 = finals::DNAN;
	return st;
}

astro_time astro_time::from_ut1 (double jd_ut1)
{
	astro_time st;

	st.julian_utc = jd_ut1; // set this temporarily so we can get correct finals data
	st.get_finals_data ();
	st.julian_utc = jd_ut1 - st.cached_finals_data.ut1_utc;
	st.julian_ut1 = jd_ut1;
	st.julian_tt = julian_utc_to_tt (st.julian_utc);
	st.julian_tdb = julian_tt_to_tdb (st.julian_tt);
	return st;
}

astro_time astro_time::from_tdb (double jd_tdb)
{
	astro_time st;
	st.julian_tt = julian_tdb_to_tt (jd_tdb);
	st.julian_utc = julian_tt_to_utc (st.julian_tt);
	st.julian_tdb = jd_tdb;
	st.julian_ut1 = finals::DNAN;
	return st;
}

astro_time astro_time::from_iso8601 (std::string const& time_str)
{

	int year = 0;
	int month = 0;
	int day = 1;
	int hours = 0;
	int minutes = 0;
	double seconds = 0.0;

	int rv;

	if (time_str.length () == 7)
	{
		rv = sscanf (time_str.c_str (), "%d-%d", &year, &month); //"2018-02"
		if (rv != 2) {
			throw std::runtime_error ("invalid time string: '" + time_str + "'; rv = " + std::to_string (rv));
		}
	}
	else if (time_str.length () == 10)
	{
		rv = sscanf (time_str.c_str (), "%d-%d-%d", &year, &month, &day); //"2018-02-23"
		if (rv != 3) {
			throw std::runtime_error ("invalid time string: '" + time_str + "'; rv = " + std::to_string (rv));
		}
	}
	else if (time_str.length () >= 20)
	{
		rv = sscanf (time_str.c_str (), "%d-%d-%dT%d:%d:%lfZ", &year, &month, &day, &hours, &minutes, &seconds); //"2018-02-23T03:05:45.01234Z"
		if (rv != 6) {
			throw std::runtime_error ("invalid time string: '" + time_str + "'; rv = " + std::to_string (rv));
		}
	}
	else {
		throw std::runtime_error ("invalid time string: '" + time_str + "'");
	}

	return from_utc (julian_date_from_values (year, month, day, hours, minutes, seconds));

}

astro_time::astro_time () : julian_tt (0), julian_utc (0), julian_ut1 (finals::DNAN), julian_tdb (0) {}

astro_time::~astro_time () {}

double astro_time::delta_t ()
{
	if (std::isnan (cached_finals_data.julian_utc))
	{
		get_finals_data ();
		julian_ut1 = julian_utc + (cached_finals_data.ut1_utc / secs_per_day);
	}
	return 32.184 + leapsec_tai_utc (julian_utc) - cached_finals_data.ut1_utc;
}

double astro_time::as_tt ()
{
	return julian_tt;
}

double astro_time::as_utc ()
{
	return julian_utc;
}

double astro_time::as_ut1 ()
{
	if (std::isnan (cached_finals_data.julian_utc))
	{
		get_finals_data ();
		julian_ut1 = julian_utc + (cached_finals_data.ut1_utc / secs_per_day);
	}
	return julian_ut1;
}

double astro_time::as_tdb ()
{
	return julian_tdb;
}

std::string strip_zero (double val, int width)
{

	std::stringstream ss;
	ss << std::setw (width) << std::setprecision (width);
	ss << std::fixed << val;
	std::string str = ss.str ();

	if (val > 0.0 && val < 1.0)
	{
		return str.substr (1, str.size () - 1);
	}
	else if (val < 0.0 && val > -1.0)
	{
		return "-" + str.substr (2, str.size () - 1);
	}
	return str;
}

std::string as_string (double julian)
{

	std::ostringstream oStream;

	short year, month, day;
	double hour_part;

	cal_date (julian, &year, &month, &day, &hour_part);

	short hours, mins, secs;
	double fractional_secs;

	hours = (short)floor (hour_part);
	hour_part -= hours;

	hour_part *= 3600;

	mins = (short)floor (hour_part / 60);
	hour_part -= ((double)mins * 60);

	secs = (short)floor (hour_part);
	hour_part -= secs;

	fractional_secs = hour_part;

	oStream << std::fixed
		<< std::setprecision (8)
		<< std::setfill ('0')
		<< std::setw (4) << year
		<< "-"
		<< std::setw (2) << month
		<< "-"
		<< std::setw (2) << day
		<< " "
		<< std::setw (2) << hours
		<< ":"
		<< std::setw (2) << mins
		<< ":"
		<< std::setw (2) << secs
		<< strip_zero (fractional_secs, 3);

	return oStream.str ();
}

std::string astro_time::as_tt_str () { return as_string (as_tt ()) + " TT"; }
std::string astro_time::as_utc_str () { return as_string (as_utc ()) + " UTC"; }
std::string astro_time::as_ut1_str () { return as_string (as_ut1 ()) + " UT1"; }
std::string astro_time::as_tdb_str () { return as_string (as_tdb ()) + " TDB"; }

std::string astro_time::as_iso8601_str ()
{

	std::ostringstream oStream;

	short year, month, day;
	double hour_part;

	cal_date (julian_utc, &year, &month, &day, &hour_part);

	short hours, mins, secs;
	double fractional_secs;

	hours = (short)floor (hour_part);
	hour_part -= hours;

	hour_part *= 3600;

	mins = (short)floor (hour_part / 60);
	hour_part -= ((double)mins * 60);

	secs = (short)floor (hour_part);
	hour_part -= secs;

	fractional_secs = hour_part;

	//"2018-02-23T03:05:45.01234Z"

	oStream << std::fixed
		<< std::setprecision (8)
		<< std::setfill ('0')
		<< std::setw (4) << year
		<< "-"
		<< std::setw (2) << month
		<< "-"
		<< std::setw (2) << day
		<< "T"
		<< std::setw (2) << hours
		<< ":"
		<< std::setw (2) << mins
		<< ":"
		<< std::setw (2) << secs
		<< strip_zero (fractional_secs, 3)
		<< "Z";

	return oStream.str ();
}

std::ostream& operator<<(std::ostream& os, astro_time st)
{

	os << st.as_utc_str ();

	return os;
}

astro_time astro_time::prev_year_start ()
{
	short year, month, day;
	double hour_part;

	cal_date (as_utc (), &year, &month, &day, &hour_part);

	return from_utc (julian_date (year - 1, 1, 1, 0.0));
}

astro_time astro_time::year_start ()
{
	short year, month, day;
	double hour_part;

	cal_date (as_utc (), &year, &month, &day, &hour_part);

	return from_utc (julian_date (year, 1, 1, 0.0));
}

astro_time astro_time::next_year_start ()
{
	short year, month, day;
	double hour_part;

	cal_date (as_utc (), &year, &month, &day, &hour_part);

	return from_utc (julian_date (year + 1, 1, 1, 0.0));
}

astro_time astro_time::year_after_next_year_start ()
{
	short year, month, day;
	double hour_part;

	cal_date (as_utc (), &year, &month, &day, &hour_part);

	return from_utc (julian_date (year + 2, 1, 1, 0.0));
}

astro_time astro_time::prev_month_start ()
{
	short year, month, day;
	double hour_part;

	cal_date (as_utc (), &year, &month, &day, &hour_part);

	month -= 1;

	if (month <= 0)
	{
		month = 12;
		year -= 1;
	}

	return from_utc (julian_date (year, month, 1, 0));
}

astro_time astro_time::month_start ()
{

	short year, month, day;
	double hour_part;

	cal_date (as_utc (), &year, &month, &day, &hour_part);

	return from_utc (julian_date (year, month, 1, 0));
}

astro_time astro_time::next_month_start ()
{

	short year, month, day;
	double hour_part;

	cal_date (as_utc (), &year, &month, &day, &hour_part);

	month += 1;

	if (month > 12)
	{
		month = 1;
		year += 1;
	}

	return from_utc (julian_date (year, month, 1, 0));
}

astro_time astro_time::month_after_next_month_start ()
{

	short year, month, day;
	double hour_part;

	cal_date (as_utc (), &year, &month, &day, &hour_part);

	month += 1;

	if (month > 12)
	{
		month = 1;
		year += 1;
	}

	month += 1;

	if (month > 12)
	{
		month = 1;
		year += 1;
	}

	return from_utc (julian_date (year, month, 1, 0));
}
