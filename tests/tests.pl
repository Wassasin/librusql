#!/usr/bin/perl
use strict;
use warnings;
use TAP::Harness;

my @test_args = @ARGV;

my @tests = qw(test_compile test_connect test_query test_placeholders test_optional test_multiconnection test_signedness test_insert_id test_iterate test_threads);

my $compiled_tests_dir;
for(qw(. tests ../tests ../build/tests)) {
	if(-f ("$_/" . $tests[0])) {
		$compiled_tests_dir = $_;
		last;
	}
}

if(!$compiled_tests_dir) {
	die "Couldn't find test: " . $tests[0] . " - did you compile them?\n";
}

my $harness = TAP::Harness->new({
	timer => 1,
	show_count => 1,
	test_args => \@test_args,
	color => 1,
	trap => 1,
	exec => ['env'],
});
$harness->runtests(map { "$compiled_tests_dir/$_" } @tests);
