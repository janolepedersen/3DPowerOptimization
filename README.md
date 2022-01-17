# 3DPowerOptimization
PRIDE tool for updating PO-prep-phase's drive scale. 

# Introduction
	PRIDE tool for updating the scanner's found drive-scale based on an input B1map (only for Classic). Tested and developed for 2ch-NOVA head coil using DREAM B1 maps as implemented in the 7T software "5.1.7 rev C".
	
	The scanner's preparation phase "PO" calculates a drivescale, that determines the relationship between RF-amplifier output and generated flip-angle. This is based on a single axial slice at iso-center, which may miss the B1 hotspot for large heads or if the isocenter-reference (the green dot on the patient table), is set incorrectly. This may result in e.g. increased "black-whole artefacts" for FLAIR images (ISMRM 2021, #3554), and bias in MPRAGE (ISMRM 2021, #1416). This PRIDE tool acts as a secondary step, that scales the PO-found drive scale based on a 3D B1map. The scaling is done such that a global max-B1 is equal to a pre-defined value (default: 140%).

	The tool consists of a perl-script that handles pulling of data from the database and an .exe file that performs the actual calculations. Individual logging is done for the perl and exe-files. The .exe-file performs slight smoothing and image erosion and determines a global maximum. Based on this value it updates rfshim.dat, which the scanner uses for scaling of B1 as long as "Contrast-> RF Shim" is set to "Shimtool".
	
	For questions, feedback and bug reports: jan.pedersen@philips.com
	

# Usage
	The tool should be applied on a B1map that is acquired before the scan(s) of interest. Since the SENSE-ref-scan is B1-dependent, ideally the B1map is runned before the SENSE ref-scan also. The DREAM B1map can however not use the Tx coil (without patching), and it is therefore necessary to have a SENSE-ref scan before the B1 map also. See also example ExamCard for reference.
	
	The control parameter "PO overrules RF shim" should be set to "No". To activate the use of updated drive scale for a given scan, set "Contrast-> RF Shim" to "shimtool". 
  
  Due to wierd behavior when exporting XML/REC B1maps, the in-plane voxelsize and FOV needs to be isometric (i.e. same RO and phase FOV size and voxelsize).
