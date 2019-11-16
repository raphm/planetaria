#include "vec3.h"

#include <cmath>

vec3::vec3 () :x(0.0), y(0.0), z(0.0) {}
vec3::~vec3 () {}

void vec3::normalize () {
    // V/|V| = (x/|V|, y/|V|, z/|V|)
    const double mag = magnitude ();
    const double nx = x/mag;
    const double ny = y/mag;
    const double nz = z/mag;
    x=nx;
    y=ny;
    z=nz;
}

double vec3::magnitude () const {
    // V = (x, y, z), |V| = sqrt (x*x + y*y + z*z)
    const double mx = x;
    const double my = y;
    const double mz = z;
    return std::sqrt (mx*mx + my*my + mz*mz);
}

double vec3::dot (vec3 const v0, vec3 const v1) {
    // Ax * Bx + Ay * By + Az * Bz
    return v0.x*v1.x + v0.y * v1.y + v0.z * v1.z;
}

vec3 vec3::cross (vec3 const v0, vec3 const v1) {
    vec3 v_cross;

    // x = Ay * Bz - By * Az
    v_cross.x = v0.y*v1.z - v1.y*v0.z;
    // y = Az * Bx - Bz * Ax
    v_cross.y = v0.z*v1.x - v1.z*v0.x;
    // z = Ax * By - Bx * Ay
    v_cross.z = v0.x*v1.y - v1.x*v0.y;

    return v_cross;
}

double vec3::angle (vec3 const v0, vec3 const v1) {
    vec3 a (v0);
    vec3 b (v1);
    a.normalize ();
    b.normalize ();
    return acos (vec3::dot (a, b));
}
