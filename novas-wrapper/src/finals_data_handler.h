#pragma once
#include <vector>
#include <cmath>

namespace finals {
    static const double DNAN = std::nan ("");
};

struct finals_data {


    double julian_utc;

    bool pm_is_prediction;
    double pm_x;        /* Polar Motion 'x' value in seconds of arc. */
    double pm_x_err;    /* Polar Motion 'x' error in seconds of arc. */
    double pm_y;        /* Polar Motion 'y' value in seconds of arc. */
    double pm_y_err;    /* Polar Motion 'y' error in seconds of arc. */

    bool ut1_utc_is_prediction;
    double ut1_utc;     /* UT1-UTC value in seconds of time. */
    double ut1_utc_err; /* UT1-UTC error in seconds of time. */

    double epsilon;
    double epsilon_err;

    finals_data () :
        julian_utc (finals::DNAN),
        pm_is_prediction (false), pm_x (0), pm_x_err (0), pm_y (0), pm_y_err (0),
        ut1_utc_is_prediction (false), ut1_utc (0), ut1_utc_err (0),
        epsilon (0), epsilon_err (0) {}

};

class finals_data_handler {
public:
    static finals_data_handler& instance () {
        static finals_data_handler m_inst;
        return m_inst;
    }

    finals_data_handler (finals_data_handler const&) = delete;
    finals_data_handler (finals_data_handler&&) = delete;
    finals_data_handler& operator=(finals_data_handler const&) = delete;
    finals_data_handler& operator=(finals_data_handler &&) = delete;

    void load_finals_data_from_file (std::string const & filepath);
    void parse_finals_data (std::string const & finals_data_str);

    finals_data finals_data_for_time (double jd_utc);

	inline std::vector<finals_data> values () { return finals_data_values;  }

private:
    std::vector<finals_data> finals_data_values;
    finals_data_handler ();
    ~finals_data_handler ();
};
