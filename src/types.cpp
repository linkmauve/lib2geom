/** \file
 * Implements Geom::Point::normalize()
 */

#include "point.h"
#include "point-ops.h"
#include <assert.h>

#include "isnan.h" //temporary fix for isnan()

/** Scales this vector to make it a unit vector (within rounding error).
 *
 *  The current version tries to handle infinite coordinates gracefully,
 *  but it's not clear that any callers need that.
 *
 *  \pre \f$this \neq (0, 0)\f$
 *  \pre Neither component is NaN.
 *  \post \f$-\epsilon<\left|this\right|-1<\epsilon\f$
 */
void Geom::Point::normalize() {
	double len = hypot(_pt[0], _pt[1]);
	if(len == 0) return;
	if(isNaN(len)) return;
	static double const inf = 1e400;
	if(len != inf) {
		*this /= len;
	} else {
		unsigned n_inf_coords = 0;
		/* Delay updating pt in case neither coord is infinite. */
		Geom::Point tmp;
		for ( unsigned i = 0 ; i < 2 ; ++i ) {
			if ( _pt[i] == inf ) {
				++n_inf_coords;
				tmp[i] = 1.0;
			} else if ( _pt[i] == -inf ) {
				++n_inf_coords;
				tmp[i] = -1.0;
			} else {
				tmp[i] = 0.0;
			}
		}
		switch (n_inf_coords) {
		case 0:
			/* Can happen if both coords are near +/-DBL_MAX. */
			*this /= 4.0;
			len = hypot(_pt[0], _pt[1]);
			assert(len != inf);
			*this /= len;
			break;

		case 1:
			*this = tmp;
			break;

		case 2:
			*this = sqrt(0.5) * tmp;
			break;
		}
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