my $mode;
print "#include <idc.idc>\n";
print "static main()\n{\n";
open(FD, "system.map") or die "error open map file!";
while (<FD>)
{
    my $line = $_;

    if ($line =~ /\.text/)
    {
        $mode = 0;
    }
    if ($line =~ /(\.data|\.bss|\.ro?data)/i)
    {
        $mode = 1;
    }
    if ($line =~ /^\s+(0x\w+)\s+(\w+)$/)
    {
        if ($mode == 0)
        {
            my $ea = $1;
            my $name = $2;
            print "\tMakeCode($ea);\n";
            print "\tMakeName($ea,\"$name\");\n";
            print "\tMakeFunction($ea,BADADDR);\n";
        }
        else
        {
            my $ea = $1;
            my $name = $2;
            print "\tMakeName($ea,\"$name\");\n";
        }
    }
    if ($line =~ /^ (\.text|\.data|\.bss|\.ro?data)\s+(0x\w+)\s+(0x\w+)\s+(\w+)\.o$/i)
    {
        my $seg = $1;
        my $base = $2;
        my $len = $3;
        my $module = $4;
        next if $len eq "0x0";
        print "\n\tAddSeg($base, $base+$len, 0, 1, 0, 0);\n";
        print "\tRenameSeg($base, \"".lc($seg)."_$module\");\n\n";
    }
}
close(FD);
print "}\n";