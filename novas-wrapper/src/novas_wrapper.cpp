#include "novas_wrapper.h"


namespace novas_wrapper {

	std::tuple<double, double, double> w_cel2ter(astro_time & lookup_time, short method, short accuracy, short option, double xp, double yp, double vec1[3]) {

		double vec2[3];	// vec2[3] (double) Position vector, geocentric equatorial rectangular coordinates, referred to ITRS axes(terrestrial system).

		short error = cel2ter(lookup_time.as_ut1 (), 0.0, lookup_time.delta_t(), method, accuracy, option, xp, yp, vec1, vec2);

		if (error == 0) {
			return { vec2[0], vec2[1], vec2[2] };
		}
		else if (error == 1) {
			throw std::runtime_error("invalid value of 'accuracy'");
		}
		else if (error == 2) {
			throw std::runtime_error("invalid value of 'method'");
		}
		else if (10 < error && error < 20) {
			throw std::runtime_error("error from function 'cio_location': " + (error - 10));
		}
		else if (20 < error) {
			throw std::runtime_error("error from function 'cio_basis': " + (error - 20));
		}
		else {
			throw std::runtime_error("unknown error: " + error);
		}

	}
	
	std::tuple<double, double> w_equ2ecl(astro_time & lookup_time, short coord_sys, short accuracy, double ra, double dec) {

		double elon, elat;

		short error = equ2ecl(lookup_time.as_tt(), coord_sys, accuracy, ra, dec, &elon, &elat);

		if (error == 0) {
			return { elon, elat };
		}
		else if (error == 1) {
			throw std::runtime_error("invalid value of 'coord_sys'");
		}
		else {
			throw std::runtime_error("unknown error: " + error);
		}

	}

	horizon_coords w_equ2hor(astro_time & lookup_time, sky_pos t_place, short accuracy, double x_pole, double y_pole, on_surface & geo_loc, short ref_option) {

		horizon_coords hc;

		equ2hor(lookup_time.as_ut1(), lookup_time.delta_t(), accuracy, x_pole, y_pole, &geo_loc, t_place.ra, t_place.dec, ref_option, 
			&hc.zd, &hc.az, &hc.rar, &hc.decr);

		hc.zd = 90 - hc.zd;

		return hc;
	}

	sky_pos w_place(astro_time & lookup_time, object & cel_object, observer & location, short coord_sys, short accuracy) {

		sky_pos t_place;

		short error = place(lookup_time.as_tt(), &cel_object, &location, lookup_time.delta_t(), coord_sys, accuracy, &t_place);

		if (error == 0) {
			return t_place;
		}
		else if (error == 1) {
			throw std::runtime_error("invalid value of 'coord_sys'");
		}
		else if (error == 2) {
			throw std::runtime_error("invalid value of 'accuracy'");
		}
		else if (error == 3) {
			throw std::runtime_error("Earth is the observed object, and the observer is either at the geocenter or on the Earth's surface (not permitted)");
		}
		else if (10 < error && error < 40) {
			// > 10, < 40  ... 10 + error from function 'ephemeris'
			throw std::runtime_error("error from function 'ephemeris': " + (error - 10));
		}
		else if (40 < error && error < 50) {
			// > 40, < 50  ... 40 + error from function 'geo_posvel'
			throw std::runtime_error("error from function 'geo_posvel': " + (error - 40));
		}
		else if (50 < error && error < 70) {
			// > 50, < 70  ... 50 + error from function 'light_time'
			throw std::runtime_error("error from function 'light_time': " + (error - 50));
		}
		else if (70 < error && error < 80) {
			// > 70, < 80  ... 70 + error from function 'grav_def'
			throw std::runtime_error("error from function 'grav_def': " + (error - 70));
		}
		else if (80 < error && error < 90) {
			// > 80, < 90  ... 80 + error from function 'cio_location'
			throw std::runtime_error("error from function 'cio_location': " + (error - 80));
		}
		else if (90 < error && error < 100) {
			// > 90, < 100 ... 90 + error from function 'cio_basis'
			throw std::runtime_error("error from function 'cio_basis': " + (error - 90));
		}
		else {
			throw std::runtime_error("unknown error: " + error);
		}
	}

	object w_make_object(short int type, novas_planet_id number, std::string const & name, cat_entry & star_data) {

		object cel_obj;

		short error = make_object(type, (short)number, const_cast<char *>(name.data()), &star_data, &cel_obj);

		switch (error) {
		case 0:
			return cel_obj;
		case 1:
			throw std::runtime_error("invalid value of 'type'");
		case 2:
			throw std::runtime_error("'number' out of range");
		case 3:
			throw std::runtime_error("Initialization of 'cel_obj' failed (object name).");
		case 4:
			throw std::runtime_error("Initialization of 'cel_obj' failed (catalog name).");
		case 5:
			throw std::runtime_error("'name' is out of string bounds.");
		default:
			throw std::runtime_error("unknown error: " + error);
		}
	}

}