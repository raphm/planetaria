#include <string>
#include <algorithm>
#include <iostream>

#include "slurp_file.h"

#include "finals_data_handler.h"

using finals::DNAN;

finals_data_handler::finals_data_handler () {}

finals_data_handler::~finals_data_handler () {}

/*
The format of the finals.data, finals.daily, and finals.all files is:

Col.#    Format  Quantity
-------  ------  -------------------------------------------------------------
1-2      I2      year (to get true calendar year, add 1900 for MJD<=51543 or add 2000 for MJD>=51544)
3-4      I2      month number
5-6      I2      day of month
7        X       [blank]
8-15     F8.2    fractional Modified Julian Date (MJD UTC)
16       X       [blank]
17       A1      IERS (I) or Prediction (P) flag for Bull. A polar motion values
18       X       [blank]
19-27    F9.6    Bull. A PM-x (sec. of arc)
28-36    F9.6    error in PM-x (sec. of arc)
37       X       [blank]
38-46    F9.6    Bull. A PM-y (sec. of arc)
47-55    F9.6    error in PM-y (sec. of arc)
56-57    2X      [blanks]
58       A1      IERS (I) or Prediction (P) flag for Bull. A UT1-UTC values
59-68    F10.7   Bull. A UT1-UTC (sec. of time)
69-78    F10.7   error in UT1-UTC (sec. of time)
79       X       [blank]
80-86    F7.4    Bull. A LOD (msec. of time) -- NOT ALWAYS FILLED
87-93    F7.4    error in LOD (msec. of time) -- NOT ALWAYS FILLED
94-95    2X      [blanks]
96       A1      IERS (I) or Prediction (P) flag for Bull. A nutation values
97       X       [blank]
98-106   F9.3    Bull. A dPSI (msec. of arc)
107-115  F9.3    error in dPSI (msec. of arc)
116      X       [blank]
117-125  F9.3    Bull. A dEPSILON (msec. of arc)
126-134  F9.3    error in dEPSILON (msec. of arc)
135-144  F10.6   Bull. B PM-x (sec. of arc)
145-154  F10.6   Bull. B PM-y (sec. of arc)
155-165  F11.7   Bull. B UT1-UTC (sec. of time)
166-175  F10.3   Bull. B dPSI (msec. of arc)
176-185  F10.3   Bull. B dEPSILON (msec. of arc)

*/


double fd_extract_double (std::string& number_holder, std::string const& filestr, size_t offset, int length) {
	number_holder.replace (0, length, filestr, offset, length);
	number_holder.resize (length);
	return std::stod (number_holder);
}

bool fd_extract_prediction_flag (std::string const& filestr, size_t offset) {
	char prediction_flag = filestr[offset];
	if (prediction_flag == 'I') {
		return false;
	}
	else if (prediction_flag == 'P') {
		return true;
	}
	throw std::runtime_error (std::string ("error while parsing prediction flag: got '") + prediction_flag + "'");
}

