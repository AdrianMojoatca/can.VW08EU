use strict; use warnings;
sub strip { local $/; open(my $f,'<',$_[0]) or die $!; my $s=<$f>; close $f;
  $s =~ s{/\*.*?\*/}{}gs; $s =~ s{//[^\n]*}{}g; return $s; }
my $h = strip($ARGV[0]); my $c = strip($ARGV[1]);
# struct fields, in order
my ($body) = $h =~ /typedef struct\s*\{(.*?)\}\s*core_ds4_api_table_t;/s or die "struct not found";
my @fields;
for my $ln (split /\n/, $body) {
  next unless $ln =~ /;\s*$/;
  next if $ln =~ /reserved_slots/;
  if ($ln =~ /([A-Za-z_][A-Za-z0-9_]*)\s*;\s*$/) { push @fields, $1; }
}
# initializer entries, in order
my ($init) = $c =~ /const core_ds4_api_table_t g_core_ds4_shared_api_table[^=]*=\s*\{(.*?)\n\};/s or die "init not found";
my @entries;
for my $ln (split /\n/, $init) {
  $ln =~ s/^\s+//; $ln =~ s/\s+$//;
  next if $ln eq '' || $ln eq '{0}' || $ln eq '{0},';
  $ln =~ s/,$//;
  push @entries, $ln;
}
printf "cimpuri=%d  initializatori=%d\n", scalar(@fields), scalar(@entries);
my $n = @fields > @entries ? @fields : @entries;
my $bad = 0;
for my $i (0..$n-1) {
  my $f = $fields[$i] // '<LIPSA>';
  my $e = $entries[$i] // '<LIPSA>';
  my $b = $e; $b =~ s/^\(core_ds4_fn_[a-z0-9_]*\)//; $b =~ s/^core_contract_ds4_(api_|fw_)?//; $b =~ s/^&g_//; $b =~ s/^&//;
  my $fn = $f; $fn =~ s/_fn$//;
  next if $b eq $fn || index($b,$fn)==0 || index($fn,$b)==0;
  printf "  [%3d] cimp=%-38s init=%s\n", $i, $f, $e; $bad++;
}
print $bad ? "\n$bad pozitii de verificat manual\n" : "\nALINIERE PERFECTA\n";
