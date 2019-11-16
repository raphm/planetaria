#ifndef ZBRENT_H
#define ZBRENT_H

#include <vector>
#include <complex>

/*
* bracket_roots -- inward root bracketing
*
* Given <func>, defined over <in_range>, divide <in_range> into <intervals>
* subranges, and if the subrange brackets a root, add it to out_ranges.
* The maximum number of subranges to report is determined by
* <out_ranges_size>.
*
*/

template <typename T>
void zbrak (T & fx, const double x1, const double x2, const int n, std::vector<double> & xb1, std::vector<double> & xb2, int & nroot) {
    int nb = 20;
    xb1.resize (nb);
    xb2.resize (nb);
    nroot = 0;
    double dx = (x2 - x1) / n;
    double x = x1;
    double fp = fx (x1);
    for (int i = 0; i < n; i++) {
        x += dx;
        double fc = fx (x);
        if (fc*fp <= 0.0) {
            xb1[nroot] = x - dx;
            xb2[nroot++] = x;
            if (nroot == nb) {
                std::vector<double> tempvec1 (xb1), tempvec2 (xb2);
                xb1.resize (2 * nb);
                xb2.resize (2 * nb);
                for (int j = 0; j < nb; j++) {
                    xb1[j] = tempvec1[j];
                    xb2[j] = tempvec2[j];
                }
                nb *= 2;
            }
        }
        fp = fc;
    }
}

template<class T>
inline T SIGN (const T &a, const T &b) {
    return b >= 0 ? (a >= 0 ? a : -a) : (a >= 0 ? -a : a);
}

template <typename T>
double zbrent (T &func, const double x1, const double x2, const double tol) {
    const int ITMAX = 100;
    const double EPS = std::numeric_limits<double>::epsilon ();
    double a = x1, b = x2, c = x2, d, e, fa = func (a), fb = func (b), fc, p, q, r, s, tol1, xm;
    if ((fa > 0.0 && fb > 0.0) || (fa < 0.0 && fb < 0.0))
        throw("Root must be bracketed in zbrent");
    fc = fb;
    for (int iter = 0; iter < ITMAX; iter++) {
        if ((fb > 0.0 && fc > 0.0) || (fb < 0.0 && fc < 0.0)) {
            c = a;
            fc = fa;
            e = d = b - a;
        }
        if (std::abs (fc) < std::abs (fb)) {
            a = b;
            b = c;
            c = a;
            fa = fb;
            fb = fc;
            fc = fa;
        }
        tol1 = 2.0*EPS*std::abs (b) + 0.5*tol;
        xm = 0.5*(c - b);
        if (std::abs (xm) <= tol1 || fb == 0.0) return b;
        if (std::abs (e) >= tol1 && std::abs (fa) > std::abs (fb)) {
            s = fb / fa;
            if (a == c) {
                p = 2.0*xm*s;
                q = 1.0 - s;
            } else {
                q = fa / fc;
                r = fb / fc;
                p = s * (2.0*xm*q*(q - r) - (b - a)*(r - 1.0));
                q = (q - 1.0)*(r - 1.0)*(s - 1.0);
            }
            if (p > 0.0) q = -q;
            p = std::abs (p);
            double min1 = 3.0*xm*q - std::abs (tol1*q);
            double min2 = std::abs (e*q);
            if (2.0*p < (min1 < min2 ? min1 : min2)) {
                e = d;
                d = p / q;
            } else {
                d = xm;
                e = d;
            }
        } else {
            d = xm;
            e = d;
        }
        a = b;
        fa = fb;
        if (std::abs (d) > tol1)
            b += d;
        else
            b += SIGN (tol1, xm);
        fb = func (b);
    }
    throw("Maximum number of iterations exceeded in zbrent");
}

#endif
