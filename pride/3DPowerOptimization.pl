
#!/perl/bin -w

use strict;
use p_apup_general_subroutines;
use Win32;

{
### Defining inputs to the PRIDE exe file 
my $EXEinput = "G:/Patch/pride/tempinputseries/DBIEX.REC"; 
my $WantedFlipAngle = 140; 
my $ErodeRadius = 5; 
my $ShimFileLocation = "G:/site/rfshims.dat"; 

my $scriptLogFile = "G:/Patch/pride/3DPowerOptimization/3DPowerOptimization.log";	
my $perlLogFile = "G:/Patch/pride/3DPowerOptimization/perl.log";
my $inputpath =  "G:/Patch/pride/tempinputseries";

my $toolpath = "G:/Patch/pride/3DPowerOptimization/3DPowerOptimization";
my $timestamp = localtime(time);
my $command;		 

## Write to Perl and EXE log file.
open (LOGFILE, ">> $perlLogFile");
print LOGFILE "\n\n$timestamp\n";
print LOGFILE "In-line PRIDE tool started\n";


## Delete content of data input folder before running EXE file
my $errors;
while ($_ = glob($inputpath."/*")) {
   next if -d $_;
   unlink($_)
      or ++$errors, warn("Can't remove $_: $!");
}
exit(1) if $errors;

my $roid = $ARGV[0];
my $dirExe = "\"C:/Program Files (x86)/PMS/Mipnet91x64\"";	
my $leacherExe = "$dirExe/pridexmlleacher_win_cs.exe";	# Check whether this file exists in the folder defined by "dirExe

# Extracting the Xml/Rec file from the database. This will always use the same filename
print LOGFILE "Extracting the XML/REC file from the database\n";
my $command;
$command = "$leacherExe $roid";
print LOGFILE ">>> $command\n" ;
	system($command);
	

## Starting the PRIDE tool here.
print LOGFILE  "Excecuting PRIDE exe. Running may take a bit of time... \n";
$command = "$toolpath.exe $EXEinput $WantedFlipAngle $ErodeRadius $ShimFileLocation > $scriptLogFile ";
system($command);

if ($? == -1) {
    print  "failed to execute: $! \n ";
		Win32::MsgBox( "PRIDE tool failed to excecute. Images was not generated ");
}
elsif ($? & 127) {
    printf  "child died with signal %d, %s coredump \n ",
    ($? & 127),  ($? & 128) ? 'with' : 'without';
		Win32::MsgBox( "Error in PRIDE tool. Check log file for further information. ");
}
elsif (($? >> 8) == 1) {
    print LOGFILE  "Return code 1  \n ",
		Win32::MsgBox( "Error in PRIDE tool. Check log file for further information. ");
}
else {
    print LOGFILE  "PRIDE tool ran as intended \n ";
	print LOGFILE  "###################################################  \n ";
}

## Delete content of tempinputseries after running EXE file
my $errors;
while ($_ = glob($inputpath."/*")) {
   next if -d $_;
   unlink($_)
      or ++$errors, warn("Can't remove $_: $!");
}
exit(1) if $errors;

print "\n Finished Perl script.\n";
}