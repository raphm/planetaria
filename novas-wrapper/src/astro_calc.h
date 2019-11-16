#ifndef ASTRO_CALC_H
#define ASTRO_CALC_H

#include <iostream>
#include <cmath>

// CONSTANTS

inline double const_pi() { return (std::atan(1) * 4); }
const double PI_VAL = const_pi();

const double degs_per_rad = 180.0/PI_VAL;
const double rads_per_deg = PI_VAL/180.0;

constexpr double secs_per_day = 86400;
constexpr double secs_per_hour = 3600;

// END CONSTANTS


// INLINE FUNCTIONS

inline double normalize(const double val, const double period)
{
	const double quot = val / period;
	return period * (quot - floor(quot));
}

// Return angle in degrees in range 0 <= d < 360
inline double normalize_degrees(const double angle) {
	return normalize(angle, 360);
}

inline double to_degrees(const double rads) {
	return rads * degs_per_rad;
}

inline double to_radians(const double degs) {
	return degs * rads_per_deg;
}

// 1 AU = 149597870700 meters, exactly
// 1 AU = 149597870.700 kilometers, exactly
// 1 KM = 1/149597870.700 AU
const double km_per_au = 149597870.700;
const double au_per_km = (1 / km_per_au);

inline double km_to_au(const double km) {
	return km * au_per_km;
}

inline double au_to_km(const double au) {
	return au * km_per_au;
}

// END INLINE FUNCTIONS

std::tuple<int, int, double> deg_to_hms(const double degrees);
std::tuple<int, int, double> hms_decimal_to_hms(const double hms_decimal);
double hms_to_deg(int hours, int minutes, double seconds);
double hms_to_deg(std::tuple<int, int, double> hms);

std::tuple<int, int, double> deg_to_dms(const double degrees);
double dms_to_deg(int degrees, int arcminutes, double arcseconds);
double dms_to_deg(std::tuple<int, int, double> dms);

std::string iid_to_str(std::tuple<int, int, double> iid);

std::ostream &operator<< (std::ostream &os, std::tuple<int, int, double> iid);

class stream_guard {
public:
	stream_guard(std::ostream &stream);
	~stream_guard();

private:
	std::ostream &os;
	std::streamsize ss;
	std::ios_base::fmtflags ff;
	char fill_c;
};

#endif // ASTRO_CALC_H



