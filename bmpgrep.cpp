/*****************************************************************************
******************************************************************************

bmpgrep

Author: Gordon McCreight
email: gordon@mccreight.com

date modified: 2008-12-17
version: 0.05

License: GPL 2
Copyright: 2007 by Gordon McCreight

usage: bmpgrep tolerance_r tolerance_g tolerance_b big.bmp small.bmp
tolerances are 0-255

description: Find the location of a small BMP within a big one.
Prints a comma seperated list of x,y (for one match)
or x,y,x,y,x,y for multiple matches.

Note: Can be compiled like so:
g++ -o bmpgrep bmpgrep.cpp EasyBMP.cpp

After you compile, you might want to test with this:
./bmpgrep 0 0 0 test_images/big.bmp test_images/small.bmp

TODO: Better options verification and add help information.
******************************************************************************
*****************************************************************************/

#include "EasyBMP.h"
using namespace std;

//#define DEBUG

//[tag:performance:gem] Not sure if the "static inline" does any good.
static inline double Abs (double Nbr) {
    if( Nbr >= 0 )
        return Nbr;
    else
        return -Nbr;
}

int main( int argc, char* argv[] ) {

    int optind = 1;

    int tolerance_r = atoi(argv[ optind ]);
    optind++;

    int tolerance_g = atoi(argv[ optind ]);
    optind++;

    int tolerance_b = atoi(argv[ optind ]);
    optind++;

    int has_tolerance_bool = 0;
    if (tolerance_r > 0 || tolerance_g > 0 || tolerance_b > 0) {
        has_tolerance_bool = 1;
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

    for (big_y = 0; big_y < max_y_to_check; ++big_y) {
        for (big_x = 0; big_x < max_x_to_check; ++big_x) {

            big_offset_y = 0;
            for (small_y = 0; small_y < small_height; ++small_y) {

                int all_x_pixels_matched = 1;

                big_offset_x = 0;
                for (small_x = 0; small_x < small_width; ++small_x) {

                    int all_channels_matched = 1;

                    RGBApixel* BigPixel = Big(big_x + big_offset_x,
                        big_y + big_offset_y);
                    RGBApixel* SmallPixel = Small(small_x, small_y);

                    if ( has_tolerance_bool == 0 ) {
                        // zero tolerance, so do it faster
                        if ( BigPixel->Red != SmallPixel->Red ) {
                            all_channels_matched = 0;
                        }
                        else if ( BigPixel->Green != SmallPixel->Green ) {
                            all_channels_matched = 0;
                        }
                        else if ( BigPixel->Blue != SmallPixel->Blue ) {
                            all_channels_matched = 0;
                        }
                    }
                    else {
                        if ( Abs(BigPixel->Red - SmallPixel->Red)
                            > tolerance_r) {
                            all_channels_matched = 0;
                        }
                        else if (Abs(BigPixel->Green -SmallPixel->Green)
                            > tolerance_g) {
                            all_channels_matched = 0;
                        }
                        else if (Abs(BigPixel->Blue - SmallPixel->Blue)
                            > tolerance_b) {
                            all_channels_matched = 0;
                        }
                    }

                    if (! all_channels_matched) {
                        all_x_pixels_matched = 0;
                        break;
                    }

                    // Last pixel of small image. There was a complete match!
                    if (small_x == small_width - 1
                        && small_y == small_height - 1) {
                      if (has_written_results == 1) {
                        cout << ",";
                      }
                      cout << big_x << "," << big_y;
                      has_written_results = 1;
                    }

                    big_offset_x++;
                }

                if (! all_x_pixels_matched) {
                    break;
                }

                big_offset_y++;
            }

        }
    }

    
    if (has_written_results == 1) {
        cout << endl;
    }
    
    return 0;

}
