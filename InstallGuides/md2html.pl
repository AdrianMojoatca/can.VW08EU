#!/usr/bin/perl
# ---------------------------------------------------------------------------
# md2html.pl - converteste ghidul generat (Markdown) in HTML de sine statator.
#
# HTML-ul se deschide in orice browser (dublu-click), se tipareste direct in
# PDF din browser si poate fi deschis si de Word pentru a fi salvat .docx.
#
# Utilizare:  perl md2html.pl <fisier.md> [> fisier.html]
#
# Recunoaste exact subsetul de Markdown produs de gen_install_guide.pl:
# titluri #/##/###, tabele, citate >, liste -, **bold**, `cod`.
# ---------------------------------------------------------------------------
use strict;
use warnings;
use MIME::Base64 qw(encode_base64);
use File::Basename qw(dirname);

my $src = shift or die "utilizare: perl $0 <fisier.md>\n";
open(my $fh, '<', $src) or die "nu pot deschide $src: $!\n";
my @lines = <$fh>;
close $fh;
my $src_dir = dirname($src);
chomp @lines;

sub esc {
    my $s = shift // '';
    $s =~ s/&/&amp;/g; $s =~ s/</&lt;/g; $s =~ s/>/&gt;/g;
    return $s;
}

sub inline {
    my $s = esc(shift);
    $s =~ s/\*\*(.+?)\*\*/<strong>$1<\/strong>/g;
    $s =~ s/`(.+?)`/<code>$1<\/code>/g;
    $s =~ s/_([^_]+)_/<em>$1<\/em>/g;
    $s =~ s/\\\|/|/g;
    return $s;
}

sub split_row {                       # imparte un rand de tabel, respectand \|
    my $r = shift;
    $r =~ s/^\s*\|//; $r =~ s/\|\s*$//;
    my @out; my $cell = '';
    my @ch = split //, $r;
    for (my $i = 0; $i < @ch; $i++) {
        if ($ch[$i] eq '\\' && $i + 1 < @ch && $ch[$i+1] eq '|') { $cell .= '\|'; $i++; }
        elsif ($ch[$i] eq '|') { push @out, $cell; $cell = ''; }
        else { $cell .= $ch[$i]; }
    }
    push @out, $cell;
    s/^\s+|\s+$//g for @out;
    return @out;
}

