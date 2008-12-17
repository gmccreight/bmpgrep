#!/usr/bin/perl

# version: 0.02
# description: You can use this script to compile and test the various
# bmp-compare programs.

use strict;
use warnings;

my $had_warning = 0;

my @programs = (
    {
        do_compile_and_test => 1,
        name => "bmpgrep",
        num_tests => 2,
        test_1 => "0 0 0 test_images/big.bmp test_images/small.bmp",
        test_1_coderef => sub {
            my $r = shift;
            return 1 if $r =~ /^105,385,105,685,105,910(\r\n|\n)$/;
            return 0;
        },
        test_2 => "1 1 1 test_images/big.bmp test_images/small.bmp",
        test_2_coderef => sub {
            my $r = shift;
            return 1 if $r =~ /^105,385,105,685,105,910(\r\n|\n)$/;
            return 0;
        },
    },
);

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
        }
    }
    else {
        my_warn("it doesn't look like $program->{name} compiled correctly.  It's possible that you don't have g++ installed on your system.");
    }
}

if ($had_warning) {
    print "had a warning... tests didn't pass flawlessly\n";
}
else {
    print "compiled ok and tests passed\n";
}

sub my_warn {
    my $message = shift;
    warn $message;
    $had_warning = 1;
}