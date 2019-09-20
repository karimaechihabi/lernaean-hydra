
#ifndef masslib_mass_file_loaders_h
#define masslib_mass_file_loaders_h
#include "../config.h"
#include "../globals.h"
#include "ts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mass_instance.h"
#include "calc_utils.h"

enum response mass_new_binary_file(const char *ifilename,
				   unsigned long long query_size,
				   unsigned int q_num,
				   struct mass_instance *mass_inst,
				   float minimum_distance);
#endif
