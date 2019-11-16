#include <cassert>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <tuple>

#include "astro_calc.h"

/*
*  Converts a degree angle to hours, minutes and seconds.
*/

std::tuple<int, int, double> deg_to_hms(const double degrees) {

	// Each hour is 15 degrees
	// Each minute is 1/4 of a degree
	// Each second is 1/240 of a degree

	int sign = 1;
	double new_degrees = degrees;
	if (new_degrees < 0) {
		sign = -1;
		new_degrees *= sign;
	}

	const double total_seconds = new_degrees/360 * secs_per_day;

	double actual_hours = total_seconds / secs_per_hour;
	int hours = (int)floor(actual_hours);
	actual_hours -= hours;

	double actual_minutes = 60 * actual_hours;
	int minutes = (int)floor(actual_minutes);
	actual_minutes -= minutes;

	double seconds = 60 * actual_minutes;

	hours *= sign;

	return std::make_tuple(hours, minutes, seconds);
}

std::tuple<int, int, double> hms_decimal_to_hms(const double hms_decimal) {
	return deg_to_hms(15 * hms_decimal);
}

double hms_to_deg(int hours, int minutes, double seconds) {
	return (hours * 15.0) + (15 * (minutes / 60.0)) + (15 * (seconds / 3600.0));
}

double hms_to_deg(std::tuple<int, int, double> hms) {
	return hms_to_deg(std::get<0>(hms), std::get<1>(hms), std::get<2>(hms));
}

std::tuple<int, int, double> deg_to_dms(const double degrees) {

	int truncated_degrees = (int)((degrees > 0) ? floor(degrees) : ceil(degrees));

	double actual_arcminutes = fabs((degrees - truncated_degrees) * 60);
	int arcminutes = (int)floor(actual_arcminutes);
	actual_arcminutes -= arcminutes;

	double arcseconds = 60 * actual_arcminutes;

	return std::make_tuple(truncated_degrees, arcminutes, arcseconds);

}

double dms_to_deg(int degrees, int arcminutes, double arcseconds) {
	return ((degrees < 0 ? -1 : 1) * (fabs(degrees) + (arcminutes / 60.0) + (arcseconds / 3600.0)));
}

double dms_to_deg(std::tuple<int, int, double> dms) {
	return dms_to_deg(std::get<0>(dms), std::get<1>(dms), std::get<2>(dms));
}

std::string iid_to_str(std::tuple<int, int, double> iid) {
	int width = 2;

	int arg0 = std::get<0>(iid);
	if (arg0 < 0) {
		if (arg0 < -9) {
			width = 3;
		}
		else if (arg0 < -99) {
			width = 4;
		}
		else if (arg0 < -999) {
			width = 5;
		}
	}
	else {
		if (arg0 > 99) {
			width = 3;
		}
		else if (arg0 > 999) {
			width = 4;
		}
		else if (arg0 > 9999) {
			width = 5;
		}
	}

	std::ostringstream oStream;
	oStream << std::fixed;
	oStream << std::setfill('0');
	oStream << std::setw(width) << std::get<0>(iid) << ":"
		<< std::setw(2) << std::get<1>(iid) << ":"
		<< std::setw(9) << std::setprecision(6) << std::get<2>(iid);
	return oStream.str();
}

std::ostream &operator<< (std::ostream &os, std::tuple<int, int, double> iid) {
	os << iid_to_str(iid);
	return os;
}

stream_guard::stream_guard(std::ostream &stream) :
	os(stream),
	ss(os.precision()),
	ff(os.flags()),
	fill_c(os.fill())
{
}

stream_guard::~stream_guard() {
	os.precision(ss);
	os.flags(ff);
	os.fill(fill_c);
}

