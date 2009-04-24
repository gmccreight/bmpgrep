/*****************************************************************************
******************************************************************************

bmpgrep

Author: Gordon McCreight
email: gordon@mccreight.com

date modified: 2009-04-24
version: 0.07

License: GPL 2
Copyright: 2009 by Gordon McCreight

usage:
  bmpgrep return_how_many_matches pattern_threshold (the line continues...)
    tolerance_r tolerance_g tolerance_b big.bmp small.bmp
  
If return_how_many_matches is set to 0, then it will find as many as it can.

"pattern_threshold" determines how aggressively it tries to shrink the pattern
it creates for the small image.  We recommend something around 100.  A value of
0 would only skip pixels that are exactly the same color.  Over 100 tends to
cause false positive matches, because there may not be enough pixels to check
on.

tolerances are 0-255

description: Find the location of a small BMP within a big one.
Prints a comma seperated list of x,y (for one match)
or x,y,x,y,x,y for multiple matches.

Note: Can be compiled like so:
g++ -o bmpgrep bmpgrep.cpp EasyBMP.cpp

After you compile, you might want to test with this (it should give
you six numbers):
./bmpgrep 0 100 0 0 0 test_images/big.bmp test_images/small.bmp

TODO: Better options verification and add help information.
******************************************************************************
*****************************************************************************/

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
    int last_pattern_pixel_brightness = -1000000;
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
    
    // If it is possible that the last pixel was skipped, then ensure that
    // it is appended to the pattern.  It is possible that the last pixel
    // wasn't skipped, so this will add it again to the pattern, but that's
    // not a very big deal.  I do it here, duplicating code, because I don't
    // want to check for pattern_threshold > 0 over and over in the inner
    // loop above.
    if ( pattern_threshold > 0 ) {
      RGBApixel* SmallPixel = Small(small_width - 1, small_height - 1);
      fast_pattern[small_pattern_array_size][0] = small_width - 1;
      fast_pattern[small_pattern_array_size][1] = small_height - 1;
      fast_pattern[small_pattern_array_size][2] = SmallPixel->Red;
      fast_pattern[small_pattern_array_size][3] = SmallPixel->Green;
      fast_pattern[small_pattern_array_size][4] = SmallPixel->Blue;
      small_pattern_array_size++;
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

    int big_offset_x = 0;
    int big_offset_y = 0;

    /* You don't need to check the whole big image.
       For example, if the small image is 100 pixels wide, then you
       know that there's no way it could match in the 99 right-most
       pixels of the big image.  The same idea is applicable for the height.
    */

    int max_y_to_check = big_height - small_height;
    int max_x_to_check = big_width - small_width;

    int has_written_results = 0;
    int has_matched_x_times = 0;
    
    int small_red, small_green, small_blue;

    for (big_y = 0; big_y < max_y_to_check; ++big_y) {
        for (big_x = 0; big_x < max_x_to_check; ++big_x) {

            for ( int small_pattern_index = 0;
                small_pattern_index < small_pattern_array_size;
                small_pattern_index++ ) {
            
                big_offset_x = fast_pattern[small_pattern_index][0];
                big_offset_y = fast_pattern[small_pattern_index][1];
                
                small_red = fast_pattern[small_pattern_index][2];
                small_green = fast_pattern[small_pattern_index][3];
                small_blue = fast_pattern[small_pattern_index][4];

                int all_channels_matched = 1;

                RGBApixel* BigPixel = Big(big_x + big_offset_x,
                    big_y + big_offset_y);

                if ( has_tolerances == false ) {
                    // zero tolerance, so do it faster
                    if ( BigPixel->Red != small_red ) {
                        all_channels_matched = 0;
                    }
                    else if ( BigPixel->Green != small_green ) {
                        all_channels_matched = 0;
                    }
                    else if ( BigPixel->Blue != small_blue ) {
                        all_channels_matched = 0;
                    }
                }
                else {
                    if ( Abs(BigPixel->Red - small_red)
                        > tolerance_r) {
                        all_channels_matched = 0;
                    }
                    else if (Abs(BigPixel->Green - small_green)
                        > tolerance_g) {
                        all_channels_matched = 0;
                    }
                    else if (Abs(BigPixel->Blue - small_blue)
                        > tolerance_b) {
                        all_channels_matched = 0;
                    }
                }

                if (! all_channels_matched) {
                    break;
                }

                // Last pixel of small pattern. There was a complete match!
                if (small_pattern_index == small_pattern_array_size - 1) {
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
    }

    
    if (has_written_results == 1) {
        cout << endl;
    }
    
    return 0;

}
