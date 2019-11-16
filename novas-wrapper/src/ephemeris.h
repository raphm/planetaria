#pragma once

#include <string>

class ephemeris
{

public:
    static ephemeris &instance()
    {
        static ephemeris m_inst;
        return m_inst;
    }


	// open: opens a JPL planetary ephemeris file and sets initial values. Must be called prior to calls to the other JPL ephemeris functions.
	// 
	// INPUT:
	//   ephemeris_path:                     path to binary JPL ephemeris file
	// 
	// OUTPUT:
	//   std::tuple<double, double, short>:  beginning Julian date of ephemeris file, ending Julian date of ephemeris file, DE number of opened ephemeris file

    void open(std::string ephemeris_path);

    ~ephemeris();

    short eph_version() const;
    double eph_begin() const;
    double eph_end() const;

    ephemeris(ephemeris const &) = delete;
    ephemeris(ephemeris &&) = delete;
    ephemeris &operator=(ephemeris const &) = delete;
    ephemeris &operator=(ephemeris &&) = delete;

private:
    ephemeris();

    short ephemeris_version;
    double ephemeris_begin;
    double ephemeris_end;
};