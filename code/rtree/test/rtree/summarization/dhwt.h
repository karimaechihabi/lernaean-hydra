/******************************************************************************
 * Copyright (C) 2013-2014, Kostas Zoumpatianos <zoumpatianos@disi.unitn.eu>  *
 * All rights reserved.                                                       *
 *                                                                            *
 * For the licensing terms see $ROOTSYS/LICENSE.                              *
 * For the list of contributors see $ROOTSYS/README/CREDITS.                  *
 ******************************************************************************/

#ifndef DHWT_H_
#define DHWT_H_

int dhwt_from_ts(ts_type *ts_in, ts_type *dhwt_out, int original_points);
float minidist_dhwt_to_dhwt(ts_type *dhwt1, ts_type *dhwt2, int original_points,
		                    int reduced_points);
float test_dhwt_distance(ts_type *ts1, ts_type *ts2, int size);
#endif /* DHWT_H_ */
