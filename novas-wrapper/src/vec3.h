#pragma once
class vec3 {
public:
    vec3 ();
    ~vec3 ();
    void normalize ();
    double magnitude () const;
    static double dot (vec3 const v0, vec3 const v1);
    static vec3 cross (vec3 const v0, vec3 const v1);
    static double angle (vec3 const v0, vec3 const v1);

    double x, y, z;
};

