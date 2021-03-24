use strict;
use Getopt::Long;

 my $in_ref;
 my $out_ref = "ref.out";
 my $out_hyp = "hyp.out";
 my $ins_fract = 0.1;
 my $del_fract = 0.1;
 my $sub_fract = 0.1;
 my $ref_length = 1000;

my $voc_length = 10000;

my $rc = GetOptions(
    "iref=s" =>\$in_ref,
    "oref=s" =>\$out_ref,
    "ohyp=s" =>\$out_hyp,
    "ins_fract=f" => \$ins_fract,
    "del_fract=f" => \$del_fract,
    "sub_fract=f" => \$sub_fract,
    "ref_length=i" => \$ref_length,
    "voc_length=i" => \$voc_length,
);

die "check your commandline!\n" if(!$rc);

# just making things slightly easier
our %words;
our $word_mass =0;
my @ref_words;
my @hyp_words;

if(!defined($in_ref)){
    for(my $i = 0; $i < $voc_length; $i++)
    {
        my $w = sprintf("w%06d", $i);
        # fix to get a non-uniform distribution
        my $mass = 1;
        $words{$w} = $mass;
        $word_mass += $mass;
    }

    for(my $i =0; $i < $ref_length; $i++){
        my $w = select_word();
        push(@ref_words, $w);
    }
} else {
    open(FF, "<$in_ref") || die "couldn't open [$in_ref] for reading!";
    while(my $l = <FF>){
        chomp($l);
        $l=~s/\s*$//;
        my @wds = split(/\s+/, $l);
        foreach (@wds){
            $words{$_}++;
            $word_mass += 1;
            push(@ref_words, $_);
        }
    }

    $ref_length = scalar(@ref_words);
}

my $num_ins = 0;
my $num_del = 0;
my $num_sub = 0;
my $i = 0;
my $last_was_ins = 0;
my $last_was_del = 0;

my $ins_thres = $ins_fract;
my $del_thres = $ins_thres + $del_fract;
my $sub_thres = $del_thres + $sub_fract;

while($i < $ref_length)
{
    my $r = rand();
    my $rw = $ref_words[$i];

    if($r <= $ins_thres)
    {
        if($last_was_del){
            #let's not insert after a deletion, this looks like 
            #a substitution
            next;
        }

        my $ins_w = select_word();
        push(@hyp_words, $ins_w);
        $num_ins++;
        $last_was_ins = 1;
        $last_was_del = 0;
        # we don't increment $i
    } elsif($r < $del_thres){
        if($last_was_ins){
            #let's not delete after an insertion, this looks like 
            #a substitution
            next;
        }

        $num_del++;
        $i++;
        $last_was_del = 1;
        $last_was_ins = 0;
    } elsif($r < $sub_thres){
        my $sub_w = select_word();
        while($sub_w eq $rw)
        {
            $sub_w = select_word();
        }

        $num_sub++;
        push(@hyp_words, $sub_w);
        $i++;
        $last_was_del = 0;
        $last_was_ins = 0;
    } else {
        $i++;
        # phew...  a correct word...
        push(@hyp_words, $rw);
        $last_was_del = 0;
        $last_was_ins = 0;
    }
}

if(defined($out_ref)){
    dump_words($out_ref, \@ref_words);
}

if(defined($out_hyp)){
    dump_words($out_hyp, \@hyp_words);
}

print "$num_ins INS\n";
print "$num_del DEL\n";
print "$num_sub SUB\n";
printf "expected WER %.3f\n", ($num_ins + $num_del + $num_sub) / $ref_length;

sub dump_words{
    my ($ofn, $aref) = @_;

    print "writing to [$ofn]\n";
    open(OUT, ">$ofn") || die "couldn't open [$ofn] for writing!";
    foreach (@$aref){
        print OUT $_," ";
    }
    print OUT "\n";
    close(OUT);

}




sub select_word {
    my $r = int(rand($word_mass));
    my $w;

    my $cur_sum = 0;
    while ( my ( $key, $value ) = each %words ) {
        $w = $key;
        $cur_sum += $value;
        if($r <= $cur_sum){
            last;
        }
    }

    return $w;
}