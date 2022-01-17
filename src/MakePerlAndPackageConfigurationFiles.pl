#!/perl/bin -w

use strict;
use warnings; 
use Win32;

my $TOOL_NAME = "3DPowerOptimization";	# Avoid spaces if you want it easy with citation marks
my $DELETE_DATA_INPUT_FOLDER_BEFORE_RUNNING = 1;
my $DELETE_DATA_INPUT_FOLDER_AFTER_RUNNING = 1;
my $DELETE_DATA_OUTPUT_FOLDER_BEFORE_RUNNING = 0;
my $START_EXE_FILE = 1;
my $LEACH_XML_FILE = 1;
my $WRITE_NEW_XML_FILE = 0;
my $PRIDE_FOLDER_LOCATION = 'G:/Patch/pride';		# Be sure to have 4 times "\" for separating folders. Not pretty, but it works..
my $DATA_INPUT_PATH = "$PRIDE_FOLDER_LOCATION/tempinputseries";
my $DATA_OUTPUT_PATH = "$PRIDE_FOLDER_LOCATION/tempoutputseries";
# my $DATA_OUTPUT_PATH = "imgout/$TOOL_NAME\.rec";
my $NUMBER_OF_EXE_INPUTS = 4;
my @EXE_INPUTS_NAMES = ("EXEinput","WantedFlipAngle", "ErodeRadius","ShimFileLocation" );
my @EXE_INPUTS_VALUES = ("\"$DATA_INPUT_PATH/DBIEX.REC\"","140", "5",'"G:/site/rfshims.dat"' );


## make basic sanity checks ##
if ($START_EXE_FILE)
{
	if( $NUMBER_OF_EXE_INPUTS != @EXE_INPUTS_NAMES)
	{
		Win32::MsgBox("Error. NUMBER_OF_EXE_INPUTS does not match the length of EXE_INPUTS_NAMES");
		exit(1);
	}
	if( $NUMBER_OF_EXE_INPUTS != @EXE_INPUTS_VALUES)
	{
		Win32::MsgBox("Error. NUMBER_OF_EXE_INPUTS does not match the length of EXE_INPUTS_VALUES");
		exit(1);	
	}
}
###############################################################################################################
### Write the command line expression we want PRIDE to run when the PRIDE tool is started (XML file in packageconfiguration     ###
###############################################################################################################
my $packageConfigurationFile = "G:\\Patch\\pride\\packageconfiguration\\$TOOL_NAME.xml";
# Use the open() function to create the file.
unless(open CONFIG_FILE, '>'.$packageConfigurationFile) {
    # Die with error message 
    # if we can't open it.
    die "\nUnable to create $packageConfigurationFile\n";
}


print CONFIG_FILE "<?xml version=\"1.0\" encoding=\"utf-8\"?>
<PrideConfiguration xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">
<PackageName>$TOOL_NAME</PackageName>
<CommandLine>\"g:\\patch\\pride\\XML2input\\$TOOL_NAME.pl\"</CommandLine>
<CommandArguments></CommandArguments>
<CommandArgumentsVisible>false</CommandArgumentsVisible>
<ShowUserInterface>true</ShowUserInterface>
</PrideConfiguration> ";

# close the file.
close CONFIG_FILE;

##########################################################################################
### Populate Perl script which we use to start logging and to clean up in tempinput series if needed. #######
##########################################################################################

my $directory = "$PRIDE_FOLDER_LOCATION/$TOOL_NAME";
    
	if (-d "$directory") {
    print "Directtory already exisits. Continuing...\n";
}
else
{
    print "Creating a new directory...\n";
    unless(mkdir $directory) {
        die "Unable to create $directory\n";
    }
}
	
my $perlExecutionFile = "$PRIDE_FOLDER_LOCATION\\$TOOL_NAME\\$TOOL_NAME.pl";
# Use the open() function to create the file.
unless(open PERL_FILE, '>'.$perlExecutionFile) {
    # Die with error message 
    # if we can't open it.
    die "\nUnable to create $perlExecutionFile\n";
}
# Writing "Mandatory" content to PRIDE perl script
print PERL_FILE "
#!/perl/bin -w

use strict;
use p_apup_general_subroutines;
use Win32;

