
#include <stdexcept>
#include <tuple>

#include "ephemeris.h"

extern "C"
{
#include "eph_manager.h"
#include "novas.h"
}

ephemeris::ephemeris () :
	ephemeris_version (-1),
	ephemeris_begin (0),
	ephemeris_end (0)
{
}

ephemeris::~ephemeris()
{
    if (ephemeris_version > -1)
        ephem_close();
}

std::tuple<double, double, short> w_ephem_open (std::string ephemeris_path) {
	double ephemeris_begin;
	double ephemeris_end;
	short ephemeris_version;

	switch (ephem_open (ephemeris_path.data (), &ephemeris_begin, &ephemeris_end, &ephemeris_version)) {
	case 0:
		return { ephemeris_begin, ephemeris_end, ephemeris_version };
	case 1:
		throw std::runtime_error ("JPL ephemeris file not found at '" + ephemeris_path + "'");
	case 2:
		throw std::runtime_error ("error reading from file header at 'ttl'");
	case 3:
		throw std::runtime_error ("error reading from file header at 'cnam'");
	case 4:
		throw std::runtime_error ("error reading from file header at 'SS'");
	case 5:
		throw std::runtime_error ("error reading from file header at 'ncon'");
	case 6:
		throw std::runtime_error ("error reading from file header at 'JPLAU'");
	case 7:
		throw std::runtime_error ("error reading from file header at 'EM_RATIO'");
	case 8:
		throw std::runtime_error ("error reading from file header at 'IPT'");
	case 9:
		throw std::runtime_error ("error reading from file header at 'denum'");
	case 10:
		throw std::runtime_error ("error reading from file header at 'LPT'");
	case 11:
		throw std::runtime_error ("unable to set record length; ephemeris (DE number) not in look - up table.");
	default:
		throw std::runtime_error ("unknown error");
	}

}

void ephemeris::open (std::string ephemeris_path) {

	auto [eph_begin, eph_end, eph_version] = w_ephem_open (ephemeris_path);
	ephemeris_begin = eph_begin;
	ephemeris_end = eph_end;
	ephemeris_version = eph_version;
}

short ephemeris::eph_version () const
{
	return ephemeris_version;
}

double ephemeris::eph_begin () const
{
	return ephemeris_begin;
}

double ephemeris::eph_end () const
{
	return ephemeris_end;
}