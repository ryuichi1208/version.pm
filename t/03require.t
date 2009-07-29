#! /usr/local/perl -w
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

use Test::More qw/no_plan/;


# Don't want to use, because we need to make sure that the import doesn't
# fire just yet (some code does this to avoid importing qv() and delare()).
require_ok("version");
is $version::VERSION, 0.7701, "Make sure we have the correct class";
ok(!"main"->can("qv"), "We don't have the imported qv()");
ok(!"main"->can("declare"), "We don't have the imported declare()");

my $Verbose;
require "t/coretests.pm";

diag "Tests with base class" if $Verbose;

BaseTests("version","new",undef);
BaseTests("version","parse",undef);
