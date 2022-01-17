#include "itkWin32Header.h"
#include <iostream>
#include <itkDirectory.h>
#include <string>
#include "itkImageFileReader.h"
#include "itkPhilipsXMLRECImageIO.h"
#include "itkMetaDataObject.h"
#include <itkExtractImageFilter.h>
#include <itkMeanImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>	// JOP 200812
#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryErodeImageFilter.h"
#include <itkMaskImageFilter.h>
#include "itkMinimumMaximumImageCalculator.h"

typedef itk::Image< float, 10 > ImageType;
typedef itk::Image< float, 3 > B1MagType;

typedef itk::PhilipsXMLRECImageIO PhilipsXMLRECImageIOType;
typedef itk::ImageFileReader< ImageType > ReaderType;
typedef itk::ExtractImageFilter< ImageType, B1MagType > ExtractImageFilterType;
typedef itk::BinaryThresholdImageFilter< B1MagType, B1MagType > ThresholdImageFilterType;
typedef itk::BinaryBallStructuringElement<float, 3  > StructuringElementType;
typedef itk::BinaryErodeImageFilter<B1MagType, B1MagType, StructuringElementType>BinaryErodeImageFilterType;
typedef itk::MaskImageFilter< B1MagType, B1MagType, B1MagType >  MaskImageFilterType;
typedef itk::MeanImageFilter< B1MagType, B1MagType >MeanImageFilterType;    // Smoothing filter

typedef itk::MinimumMaximumImageCalculator<B1MagType> minMaxFinderFilterType;

typedef PhilipsXMLRECImageIOType::ImageTypesTextType ImageTypesTextContainerType;
typedef PhilipsXMLRECImageIOType::ScanningSequencesTextType SequencesTypesTextContainerType;

