/*==============================================================================

  Library: MSVTK

  Copyright (c) Kitware Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// MSVTK
#include "msvVTKCompositeFileSeriesReader.h"

// VTK includes
#include "vtkXMLMultiBlockDataReader.h"
#include "vtkNew.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPolyDataReader.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStringArray.h"
#include "vtkTestUtilities.h"
#include "vtkXMLReader.h"

// STD includes
#include <cstdlib>
#include <iostream>
#include <string>

// -----------------------------------------------------------------------------
int msvVTKCompositeFileSeriesReaderTest1(int argc, char* argv[])
{
  // Get the data test files
  const char* file0 =
    vtkTestUtilities::ExpandDataFileName(argc,argv,"compositeMetaData.xml");

  // Create the fileSeriesReader
  vtkNew<vtkXMLMultiBlockDataReader> xmlReader;
  vtkNew<msvVTKCompositeFileSeriesReader> compositeSeriesReader;
  compositeSeriesReader->SetReader(xmlReader.GetPointer());

  std::cout << "Can reader read the file: " << xmlReader->CanReadFile(file0) << std::endl;

 /* if (!compositeSeriesReader->CanReadFile(file0))
    {
    std::cerr << "Error: Cannot read the file: " << file0
              << std::endl;
    return EXIT_FAILURE;
    }*/

  compositeSeriesReader->AddFileName(file0);
  compositeSeriesReader->CreateFileSeriesReaders();

  std::cout << "Number of file from the MetaData: " << compositeSeriesReader->GetNumberOfFileNames() << std::endl;
  std::cout << "FileName: " << compositeSeriesReader->GetFileName(0) << std::endl;

  return EXIT_SUCCESS;
}
