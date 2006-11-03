#include "path.h"

/*** Routines in this group return a path that looks the same, but
 * include extra knots for certain points of interest. */

/*** find_vector_extreme_points
 * extreme points . dir.
 */

double arc_length_subdividing(Geom::Path const & p, double tol);
double arc_length_integrating(Geom::Path const & p, double tol);
double arc_length_integrating(Geom::Path const & p, Geom::Path::Location const & pl, double tol);


Geom::Path::Location natural_parameterisation(Geom::Path const & p, double s, double tol);

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(substatement-open . 0))
  indent-tabs-mode:nil
  c-brace-offset:0
  fill-column:99
  End:
  vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/