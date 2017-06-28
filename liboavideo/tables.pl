#!/usr/bin/perl

my @coeffs = ( 1.4075, 0.3455, 0.7169, 1.7790 );

foreach my $coeff ( @coeffs ) {
  my $n = sprintf ( "%1.4f", $coeff );
  $n =~ s/\./_/;
  print "static float lut_$n"."[256] = {\n";
  for ( my $v = 0; $v < 256; $v++ ) {
    my $lut = ( $v - 128 ) * $coeff;
    if ( $v % 8 == 0 ) {
      print "    ";
    }
    printf ( "%3.2f", $lut );
    if ( $v < 255 ) {
      print ",";
    }
    if ( $v % 8 < 7 ) {
      print " ";
    } else {
      print "\n";
    }
  }
  print "};\n\n";
}