int main(int argc, char **argv)
{
    if( argc < 5 )
    {
	std::cout << "Usage: " << std::endl;
	std::cout << argv[0] << " InputFile WantedFlipAngle ErodeRadius ShimFileLocation" << std::endl;
	return 1;
    }
    const char * inputFile   = argv[1];
    float wantedFlipAngle = atoi (argv[2] );
    const unsigned int radiusValue = atoi(argv[3]);
    const char * RFShimLocation   = argv[4];

    std::cout << "=== Loading input data ===" << std::endl;
    std::cout << "Currently processing " << inputFile << std::endl;
    PhilipsXMLRECImageIOType::Pointer imageIO = PhilipsXMLRECImageIOType::New();
    imageIO->SetFileName(inputFile);
    try
    {
	imageIO->CanReadFile( argv[1] );
    }
    catch( itk::ExceptionObject &err )
    {
        std::cerr << " : "  << err.GetDescription();
        return 1;
	char errorstring[] = "Could not open XML/REC files. Was the B1 image scanned? am_scale was not updated";
 	MessageBox( NULL, TEXT( errorstring ), TEXT( "PRIDE B1 Drive scale rescaling" ), MB_OK );
	std::cout << errorstring << std::endl;
	return 1;
    }
    ReaderType::Pointer baselineReader = ReaderType::New();
    baselineReader->SetFileName(inputFile);
    baselineReader->SetImageIO(imageIO);
    try
    {
	baselineReader->UpdateLargestPossibleRegion();
    }
    catch( itk::ExceptionObject &err )
    {
	std::cerr << "ExceptionObject caught here ";
	std::cerr << " : "  << err.GetDescription();
	char errorstring[] = "Error while reading XML/REC file. am_scale was not updated";
 	MessageBox( NULL, TEXT( errorstring ), TEXT( "PRIDE B1 Drive scale rescaling" ), MB_OK );
	std::cout << errorstring << std::endl;
	return 1;
    }

    // Load image types and sequence types
    ImageTypesTextContainerType ImageTypesText;
    if (!itk::ExposeMetaData<ImageTypesTextContainerType>(imageIO->GetMetaDataDictionary(), "PAR_ImageTypesText", ImageTypesText) )
    {
	char errorstring[] = "Could not get the image types Text vector. am_scale was not updated";
 	MessageBox( NULL, TEXT( errorstring ), TEXT( "PRIDE B1 Drive scale rescaling" ), MB_OK );
	std::cout << errorstring << std::endl;
	return 1;
    }
    SequencesTypesTextContainerType SequencesTypesText;
    if (!itk::ExposeMetaData<SequencesTypesTextContainerType>(imageIO->GetMetaDataDictionary(), "PAR_ScanningSequencesText", SequencesTypesText) )
    {
	char errorstring[] = "Could not get the sequence types text vector. am_scale was not updated";
 	MessageBox( NULL, TEXT( errorstring ), TEXT( "PRIDE B1 Drive scale rescaling" ), MB_OK );
	std::cout << errorstring << std::endl;
	return 1;
    }
    
    // We are looking for a Magnitude B1 map. First we find the M images. They get the index "0"
    int MagnitudeFound = 0;   
    int indexOfMagnitudeImages = -1;
    for (int m = 0; m < imageIO->GetDimensions(8); m++)	// First look for an M image
    {
	if  (!( strcmp(ImageTypesText.TextContainer[m], "M" ) ))
	{
	    ++ MagnitudeFound;
	    indexOfMagnitudeImages = m;
	    break;
	}
    }
    // Then we look for the B1 images
    int B1Found = 0;   
    int indexOfB1Images = -1;
    for (int n = 0; n < imageIO->GetDimensions(9); n++)	// First look for an M image
    {
	if  (!( strcmp(SequencesTypesText.TextContainer[n], "B1" ) ))
	{
	    ++ B1Found;
	    indexOfB1Images = n;
	    break;
	}
    }
    if ((MagnitudeFound != 1) && (B1Found != 1))
    {
	char errorstring[] = "Error: A B1 magnitude image type was not found. Image type is case sensitive.";
	MessageBox( NULL, TEXT( errorstring ), TEXT( "PRIDE B1 Drive scale rescaling" ), MB_OK );
	std::cout << errorstring << std::endl;
	return 1;
    }
    

    ////
    // Make ROI on the relevant data
    std::cout << "=== Look for max value in the B1 amplitude image ===" << std::endl;   
    ///////////////// 3. Extract data of interest /////////////////
    ExtractImageFilterType::InputImageRegionType extractionRegion;
    ExtractImageFilterType::InputImageSizeType desiredSize;
    desiredSize[0] = imageIO->GetDimensions(0);	// JOP If these are zero, then the matrix will be collapsed..
    desiredSize[1] = imageIO->GetDimensions(1);
    desiredSize[2] = imageIO->GetDimensions(2);
    desiredSize[3] = 0;
    desiredSize[4] = 0;
    desiredSize[5] = 0;
    desiredSize[6] = 0;
    desiredSize[7] = 0;
    desiredSize[8] = 0;
    desiredSize[9] = 0;
    extractionRegion.SetSize(desiredSize);

    ExtractImageFilterType::InputImageIndexType desiredRegion;
    desiredRegion[0] = 0;
    desiredRegion[1] = 0;
    desiredRegion[2] = 0;
    desiredRegion[3] = 0;
    desiredRegion[4] = 0;
    desiredRegion[5] = 0;
    desiredRegion[6] = 0;
    desiredRegion[7] = 0;
    desiredRegion[8] = indexOfMagnitudeImages;
    desiredRegion[9] = indexOfB1Images;

    extractionRegion.SetIndex(desiredRegion);

    ExtractImageFilterType::Pointer ExtractB1Mag = ExtractImageFilterType::New();
    ExtractB1Mag->SetDirectionCollapseToIdentity();
    ExtractB1Mag->SetInput(baselineReader->GetOutput());
    ExtractB1Mag->SetExtractionRegion(extractionRegion);

/// Smooth input
    MeanImageFilterType::Pointer smoothImage = MeanImageFilterType::New();
    smoothImage->SetInput(ExtractB1Mag->GetOutput());
    B1MagType::SizeType smoothingRadius;
    smoothingRadius[0] = 2;
    smoothingRadius[1] = 2;
    smoothingRadius[2] = 2;
    smoothImage->SetRadius(smoothingRadius);
    smoothImage->Update();

   ///////////////// 6. Erode mask to remove wierd edge stuff /////////////////
    ThresholdImageFilterType::Pointer mask = ThresholdImageFilterType::New();
    mask->SetInput(smoothImage->GetOutput());
    mask->SetOutsideValue(0);
    mask->SetInsideValue(1);
    mask->SetLowerThreshold(3);	// Assuming everything below 10% is noise
    
    StructuringElementType structuringElement;
    structuringElement.SetRadius( radiusValue );  // 3x3 structuring element
    structuringElement.CreateStructuringElement();

    BinaryErodeImageFilterType::Pointer erodedMask = BinaryErodeImageFilterType::New();
    erodedMask->SetInput(mask->GetOutput());
    erodedMask->SetKernel(structuringElement);
    erodedMask->SetForegroundValue(1); // Intensity value to erode
    erodedMask->SetBackgroundValue(0);   // Replacement value for eroded voxels

    MaskImageFilterType::Pointer maskedImage = MaskImageFilterType::New();
    maskedImage->SetInput1(smoothImage->GetOutput());
    maskedImage->SetInput2(erodedMask->GetOutput());
    maskedImage->Update();

    ///////////////// 9. Find wanted value /////////////////
    minMaxFinderFilterType::Pointer findMinMax = minMaxFinderFilterType::New();
    findMinMax->SetImage(maskedImage->GetOutput());
    findMinMax->Compute();
    std::cout << "Maximum intensity found: " << findMinMax->GetMaximum() << std::endl;
    //std::cout << "Mimimum intensity found: " << findMinMax->GetMinimum() << std::endl;

    float driveScaleRF = 0.0;
    if (!itk::ExposeMetaData<float>(imageIO->GetMetaDataDictionary(), "PAR_DiffusionEchoTime", driveScaleRF) ) 
    {
	std::cout << "Could not get the RF drive scale" << std::endl;
	return 1;
    }

    if ( driveScaleRF == 0 )
    {
	char errorstring[] = "Drive scale found in XML/REC file is 0. Was drive scale saved in the ExamCard? am_scale was not updated";
 	MessageBox( NULL, TEXT( errorstring ), TEXT( "PRIDE B1 Drive scale rescaling" ), MB_OK );
	std::cout << errorstring << std::endl;
	return 1;
    }

 // Overwrite am scale in rfshim.dat
    float foundB1scale = ( wantedFlipAngle / findMinMax->GetMaximum() );
    std::cout << "Scaling by a factor " << foundB1scale << " to get flip angle of " << wantedFlipAngle << "%" << std::endl;

    const char *RFShimFileName = RFShimLocation;
    FILE *pRFShimFile = fopen( RFShimFileName, "r+");	
    if ( pRFShimFile!= NULL )
    {
	int version = 0;
	int nr_channels = 0;
	float am_scale[8];
	float new_am_scale;
	//for (int n = 0; n < 8; n++)
	//{
	//    am_scales[n] = 0.0;
	//}
	fscanf( pRFShimFile, "%d", &version );		// Read the version
	if (version == 4 || version == 5 )
        {
            fscanf( pRFShimFile, "%d", &nr_channels );	// Read the number of channels
            for ( int c = 0; c < nr_channels; c++ )
            {

                fscanf( pRFShimFile, "%8f", &am_scale[c] );
		std::cout << "Found am_scale: " << am_scale[c] << " for channel " << c << std::endl;

	    }
	    fseek(pRFShimFile, 0, SEEK_SET);
	    fscanf( pRFShimFile, "%d", &version );		// Read the version
	    fscanf( pRFShimFile, "%d", &nr_channels );	// Read the number of channels
	    fseek ( pRFShimFile , 2, SEEK_CUR );	// Jump the space and line shift bits after the number of coils
	    new_am_scale = ( driveScaleRF * foundB1scale );
	    if ( new_am_scale < 0.00001 )
	    {
		char errorstring[] = "Error: The original am_scale has an unsupported size (likely negative or above 10). No changes was made to am_scale";
 		//MessageBox( NULL, TEXT( errorstring ), TEXT( "PRIDE B1 Drive scale rescaling" ), MB_OK );
		std::cout << errorstring << std::endl;
		fclose ( pRFShimFile );
		return 1;
	    }
	    else if ( foundB1scale > 1.20 )
	    {	
		//MessageBox( NULL, TEXT( "The found scaling is abnormally high, and likely not safe. The am_scale was not updated, but can be done manually" ), TEXT( "PRIDE B1 Drive scale rescaling" ), MB_OK );
		char errorstring[] = "The found am_scaling is abnormally high, and likely not safe. The am_scale was not updated, but can be done manually (the needed drive scale is written in the PRIDE log file)";
 		MessageBox( NULL, TEXT( errorstring ), TEXT( "PRIDE B1 Drive scale rescaling" ), MB_OK );
		std::cout << errorstring << std::endl;
		std::cout << "Use drive scales" << std::endl;
		for (int n = 0; n < nr_channels; n++)
		{
		    std::cout << " " << new_am_scale;
		}
		std::cout << std::endl;
		//system("pause");
		return 1;
	    }
	    else
	    {
		std::cout << "Writing new drive scale for all channels" << std::endl;
		for (int n = 0; n < nr_channels; n++)
		{		    
		    fprintf(pRFShimFile,"%6f ", new_am_scale );	// Overwrite with rescaled am scale
		    std::cout << "Wrote " << new_am_scale << " for channel " << n << std::endl;
		}
	    }
	}
	

	else
	{ 
	    char errorstring[] = "Error: The rfshim file was not read-in correctly. This is likely related to the first value (version number). am_scale was not updated";
 	    MessageBox( NULL, TEXT( errorstring ), TEXT( "PRIDE B1 Drive scale rescaling" ), MB_OK );
	    std::cout << errorstring << std::endl;
	    fclose ( pRFShimFile );
	    return 1;
	}

    }
    fclose ( pRFShimFile );
    std::cout << std::endl << "  Finished running PRDE-tool exe-file correctly" << std::endl;
return 0;
}