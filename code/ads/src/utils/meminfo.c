#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <libgen.h>

#include <ads.h>


//==========================================================================//
// The Command-Line Arguments                                               //
//==========================================================================//

static const char* SHORT_OPTIONS = "i:h";

static struct option LONG_OPTIONS[] =
{
        {"index"        , required_argument, 0, 'i'},
        {"help"         , no_argument,       0, 'h'},
        {0, 0, 0, 0}
};


/**
 * Print usage infomration
 *
 * @param arg0 the first element in the argv array.
 */
static void usage(const char* arg0) 
{
	char* s = strdup(arg0);
	char* p = basename(s);
    fprintf(stderr, "Usage: %s [OPTIONS] DATABASE_FOLDER\n\n", p);
    free(s);

    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -i, --index DIR       Set the index directory\n");
    fprintf(stderr, "  -h, --help            Show this usage information and exit\n");
}


//==========================================================================//
// The Main Function                                                        //
//==========================================================================//

int main(int argc, char **argv) 
{
    char* index_directory = NULL;	
 	
 	// Pase the command-line arguments

    int option_index = 0;
    while (1) {
        int c = getopt_long(argc, argv, SHORT_OPTIONS, LONG_OPTIONS, &option_index);

        if (c == -1) break;

        switch (c) {

            case 'i':
                index_directory = optarg;
                break;
        
            case 'h':
                usage(argv[0]);
                return 0;
                break;

            case '?':
            case ':':
                return 1;

            default: 
        			abort();
        }
    }

    if(index_directory) 
    {
	    isax_index *idx = index_read(index_directory);
	    cache_sax_file(idx);
	    // idx->settings->tight_bound = tight_bound;
	    // idx->settings->aggressive_check = aggressive_check;
	    // idx->settings->total_loaded_leaves = total_loaded_leaves;
	    // idx->settings->min_leaf_size = min_leaf_size;
	    //print_settings(idx->settings);

	    print_mem_info(idx);
	    isax_index_destroy(idx, NULL);
    }

    return 1;
}