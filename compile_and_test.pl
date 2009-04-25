#!/usr/bin/perl

# version: 0.05
# description: You can use this script to compile and test the bmpgrep program.
#
# Usage: If you want to run a speed test, append a 1 after this script name
# To get a more representative set of numbers, you can do:
# for i in {1..3}; do ./compile_and_test.pl 1; done

use strict;
use warnings;

use Time::HiRes qw( gettimeofday tv_interval );

my $should_do_speed_test = $ARGV[0] || 0;

my $had_warning = 0;
my $should_do_compilation = 1;

my @programs = (
    {
        do_compile_and_test => 1,
        name => "bmpgrep",
        num_tests => 10,

        test_1 => "0 10 0 0 0 test_images/big.bmp test_images/small.bmp",
        test_1_description => "return all matches of an exact matching scheme",
        test_1_coderef => sub {
            my $r = shift;
            return 1 if $r =~ /^105,385,105,685,105,910(\r\n|\n)$/;
            return 0;
        },
        test_2 => "0 10 1 1 1 test_images/big.bmp test_images/small.bmp",
        test_2_description => "return all matches of a non-exact matching scheme",
        test_2_coderef => sub {
            my $r = shift;
            return 1 if $r =~ /^105,385,105,685,105,910(\r\n|\n)$/;
            return 0;
        },
        test_3 => "1 10 0 0 0 test_images/big.bmp test_images/small.bmp",
        test_3_description => "return only one match",
        test_3_coderef => sub {
            my $r = shift;
            return 1 if $r =~ /^105,385(\r\n|\n)$/;
            return 0;
        },
        test_4 => "2 10 0 0 0 test_images/big.bmp test_images/small.bmp",
        test_4_description => "return two matches",
        test_4_coderef => sub {
            my $r = shift;
            return 1 if $r =~ /^105,385,105,685(\r\n|\n)$/;
            return 0;
        },
        test_5 => "0 20 0 0 0 test_images/big.bmp test_images/small.bmp",
        test_5_description => "no false positives when pattern threshold is high.  Fixed by adding the last pixel.",
        test_5_coderef => sub {
            my $r = shift;
            return 1 if $r =~ /^105,385,105,685,105,910(\r\n|\n)$/;
            return 0;
        },
        test_6 => "0 0 0 0 0 test_images/big.bmp test_images/small_text.bmp",
        test_6_description => "perfect match on small text with no pattern threshold",
        test_6_coderef => sub {
            my $r = shift;
            return 1 if $r =~ /^851,540,851,603,851,666,851,792(\r\n|\n)$/;
            return 0;
        },
        test_7 => "0 20 0 0 0 test_images/big.bmp test_images/small_text.bmp",
        test_7_description => "perfect match on small text with a big pattern threshold",
        test_7_coderef => sub {
            my $r = shift;
            return 1 if $r =~ /^851,540,851,603,851,666,851,792(\r\n|\n)$/;
            return 0;
        },
        test_8 => "0 20 0 0 0 test_images/big.bmp test_images/perl_folder.bmp",
        test_8_description => "perl folder matched",
        test_8_coderef => sub {
            my $r = shift;
            return 1 if $r =~ /^22,678(\r\n|\n)$/;
            return 0;
        },
        test_9 => "0 30 0 0 0 test_images/big.bmp test_images/large_sub_image.bmp",
        test_9_description => "the large sub image",
        test_9_coderef => sub {
            my $r = shift;
            return 1 if $r =~ /^9,434(\r\n|\n)$/;
            return 0;
        },
        test_10 => "0 10 0 0 0 test_images/big.bmp test_images/movie_icon.bmp",
        test_10_description => "movie icon",
        test_10_coderef => sub {
            my $r = shift;
            return 1 if $r =~ /^731,531,731,594,731,657,731,783(\r\n|\n)$/;
            return 0;
        },
    },
);

if ( $should_do_speed_test ) {

  # Do one full run, including compilation, before starting the timing.
  main();

  # Turn off compilation and do five runs while timing
  $should_do_compilation = 0;
  my $t0 = [gettimeofday];  
  for my $i ( 1..5 ) {
    main();
  }
  my $elapsed = tv_interval($t0);
  
  print "speed test elapsed time: $elapsed\n";
}
else {
  main();
}

sub main {
    my $total_num_tests_run = 0;

    PROGRAM:
    for my $program (@programs) {

        next PROGRAM if $program->{do_compile_and_test} != 1;
        
        if ( $should_do_compilation ) {

          # In cygwin on Windows, it compiles with a .exe extension
          if ( -f "$program->{name}.exe") {
              unlink("$program->{name}.exe");
          }
          else {
              unlink($program->{name});
          }
          
          system("g++ -o $program->{name} $program->{name}.cpp EasyBMP.cpp");
        
        }

        if ( -f "$program->{name}.exe" || -f "$program->{name}" ) {
            for my $test_num (1..$program->{num_tests}) {
                my $results = `./$program->{name} $program->{"test_$test_num"}`;
                if (! $program->{"test_${test_num}_coderef"}->($results) ) {
                    my_warn("test ${test_num} for $program->{name} failed with the results $results");
                }
                $total_num_tests_run++;
            }
        }
        else {
            my_warn("it doesn't look like $program->{name} compiled correctly.  It's possible that you don't have g++ installed on your system.");
        }
    }

    if ($had_warning) {
        print "not ok: had a warning... tests didn't pass flawlessly\n";
    }
    else {
        print "ok: compiled ok and the $total_num_tests_run tests passed\n";
    }
}

sub my_warn {
    my $message = shift;
    warn $message;
    $had_warning = 1;
}