#!/usr/bin/perl

# version: 0.04
# description: You can use this script to compile and test the
# bmpgrep program.

use strict;
use warnings;

my $had_warning = 0;

my @programs = (
    {
        do_compile_and_test => 1,
        name => "bmpgrep",
        num_tests => 7,

        test_1 => "0 100 0 0 0 test_images/big.bmp test_images/small.bmp",
        test_1_description => "return all matches of an exact matching scheme",
        test_1_coderef => sub {
            my $r = shift;
            return 1 if $r =~ /^105,385,105,685,105,910(\r\n|\n)$/;
            return 0;
        },
        test_2 => "0 100 1 1 1 test_images/big.bmp test_images/small.bmp",
        test_2_description => "return all matches of a non-exact matching scheme",
        test_2_coderef => sub {
            my $r = shift;
            return 1 if $r =~ /^105,385,105,685,105,910(\r\n|\n)$/;
            return 0;
        },
        test_3 => "1 100 0 0 0 test_images/big.bmp test_images/small.bmp",
        test_3_description => "return only one match",
        test_3_coderef => sub {
            my $r = shift;
            return 1 if $r =~ /^105,385(\r\n|\n)$/;
            return 0;
        },
        test_4 => "2 100 0 0 0 test_images/big.bmp test_images/small.bmp",
        test_4_description => "return two matches",
        test_4_coderef => sub {
            my $r = shift;
            return 1 if $r =~ /^105,385,105,685(\r\n|\n)$/;
            return 0;
        },
        test_5 => "0 200 0 0 0 test_images/big.bmp test_images/small.bmp",
        test_5_description => "some false positives when pattern threshold too high",
        test_5_coderef => sub {
            my $r = shift;
            return 1 if $r =~ /^181,10,105,385,181,460,181,610,105,685,105,910(\r\n|\n)$/;
            return 0;
        },
        test_6 => "0 0 0 0 0 test_images/big.bmp test_images/small_text.bmp",
        test_6_description => "perfect match on small text with no pattern threshold",
        test_6_coderef => sub {
            my $r = shift;
            return 1 if $r =~ /^851,540,851,603,851,666,851,792(\r\n|\n)$/;
            return 0;
        },
        test_7 => "0 200 0 0 0 test_images/big.bmp test_images/small_text.bmp",
        test_7_description => "perfect match match on small text with a big pattern threshold",
        test_7_coderef => sub {
            my $r = shift;
            return 1 if $r =~ /^851,540,851,603,851,666,851,792(\r\n|\n)$/;
            return 0;
        },
    },
);

my $total_num_tests_run = 0;

PROGRAM:
for my $program (@programs) {

    next PROGRAM if $program->{do_compile_and_test} != 1;

    # In cygwin on Windows, it compiles with a .exe extension
    if ( -f "$program->{name}.exe") {
        unlink("$program->{name}.exe");
    }
    else {
        unlink($program->{name});
    }
    
    system("g++ -o $program->{name} $program->{name}.cpp EasyBMP.cpp");

    if ( -f "$program->{name}.exe" || -f "$program->{name}" ) {
        for my $test_num (1..$program->{num_tests}) {
            my $results = `./$program->{name} $program->{"test_$test_num"}`;
            if (! $program->{"test_${test_num}_coderef"}->($results) ) {
                my_warn("the test for $program->{name} failed with the results $results");
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

sub my_warn {
    my $message = shift;
    warn $message;
    $had_warning = 1;
}