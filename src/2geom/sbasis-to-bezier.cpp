/*
 * Symmetric Power Basis - Bernstein Basis conversion routines
 *
 * Authors:
 *      Marco Cecchetti <mrcekets at gmail.com>
 *      Nathan Hurst <njh@mail.csse.monash.edu.au>
 *
 * Copyright 2007-2008  authors
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 */


#include <2geom/sbasis-to-bezier.h>
#include <2geom/choose.h>
#include <2geom/svg-path.h>
#include <2geom/exception.h>

#include <iostream>




namespace Geom
{

/*
 *  Symmetric Power Basis - Bernstein Basis conversion routines
 *
 *  some remark about precision:
 *  interval [0,1], subdivisions: 10^3
 *  - bezier_to_sbasis : up to degree ~39 precision is at least 10^-5
 *                       up to degree ~51 precision is at least 10^-3
 *  - sbasis_to_bezier : precision is at least 10^-5 even beyond order 100
 *
 *  interval [-1,1], subdivisions: 10^3
 *  - bezier_to_sbasis : up to degree ~22 precision is at least 10^-5
 *                       up to degree ~27 precision is at least 10^-3
 *  - sbasis_to_bezier : up to order ~23 precision is at least 10^-5
 *                       up to order ~27 precision is at least 10^-3
 *
 *  interval [-10,10], subdivisions: 10^3
 *  - bezier_to_sbasis : up to degree ~7 precision is at least 10^-5
 *                       up to degree ~9 precision is at least 10^-3
 *  - sbasis_to_bezier : up to order ~8 precision is at least 10^-5
 *                       up to order ~9 precision is at least 10^-3
 *
 *  references:
 *  this implementation is based on the following article:
 *  J.Sanchez-Reyes - The Symmetric Analogue of the Polynomial Power Basis
 */

inline
double binomial(unsigned int n, unsigned int k)
{
    return choose<double>(n, k);
}

inline
int sgn(unsigned int j, unsigned int k)
{
    assert (j >= k);
    // we are sure that j >= k
    return ((j-k) &  1u) ? -1 : 1;
}


void sbasis_to_bezier (Bezier & bz, SBasis const& sb, size_t sz)
{
    // if the degree is even q is the order in the symmetrical power basis,
    // if the degree is odd q is the order + 1
    // n is always the polynomial degree, i. e. the Bezier order
    size_t q, n;
    bool even;
    if (sz == 0)
    {
        q = sb.size();
        if (sb[q-1][0] == sb[q-1][1])
        {
            even = true;
            --q;
            n = 2*q;
        }
        else
        {
            even = false;
            n = 2*q-1;
        }
    }
    else
    {
        q = (sz > sb.size()) ?  sb.size() : sz;
        n = 2*sz-1;
        even = false;
    }
    bz.clear();
    bz.resize(n+1);
    double Tjk;
    for (size_t k = 0; k < q; ++k)
    {
        for (size_t j = k; j < n-k; ++j) // j <= n-k-1
        {
            Tjk = binomial(n-2*k-1, j-k);
            bz[j] += (Tjk * sb[k][0]);
            bz[n-j] += (Tjk * sb[k][1]); // n-k <-> [k][1]
        }
    }
    if (even)
    {
        bz[q] += sb[q][0];
    }
    // the resulting coefficients are with respect to the scaled Bernstein
    // basis so we need to divide them by (n, j) binomial coefficient
    for (size_t j = 1; j < n; ++j)
    {
        bz[j] /= binomial(n, j);
    }
}

void sbasis_to_bezier (std::vector<Point> & bz, D2<SBasis> const& sb, size_t sz)
{
    Bezier bzx, bzy;
    sbasis_to_bezier(bzx, sb[X], sz);
    sbasis_to_bezier(bzy, sb[Y], sz);
    size_t n = (bzx.size() >= bzy.size()) ? bzx.size() : bzy.size();

    bz.resize(n, Point(0,0));
    for (size_t i = 0; i < bzx.size(); ++i)
    {
        bz[i][X] = bzx[i];
    }
    for (size_t i = 0; i < bzy.size(); ++i)
    {
        bz[i][Y] = bzy[i];
    }
}


void bezier_to_sbasis (SBasis & sb, Bezier const& bz)
{
    // if the degree is even q is the order in the symmetrical power basis,
    // if the degree is odd q is the order + 1
    // n is always the polynomial degree, i. e. the Bezier order
    size_t n = bz.order();
    size_t q = (n+1) / 2;
    size_t even = (n & 1u) ? 0 : 1;
    sb.clear();
    sb.resize(q + even, Linear(0, 0));
    double Tjk;
    for (size_t k = 0; k < q; ++k)
    {
        for (size_t j = k; j < q; ++j)
        {
            Tjk = sgn(j, k) * binomial(n-j-k, j-k) * binomial(n, k);
            sb[j][0] += (Tjk * bz[k]);
            sb[j][1] += (Tjk * bz[n-k]); // n-j <-> [j][1]
        }
        for (size_t j = k+1; j < q; ++j)
        {
            Tjk = sgn(j, k) * binomial(n-j-k-1, j-k-1) * binomial(n, k);
            sb[j][0] += (Tjk * bz[n-k]);
            sb[j][1] += (Tjk * bz[k]);   // n-j <-> [j][1]
        }
    }
    if (even)
    {
        for (size_t k = 0; k < q; ++k)
        {
            Tjk = sgn(q,k) * binomial(n, k);
            sb[q][0] += (Tjk * (bz[k] + bz[n-k]));
        }
        sb[q][0] += (binomial(n, q) * bz[q]);
        sb[q][1] = sb[q][0];
    }
}


void bezier_to_sbasis (D2<SBasis> & sb, std::vector<Point> const& bz)
{
    size_t n = bz.size() - 1;
    size_t q = (n+1) / 2;
    size_t even = (n & 1u) ? 0 : 1;
    sb[X].clear();
    sb[Y].clear();
    sb[X].resize(q + even, Linear(0, 0));
    sb[Y].resize(q + even, Linear(0, 0));
    double Tjk;
    for (size_t k = 0; k < q; ++k)
    {
        for (size_t j = k; j < q; ++j)
        {
            Tjk = sgn(j, k) * binomial(n-j-k, j-k) * binomial(n, k);
            sb[X][j][0] += (Tjk * bz[k][X]);
            sb[X][j][1] += (Tjk * bz[n-k][X]);
            sb[Y][j][0] += (Tjk * bz[k][Y]);
            sb[Y][j][1] += (Tjk * bz[n-k][Y]);
        }
        for (size_t j = k+1; j < q; ++j)
        {
            Tjk = sgn(j, k) * binomial(n-j-k-1, j-k-1) * binomial(n, k);
            sb[X][j][0] += (Tjk * bz[n-k][X]);
            sb[X][j][1] += (Tjk * bz[k][X]);
            sb[Y][j][0] += (Tjk * bz[n-k][Y]);
            sb[Y][j][1] += (Tjk * bz[k][Y]);
        }
    }
    if (even)
    {
        for (size_t k = 0; k < q; ++k)
        {
            Tjk = sgn(q,k) * binomial(n, k);
            sb[X][q][0] += (Tjk * (bz[k][X] + bz[n-k][X]));
            sb[Y][q][0] += (Tjk * (bz[k][Y] + bz[n-k][Y]));
        }
        sb[X][q][0] += (binomial(n, q) * bz[q][X]);
        sb[X][q][1] = sb[X][q][0];
        sb[Y][q][0] += (binomial(n, q) * bz[q][Y]);
        sb[Y][q][1] = sb[Y][q][0];
    }
}


}  // end namespace Geom