my $title = 'Install Guide';
for (@lines) { if (/^#\s+(.*)/) { $title = $1; last } }

my @html;
my ($in_table, $in_list) = (0, 0);

sub close_blocks {
    if ($in_table) { push @html, '</tbody></table>'; $in_table = 0 }
    if ($in_list)  { push @html, '</ul>';            $in_list  = 0 }
}

for (my $i = 0; $i < @lines; $i++) {
    my $l = $lines[$i];

    next if $l =~ /^<!--/;                                  # comentariile de generare

    if ($l =~ /^\s*$/)            { close_blocks(); next }
    if ($l =~ /^---+\s*$/)        { close_blocks(); push @html, '<hr>'; next }

    if ($l =~ /^(#{1,4})\s+(.*)/) {
        close_blocks();
        my $n = length $1;
        push @html, "<h$n>" . inline($2) . "</h$n>";
        next;
    }

    if ($l =~ /^>\s?(.*)/) {
        close_blocks();
        my @q = (inline($1));
        while ($i + 1 < @lines && $lines[$i+1] =~ /^>\s?(.*)/) { push @q, inline($1); $i++ }
        push @html, '<blockquote>' . join('<br>', @q) . '</blockquote>';
        next;
    }

    if ($l =~ /^-\s+(.*)/) {
        if ($in_table) { push @html, '</tbody></table>'; $in_table = 0 }
        push @html, '<ul>' unless $in_list;
        $in_list = 1;
        push @html, '<li>' . inline($1) . '</li>';
        next;
    }

    if ($l =~ /^\s*\|/) {
        if ($l =~ /^\s*\|[\s:|-]+\|\s*$/) { next }          # randul de separare
        my @c = split_row($l);
        if (!$in_table) {
            if ($in_list) { push @html, '</ul>'; $in_list = 0 }
            push @html, '<table><thead><tr>' .
                join('', map { '<th>' . inline($_) . '</th>' } @c) .
                '</tr></thead><tbody>';
            $in_table = 1;
        } else {
            my $cls = join '', map {
                my $v = $_;
                '<td' . ($v eq 'No Connection' ? ' class="nc"' : '') . '>' . inline($v) . '</td>'
            } @c;
            push @html, "<tr>$cls</tr>";
        }
        next;
    }

    # imagine: se inglobeaza ca data: URI, ca HTML-ul sa ramina un singur fisier
    if ($l =~ /^!\[([^\]]*)\]\(([^)]+)\)/) {
        close_blocks();
        my ($alt, $ref) = ($1, $2);
        my $path = ($ref =~ m{^[A-Za-z]:|^/}) ? $ref : "$src_dir/$ref";
        if (open(my $img, '<:raw', $path)) {
            local $/; my $bin = <$img>; close $img;
            if ($ref =~ /\.svg$/i) {
                # SVG se pune ca atare: ramine vectorial si e mult mai mic decit base64
                $bin =~ s/^\s*<\?xml.*?\?>\s*//s;
                $bin =~ s/^\s*<!DOCTYPE[^>]*>\s*//s;
                push @html, qq{<figure>$bin<figcaption>} . inline($alt) . qq{</figcaption></figure>};
            } else {
                my $mime = ($ref =~ /\.jpe?g$/i) ? 'image/jpeg' : 'image/png';
                my $b64 = encode_base64($bin, '');
                push @html, qq{<figure><img src="data:$mime;base64,$b64" alt="} . esc($alt) .
                            qq{"><figcaption>} . inline($alt) . qq{</figcaption></figure>};
            }
        } else {
            push @html, '<p><em>[imagine lipsa: ' . esc($ref) . ']</em></p>';
        }
        next;
    }

    close_blocks();
    push @html, '<p>' . inline($l) . '</p>';
}
close_blocks();

print <<"HEAD";
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>@{[ esc($title) ]}</title>
<style>
  body   { font-family: Segoe UI, Arial, sans-serif; font-size: 11pt; color: #1a1a1a;
           max-width: 1100px; margin: 24px auto; padding: 0 20px; line-height: 1.45; }
  h1     { font-size: 22pt; margin: 0 0 4px; border-bottom: 3px solid #0b5394; padding-bottom: 6px; }
  h2     { font-size: 15pt; margin: 28px 0 8px; color: #0b5394; }
  h3     { font-size: 12pt; margin: 20px 0 6px; background: #eef3f9;
           padding: 6px 10px; border-left: 4px solid #0b5394; }
  table  { border-collapse: collapse; width: 100%; margin: 8px 0 16px; font-size: 9.5pt; }
  th     { background: #0b5394; color: #fff; text-align: left; padding: 6px 8px; font-weight: 600; }
  td     { border: 1px solid #c9d4e0; padding: 5px 8px; vertical-align: top; }
  tr:nth-child(even) td { background: #f7f9fc; }
  td.nc  { color: #8a8a8a; }
  code   { background: #f0f0f0; padding: 1px 4px; border-radius: 3px;
           font-family: Consolas, monospace; font-size: 9.5pt; }
  blockquote { margin: 8px 0; padding: 8px 12px; background: #fff8e1;
               border-left: 4px solid #e0a800; }
  figure { margin: 12px 0 20px; }
  figure img { width: 100%; border: 1px solid #c9d4e0; }
  figcaption { font-size: 9pt; color: #666; margin-top: 4px; font-style: italic; }
  hr     { border: 0; border-top: 1px solid #ccc; margin: 24px 0; }
  \@media print { body { margin: 0; max-width: none; } h3 { break-after: avoid; }
                  table { break-inside: auto; } tr { break-inside: avoid; } }
</style>
</head>
<body>
HEAD

print "$_\n" for @html;
print "</body>\n</html>\n";
