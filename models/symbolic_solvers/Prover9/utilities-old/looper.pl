#!/usr/bin/perl -w

$| = 1;				# flush output

$scriptname = "looper";
if ($#ARGV != 2 ||
    !($ARGV[0] eq "prover9" || $ARGV[0] eq "mace4") ||
    !($ARGV[1] eq "assumptions" || $ARGV[1] eq "goals") ) {
    print STDERR "$scriptname (prover9|mace4) (assumptions|goals) head < candidates\n";
    die;
}

$program = "$ARGV[0]";
$list    = "$ARGV[1]";

open(FIN,  $ARGV[2]) || die "head file not found";

$host=`hostname`; chop($host);
$date=`date`; chop($date);
print "Started $scriptname $date on $host.\n";

@head  = <FIN>;   # read the whole head file

print "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n";
print @head;
print "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n";

$i = 0;           # problem counter
$input = "in$$";

while ($problem = <STDIN>) {
    $i++;
    chop $problem;
    print "-----------------------------------------------------\n";
    print "$problem  % Problem $i\n\n";

    open(FH, ">$input") || die "Cannot open file $input";

    print FH @head;
    print FH "formulas($list).\n";
    print FH "$problem\n";
    print FH "end_of_list.\n";

    close(FH);

    $command =
	"$program < $input 2> /dev/null | " .
	"awk " .
	    "'/(Fatal|Given|Selections|CPU|Process.*exit)/ {print} " .
	    "/= PROOF =/,/= end of proof =/ {print} " .
	    "/= MODEL =/,/= end of model =/ {print}'";

    @out = `$command`;

    if (grep(/Fatal/, @out)) {
	print STDERR "\nFatal error in $program, see output.\n\n"; 
	system("/bin/rm $input");
	die; 
    }

    ($timeline) = grep(/User_CPU/, @out);
    ($_,$sec) = split(/[=,]/, $timeline);

    $id = $problem;
    if ($problem =~ /label/) {
	$id =~ s/.*label/label/;
    }

    print @out;

    if (grep(/max_proofs/, @out))
    { print "\n$id Proved $sec seconds PROBLEM $i\n"; }
    elsif (grep(/max_models/, @out))
    { print "\n$id Disproved $sec seconds PROBLEM $i\n"; }
    else
    { print "\n$id Failed $sec seconds PROBLEM $i\n"; }
}

system("/bin/rm $input");

$date=`date`; chop($date);

print "\nFinished $scriptname $date on $host.\n";