{
";

# Write EXE input variables to PRIDE Perl script
if( $START_EXE_FILE)
{
	print PERL_FILE "### Defining inputs to the PRIDE exe file \n";
	foreach my $i( 0 .. $NUMBER_OF_EXE_INPUTS-1 ) 
	{
		print PERL_FILE  'my $'."$EXE_INPUTS_NAMES[$i] = $EXE_INPUTS_VALUES[$i]\; \n";
	}
	print PERL_FILE "\nmy \$scriptLogFile = \"$PRIDE_FOLDER_LOCATION/$TOOL_NAME/$TOOL_NAME.log\";	";
}

# Writing "Mandatory" content to PRIDE perl script
print PERL_FILE "
my \$perlLogFile = \"$PRIDE_FOLDER_LOCATION/$TOOL_NAME/perl.log\";
";
if ($LEACH_XML_FILE)
{
	print PERL_FILE "my \$inputpath =  \"$DATA_INPUT_PATH\";\n";
}
if ($WRITE_NEW_XML_FILE)
{
	print PERL_FILE "my \$outputpath =  \"$DATA_OUTPUT_PATH\";\n";
}
print PERL_FILE "
my \$toolpath = \"$PRIDE_FOLDER_LOCATION/$TOOL_NAME/$TOOL_NAME\";
my \$timestamp = localtime(time);

## Write to Perl and EXE log file.
open (LOGFILE, \">> \$perlLogFile\");
print LOGFILE \"\\n\\n\$timestamp\\n\";
print LOGFILE \"In-line PRIDE tool started\\n\";

";

if( $DELETE_DATA_INPUT_FOLDER_BEFORE_RUNNING)
{
print PERL_FILE '
## Delete content of data input folder before running EXE file
my $errors;
while ($_ = glob($inputpath."/*")) {
   next if -d $_;
   unlink($_)
      or ++$errors, warn("Can\'t remove $_: $!");
}
exit(1) if $errors;
';
}

if( $DELETE_DATA_OUTPUT_FOLDER_BEFORE_RUNNING)
{
print PERL_FILE '
## Delete content of data output folder content before running EXE file
my $errors;
while ($_ = glob($outputpath."/*")) {
   next if -d $_;
   unlink($_)
      or ++$errors, warn("Can\'t remove $_: $!");
}
exit(1) if $errors;
';
}


if( $LEACH_XML_FILE)
{
print PERL_FILE '
my $roid = $ARGV[0];
my $dirExe = "\"C:/Program Files (x86)/PMS/Mipnet91x64\"";	
my $leacherExe = "$dirExe/pridexmlleacher_win_cs.exe";	# Check whether this file exists in the folder defined by "dirExe

# Extracting the Xml/Rec file from the database. This will always use the same filename
print LOGFILE "Extracting the XML/REC file from the database\n";
my $command;
$command = "$leacherExe $roid";
print LOGFILE ">>> $command\n" ;
	system($command);
	
';
}

if( $START_EXE_FILE)
{
print PERL_FILE '
## Starting the PRIDE tool here.
print LOGFILE  "Excecuting PRIDE exe. Running may take a bit of time... \\n";
$command = ';
### Construct the command line call that is used when calling the PRIDE tool. This should contain all the variables defined in EXE_INPUTS_NAMES
my $commandForExecutingEXE = "\"\$toolpath\.exe";
foreach my $element (@EXE_INPUTS_NAMES) {
	$commandForExecutingEXE = $commandForExecutingEXE." \$" ."$element";
}
$commandForExecutingEXE = $commandForExecutingEXE." > \$scriptLogFile \"\;";
## Now write the expression to the PRIDE-tool Perl file.
print PERL_FILE "$commandForExecutingEXE";

print PERL_FILE '
system($command);

if ($? == -1) {
    print  "failed to execute: $! \\n ";
		Win32::MsgBox( "PRIDE tool failed to excecute. Images was not generated ");
}
elsif ($? & 127) {
    printf  "child died with signal %d, %s coredump \\n ",
    ($? & 127),  ($? & 128) ? \'with\' : \'without\';
		Win32::MsgBox( "Error in PRIDE tool. Check log file for further information. ");
}
elsif (($? >> 8) == 1) {
    print LOGFILE  "Return code 1  \\n ",
		Win32::MsgBox( "Error in PRIDE tool. Check log file for further information. ");
}
else {
    print LOGFILE  "PRIDE tool ran as intended \\n ";
	print LOGFILE  "###################################################  \\n ";
}
';
}

if( $DELETE_DATA_INPUT_FOLDER_AFTER_RUNNING)
{
print PERL_FILE '
## Delete content of tempinputseries after running EXE file
my $errors;
while ($_ = glob($inputpath."/*")) {
   next if -d $_;
   unlink($_)
      or ++$errors, warn("Can\'t remove $_: $!");
}
exit(1) if $errors;
';
}
print PERL_FILE '
print "\n Finished Perl script.\n";
}';

# close the file.
close CONFIG_FILE;