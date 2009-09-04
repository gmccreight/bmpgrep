/*****************************************************************************
******************************************************************************

bmpgrep

Author: Gordon McCreight
email: gordon@mccreight.com

date modified: 2009-04-25
version: 0.08

License: GPL 2
Copyright: 2009 by Gordon McCreight

usage:
  bmpgrep return_how_many_matches pattern_threshold (the line continues...)
    tolerance_r tolerance_g tolerance_b big.bmp small.bmp

If return_how_many_matches is set to 0, then it will find as many as it can.

"pattern_threshold" determines how aggressively it tries to shrink the pattern
it creates for the small image.  We recommend something around 30.  A value of
0 skips no pixels.  Over 100 tends to cause false positive matches, because
there may not be enough pixels to check.

tolerances are 0-255

description: Find the location of a small BMP within a big one.
Prints a comma seperated list of x,y (for one match)
or x,y,x,y,x,y for multiple matches.

Note: Can be compiled like so:
g++ -o bmpgrep bmpgrep.cpp EasyBMP.cpp

After you compile, you might want to test with this (the result should be
six numbers long):
./bmpgrep 0 30 0 0 0 test_images/big.bmp test_images/small.bmp

The program also ships with a compile_and_test.pl perl script that
pushes the tests a bit harder.  You can also run that test script
with the option 1 ( so, like ./compile_and_test.pl 1 ) and it will
run it as a performance test.

One note, we modify the stock EasyBMP library in one place where is does
bounds checking on the pixel requested.  We've removed the bounds checking.
This speeds things up a bit.

TODO: Better options verification and add help information.
******************************************************************************
*****************************************************************************/

#include <stdlib.h>
#include "EasyBMP.h"
using namespace std;

// This needs to be created as a global because it potentially needs to be
// really big.  Arrays allocated inside a function cannot be this big.
int fast_pattern[800 * 600][5];

static inline double Abs (double Nbr) {
    if( Nbr >= 0 )
        return Nbr;
    else
        return -Nbr;
}

int main( int argc, char* argv[] ) {

    int optind = 1;

    int return_how_many_matches = atoi(argv[ optind ]);
    optind++;

    int pattern_threshold = atoi(argv[ optind ]);
    optind++;

    int tolerance_r = atoi(argv[ optind ]);
    optind++;

    int tolerance_g = atoi(argv[ optind ]);
    optind++;

    int tolerance_b = atoi(argv[ optind ]);
    optind++;

    int has_tolerances = false;
    if (tolerance_r > 0 || tolerance_g > 0 || tolerance_b > 0) {
        has_tolerances = true;
    }

    BMP Big;
    Big.ReadFromFile(argv[ optind ]);
    optind++;

    BMP Small;
    Small.ReadFromFile(argv[ optind ]);
    optind++;

    int big_x, big_y, small_x, small_y;

    int big_height = Big.TellHeight();
    int big_width = Big.TellWidth();
    int small_height = Small.TellHeight();
    int small_width = Small.TellWidth();

    if ( small_height * small_width > 800 * 600 ) {
        cout << "We can only handle small images that contain fewer than"
          << "800 x 600 pixels" << endl;
        return 0;
    }

    int small_pattern_array_size = 0;
    int last_pattern_pixel_brightness = -1;
    for (small_y = 0; small_y < small_height; small_y++) {
        for (small_x = 0; small_x < small_width; small_x++) {
            RGBApixel* SmallPixel = Small(small_x, small_y);
            int this_pixel_brightness = SmallPixel->Red
              + SmallPixel->Green + SmallPixel->Blue;
            if ( Abs( this_pixel_brightness - last_pattern_pixel_brightness )
              >= pattern_threshold ) {
                fast_pattern[small_pattern_array_size][0] = small_x;
                fast_pattern[small_pattern_array_size][1] = small_y;
                fast_pattern[small_pattern_array_size][2] = SmallPixel->Red;
                fast_pattern[small_pattern_array_size][3] = SmallPixel->Green;
                fast_pattern[small_pattern_array_size][4] = SmallPixel->Blue;

                last_pattern_pixel_brightness = this_pixel_brightness;
                small_pattern_array_size++;
            }
        }
    }
 
    //#define DEBUG_THE_FAST_PATTERN
    #ifdef DEBUG_THE_FAST_PATTERN
    for ( int pattern_index = 0; pattern_index < 5; pattern_index++ ) {
        for ( int part = 0; part < 5; part++ ) {
            cout << fast_pattern[pattern_index][part] << endl;
        }
        cout << endl;
    }
    cout << small_pattern_array_size << endl;
    return 0;
    #endif

    /*
    You don't need to check the whole big image.
    For example, if the small image is 100 pixels wide, then you
    know that there's no way it could match in the 99 right-most
    pixels of the big image.  The same idea is applicable for the height.
    */

    int max_y_to_check = big_height - small_height;
    int max_x_to_check = big_width - small_width;

    int has_written_results = 0;
    int has_matched_x_times = 0;

    /*
    This is declared here instead of inside the inner "pattern" for loop
    because we use it after the for loop is completed to check if there
    was a perfect match
    */
    int small_pattern_index = 0;

    for (big_y = 0; big_y < max_y_to_check; ++big_y) {
        for (big_x = 0; big_x < max_x_to_check; ++big_x) {

            for ( small_pattern_index = 0;
                small_pattern_index < small_pattern_array_size;
                small_pattern_index++ ) {

                RGBApixel* BigPixel = Big(big_x + fast_pattern[small_pattern_index][0],
                               big_y + fast_pattern[small_pattern_index][1]);

                /*
                Do these as preprocessor macros for two reasons.
                The first is that they're used in two places, and
                the code looks a lot cleaner.
                The second is that it's better than writing them
                to variables, since they may not all three be used
                in each comparison, and variables are overkill anyhow.
                */
                #define SMALL_RED fast_pattern[small_pattern_index][2]
                #define SMALL_GREEN fast_pattern[small_pattern_index][3]
                #define SMALL_BLUE fast_pattern[small_pattern_index][4]

                if ( has_tolerances == false ) {
                    // zero tolerance, so do it faster
                    if ( BigPixel->Red != SMALL_RED ) {
                        break;
                    }
                    else if ( BigPixel->Green != SMALL_GREEN ) {
                        break;
                    }
                    else if ( BigPixel->Blue != SMALL_BLUE ) {
                        break;
                    }
                }
                else {
                    if ( Abs(BigPixel->Red - SMALL_RED )
                        > tolerance_r ) {
                        break;
                    }
                    else if ( Abs(BigPixel->Green - SMALL_GREEN )
                        > tolerance_g ) {
                        break;
                    }
                    else if ( Abs(BigPixel->Blue - SMALL_BLUE )
                        > tolerance_b ) {
                        break;
                    }
                }
            }

            // There was a complete match!  Note that this check
            // is done after the for loop, not inside it.  Checking
            // outside the loop is a bit faster.
            if (small_pattern_index == small_pattern_array_size) {
              if (has_written_results == 1) {
                cout << ",";
              }
              cout << big_x << "," << big_y;
              has_matched_x_times++;

              if (has_matched_x_times == return_how_many_matches) {
                  cout << endl;
                  return 0;
              }

              has_written_results = 1;
            }
        }
    }


    if (has_written_results == 1) {
        cout << endl;
    }

    return 0;

}