void finals_data_handler::parse_finals_data (std::string const& finals_data_str) {

	std::string number_holder;

	long linenum = 0;

	finals_data_values.clear ();

	try {

		number_holder.reserve (64);

		size_t offset = 0;
		size_t oldoffset = 0;

		while (offset != std::string::npos) {
			offset = finals_data_str.find ('\n', oldoffset);
			if (offset == std::string::npos) break;

			++linenum;

			finals_data fd;

			// 8-15     F8.2    fractional Modified Julian Date (MJD UTC)
			// MJD = JD - 2400000.5; see http://tycho.usno.navy.mil/mjd.html
			// The half day is subtracted so that the day starts at midnight in conformance with civil time reckoning.
			fd.julian_utc = 2400000.50 + fd_extract_double (number_holder, finals_data_str, oldoffset + 7, 8);

			// 17       A1      IERS (I) or Prediction (P) flag for Bull. A polar motion values
			fd.pm_is_prediction = fd_extract_prediction_flag (finals_data_str, oldoffset + 16);
			// 19-27    F9.6    Bull. A PM-x (sec. of arc)
			fd.pm_x = fd_extract_double (number_holder, finals_data_str, oldoffset + 18, 9);
			// 28-36    F9.6    error in PM-x (sec. of arc)
			fd.pm_x_err = fd_extract_double (number_holder, finals_data_str, oldoffset + 27, 9);
			// 38-46    F9.6    Bull. A PM-y (sec. of arc)
			fd.pm_y = fd_extract_double (number_holder, finals_data_str, oldoffset + 37, 9);
			// 47-55    F9.6    error in PM-y (sec. of arc)
			fd.pm_y_err = fd_extract_double (number_holder, finals_data_str, oldoffset + 46, 9);

			// 58       A1      IERS (I) or Prediction (P) flag for Bull. A UT1-UTC values
			fd.ut1_utc_is_prediction = fd_extract_prediction_flag (finals_data_str, oldoffset + 57);
			// 59-68    F10.7   Bull. A UT1-UTC (sec. of time)
			fd.ut1_utc = fd_extract_double (number_holder, finals_data_str, oldoffset + 58, 10);
			// 69-78    F10.7   error in UT1-UTC (sec. of time)
			fd.ut1_utc_err = fd_extract_double (number_holder, finals_data_str, oldoffset + 68, 10);

			try {
				// 117 - 125  F9.3    Bull.A dEPSILON(msec.of arc)
				fd.epsilon = fd_extract_double (number_holder, finals_data_str, oldoffset + 116, 9);
				// 126 - 134  F9.3    error in dEPSILON(msec.of arc)
				fd.epsilon_err = fd_extract_double (number_holder, finals_data_str, oldoffset + 125, 9);

			}
			catch (std::exception& e) {
				(void)e;
				// We don't care about not getting an epsilon value.
			}

			oldoffset = offset + 1;

			finals_data_values.push_back (fd);

		}
	}
	catch (std::exception& e) {
		// No UT1-UTC, time to stop parsing.
		// If we're in the first few lines of the file, something is very wrong...
		if (linenum < 10) {
			throw std::runtime_error (std::string ("Error: ") + e.what () + " (line " + std::to_string (linenum) + ")");
		}
	}
}

void finals_data_handler::load_finals_data_from_file (std::string const& filepath) {
	parse_finals_data (slurpfile (filepath));
}

inline double mix (const double interp_factor, const double start, const double end) {
	return start + (interp_factor * (end - start));
}

finals_data finals_data_handler::finals_data_for_time (double jd_utc) {

	finals_data rv;

	rv.julian_utc = jd_utc;

	double key0;
	//double key1;

	double base_val = std::floor (jd_utc) + 0.5; // Julian is noon to noon

	if (jd_utc < base_val) {
		key0 = base_val - 1.0;
		//key1 = base_val;
	}
	else {
		key0 = base_val;
		//key1 = base_val + 1.0;
	}
	
	std::vector<finals_data>::iterator it = std::find_if (finals_data_values.begin (), finals_data_values.end (), [key0](finals_data fd)->bool {
		return std::abs (fd.julian_utc - key0) < 0.001;
		}
	);

	if (it == finals_data_values.end ()) {
		return rv;
	}

	if((it+1) == finals_data_values.end ()) {
		return rv;
	}

	finals_data rv_start = *it;
	finals_data rv_end = *(it + 1);
	
	const double interp_factor = (jd_utc - rv_start.julian_utc) / (rv_end.julian_utc - rv_start.julian_utc);

	//rv.julian_utc = jd_utc;

	rv.pm_is_prediction = rv_start.pm_is_prediction;
	rv.pm_x = mix (interp_factor, rv_start.pm_x, rv_end.pm_x);
	rv.pm_x_err = mix (interp_factor, rv_start.pm_x_err, rv_end.pm_x_err);
	rv.pm_y = mix (interp_factor, rv_start.pm_y, rv_end.pm_y);
	rv.pm_y_err = mix (interp_factor, rv_start.pm_y_err, rv_end.pm_y_err);

	rv.ut1_utc_is_prediction = rv_start.ut1_utc_is_prediction;
	rv.ut1_utc = mix (interp_factor, rv_start.ut1_utc, rv_end.ut1_utc);
	rv.ut1_utc_err = mix (interp_factor, rv_start.ut1_utc_err, rv_end.ut1_utc_err);

	rv.epsilon = mix (interp_factor, rv_start.epsilon, rv_end.epsilon);
	rv.epsilon_err = mix (interp_factor, rv_start.epsilon_err, rv_end.epsilon_err);

	return rv;
}