namespace Geom{
#if 0

/* From Sanchez-Reyes 1997
   W_{j,k} = W_{n0j, n-k} = choose(n-2k-1, j-k)choose(2k+1,k)/choose(n,j)
     for k=0,...,q-1; j = k, ...,n-k-1
   W_{q,q} = 1 (n even)

This is wrong, it should read
   W_{j,k} = W_{n0j, n-k} = choose(n-2k-1, j-k)/choose(n,j)
     for k=0,...,q-1; j = k, ...,n-k-1
   W_{q,q} = 1 (n even)

*/
double W(unsigned n, unsigned j, unsigned k) {
    unsigned q = (n+1)/2;
    if((n & 1) == 0 && j == q && k == q)
        return 1;
    if(k > n-k) return W(n, n-j, n-k);
    assert((k <= q));
    if(k >= q) return 0;
    //assert(!(j >= n-k));
    if(j >= n-k) return 0;
    //assert(!(j < k));
    if(j < k) return 0;
    return choose<double>(n-2*k-1, j-k) /
        choose<double>(n,j);
}


// this produces a degree 2q bezier from a degree k sbasis
Bezier
sbasis_to_bezier(SBasis const &B, unsigned q) {
    if(q == 0) {
        q = B.size();
        /*if(B.back()[0] == B.back()[1]) {
            n--;
            }*/
    }
    unsigned n = q*2;
    Bezier result = Bezier(Bezier::Order(n-1));
    if(q > B.size())
        q = B.size();
    n--;
    for(unsigned k = 0; k < q; k++) {
        for(unsigned j = 0; j <= n-k; j++) {
            result[j] += (W(n, j, k)*B[k][0] +
                          W(n, n-j, k)*B[k][1]);
        }
    }
    return result;
}

double mopi(int i) {
    return (i&1)?-1:1;
}

// WARNING: this is wrong!
// this produces a degree k sbasis from a degree 2q bezier
SBasis
bezier_to_sbasis(Bezier const &B) {
    unsigned n = B.size();
    unsigned q = (n+1)/2;
    SBasis result;
    result.resize(q+1);
    for(unsigned k = 0; k < q; k++) {
        result[k][0] = result[k][1] = 0;
        for(unsigned j = 0; j <= n-k; j++) {
            result[k][0] += mopi(int(j)-int(k))*W(n, j, k)*B[j];
            result[k][1] += mopi(int(j)-int(k))*W(n, j, k)*B[j];
            //W(n, n-j, k)*B[k][1]);
        }
    }
    return result;
}

// this produces a 2q point bezier from a degree q sbasis
std::vector<Geom::Point>
sbasis_to_bezier(D2<SBasis> const &B, unsigned qq) {
    std::vector<Geom::Point> result;
    if(qq == 0) {
        qq = sbasis_size(B);
    }
    unsigned n = qq * 2;
    result.resize(n, Geom::Point(0,0));
    n--;
    for(unsigned dim = 0; dim < 2; dim++) {
        unsigned q = qq;
        if(q > B[dim].size())
            q = B[dim].size();
        for(unsigned k = 0; k < q; k++) {
            for(unsigned j = 0; j <= n-k; j++) {
                result[j][dim] += (W(n, j, k)*B[dim][k][0] +
                             W(n, n-j, k)*B[dim][k][1]);
                }
        }
    }
    return result;
}
/*
template <unsigned order>
D2<Bezier<order> > sbasis_to_bezier(D2<SBasis> const &B) {
    return D2<Bezier<order> >(sbasis_to_bezier<order>(B[0]), sbasis_to_bezier<order>(B[1]));
}
*/
#endif

#if 0 // using old path
//std::vector<Geom::Point>
// mutating
void
subpath_from_sbasis(Geom::OldPathSetBuilder &pb, D2<SBasis> const &B, double tol, bool initial) {
    assert(B.IS_FINITE());
    if(B.tail_error(2) < tol || B.size() == 2) { // nearly cubic enough
        if(B.size() == 1) {
            if (initial) {
                pb.start_subpath(Geom::Point(B[0][0][0], B[1][0][0]));
            }
            pb.push_line(Geom::Point(B[0][0][1], B[1][0][1]));
        } else {
            std::vector<Geom::Point> bez = sbasis_to_bezier(B, 2);
            if (initial) {
                pb.start_subpath(bez[0]);
            }
            pb.push_cubic(bez[1], bez[2], bez[3]);
        }
    } else {
        subpath_from_sbasis(pb, compose(B, Linear(0, 0.5)), tol, initial);
        subpath_from_sbasis(pb, compose(B, Linear(0.5, 1)), tol, false);
    }
}

/*
* This version works by inverting a reasonable upper bound on the error term after subdividing the
* curve at $a$.  We keep biting off pieces until there is no more curve left.
*
* Derivation: The tail of the power series is $a_ks^k + a_{k+1}s^{k+1} + \ldots = e$.  A
* subdivision at $a$ results in a tail error of $e*A^k, A = (1-a)a$.  Let this be the desired
* tolerance tol $= e*A^k$ and invert getting $A = e^{1/k}$ and $a = 1/2 - \sqrt{1/4 - A}$
*/
void
subpath_from_sbasis_incremental(Geom::OldPathSetBuilder &pb, D2<SBasis> B, double tol, bool initial) {
    const unsigned k = 2; // cubic bezier
    double te = B.tail_error(k);
    assert(B[0].IS_FINITE());
    assert(B[1].IS_FINITE());

    //std::cout << "tol = " << tol << std::endl;
    while(1) {
        double A = std::sqrt(tol/te); // pow(te, 1./k)
        double a = A;
        if(A < 1) {
            A = std::min(A, 0.25);
            a = 0.5 - std::sqrt(0.25 - A); // quadratic formula
            if(a > 1) a = 1; // clamp to the end of the segment
        } else
            a = 1;
        assert(a > 0);
        //std::cout << "te = " << te << std::endl;
        //std::cout << "A = " << A << "; a=" << a << std::endl;
        D2<SBasis> Bs = compose(B, Linear(0, a));
        assert(Bs.tail_error(k));
        std::vector<Geom::Point> bez = sbasis_to_bezier(Bs, 2);
        reverse(bez.begin(), bez.end());
        if (initial) {
          pb.start_subpath(bez[0]);
          initial = false;
        }
        pb.push_cubic(bez[1], bez[2], bez[3]);

// move to next piece of curve
        if(a >= 1) break;
        B = compose(B, Linear(a, 1));
        te = B.tail_error(k);
    }
}

#endif

/*
 * If only_cubicbeziers is true, the resulting path may only contain CubicBezier curves.
 */
void build_from_sbasis(Geom::PathBuilder &pb, D2<SBasis> const &B, double tol, bool only_cubicbeziers) {
    if (!B.isFinite()) {
        THROW_EXCEPTION("assertion failed: B.isFinite()");
    }
    if(tail_error(B, 2) < tol || sbasis_size(B) == 2) { // nearly cubic enough
        if( !only_cubicbeziers && (sbasis_size(B) <= 1) ) {
            pb.lineTo(B.at1());
        } else {
            std::vector<Geom::Point> bez;
            sbasis_to_bezier(bez, B, 2);
            pb.curveTo(bez[1], bez[2], bez[3]);
        }
    } else {
        build_from_sbasis(pb, compose(B, Linear(0, 0.5)), tol, only_cubicbeziers);
        build_from_sbasis(pb, compose(B, Linear(0.5, 1)), tol, only_cubicbeziers);
    }
}

/*
 * If only_cubicbeziers is true, the resulting path may only contain CubicBezier curves.
 */
Path
path_from_sbasis(D2<SBasis> const &B, double tol, bool only_cubicbeziers) {
    PathBuilder pb;
    pb.moveTo(B.at0());
    build_from_sbasis(pb, B, tol, only_cubicbeziers);
    pb.finish();
    return pb.peek().front();
}

/*
 * If only_cubicbeziers is true, the resulting path may only contain CubicBezier curves.
 */
//TODO: some of this logic should be lifted into svg-path
std::vector<Geom::Path>
path_from_piecewise(Geom::Piecewise<Geom::D2<Geom::SBasis> > const &B, double tol, bool only_cubicbeziers) {
    Geom::PathBuilder pb;
    if(B.size() == 0) return pb.peek();
    Geom::Point start = B[0].at0();
    pb.moveTo(start);
    for(unsigned i = 0; ; i++) {
        if(i+1 == B.size() || !are_near(B[i+1].at0(), B[i].at1(), tol)) {
            //start of a new path
            if(are_near(start, B[i].at1()) && sbasis_size(B[i]) <= 1) {
                pb.closePath();
                //last line seg already there (because of .closePath())
                goto no_add;
            }
            build_from_sbasis(pb, B[i], tol, only_cubicbeziers);
            if(are_near(start, B[i].at1())) {
                //it's closed, the last closing segment was not a straight line so it needed to be added, but still make it closed here with degenerate straight line.
                pb.closePath();
            }
          no_add:
            if(i+1 >= B.size()) break;
            start = B[i+1].at0();
            pb.moveTo(start);
        } else {
            build_from_sbasis(pb, B[i], tol, only_cubicbeziers);
        }
    }
    pb.finish();
    return pb.peek();
}

}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
