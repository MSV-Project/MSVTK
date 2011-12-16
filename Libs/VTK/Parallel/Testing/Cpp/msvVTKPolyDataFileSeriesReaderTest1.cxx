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
#include "msvVTKPolyDataFileSeriesReader.h"

// VTK includes
#include "vtkNew.h"
#include "vtkTestUtilities.h"

// STD includes
#include <cstdlib>
#include <string>

// -----------------------------------------------------------------------------
int msvVTKPolyDataFileSeriesReaderTest1(int argc, char* argv[])
{
  // Get the data test files
  const char* file0 =
    vtkTestUtilities::ExpandDataFileName(argc,argv,"Polydata00.vtk");
  const char* file1 =
    vtkTestUtilities::ExpandDataFileName(argc,argv,"Polydata00.vtk");

  // Create the fileSeriesReader
  vtkNew<vtkPolyDataReader> polyDataReader;
  vtkNew<msvVTKPolyDataFileSeriesReader> polyDataFileSeriesReader;

  if (polyDataFileSeriesReader->CanReadFile(file0))
    {
    std::cerr << "Error: method CanReadFile must return 0 when no reader set."
              << std::endl;
    return EXIT_FAILURE;
    }
  if (polyDataFileSeriesReader->CanReadFile(polyDataReader.GetPointer(),0))
    {
    std::cerr << "Error: method CanReadFile must return 0 when no file set."
              << std::endl;
    return EXIT_FAILURE;
    }

  polyDataFileSeriesReader->SetReader(polyDataReader.GetPointer());

  polyDataFileSeriesReader->GetMTime();
  polyDataFileSeriesReader->RemoveAllFileNames();

  polyDataFileSeriesReader->AddFileName(file0);
  polyDataFileSeriesReader->AddFileName(file1);

  if (polyDataFileSeriesReader->GetNumberOfFileNames() != 2)
    {
    std::cerr << "Error: NumberOfFileNames != to the number of file added"
              << std::endl;
    return EXIT_FAILURE;
    }

  polyDataFileSeriesReader->GetFileName(5);
  if (strcmp(polyDataFileSeriesReader->GetFileName(1),file1) != 0)
    {
    std::cerr << "Error: GetFileName different than expected: " << std::endl
              << "Expected fileName: " << file1 << std::endl
              << "Filename retrived: " << polyDataFileSeriesReader->GetFileName(1)
              << std::endl;
    return EXIT_FAILURE;
    }

  // Set the default reader
  polyDataFileSeriesReader->CanReadFile(file1);
  polyDataFileSeriesReader->SetReader(polyDataReader.GetPointer());

  polyDataFileSeriesReader->SetUseMetaFile(true);
  if (!polyDataFileSeriesReader->GetUseMetaFile())
    {
    std::cerr << "Error: UseMetaFile true not set." << std::endl;
    return EXIT_FAILURE;
    }

  if (polyDataFileSeriesReader->CanReadFile(file0))
    {
    std::cerr << "Error: filename doesn not really points to a metafile, "
              << "must return 0" << std::endl;
    return EXIT_FAILURE;
    }
  polyDataFileSeriesReader->UseMetaFileOff();

  polyDataFileSeriesReader->GetMTime();

  if (!polyDataFileSeriesReader->CanReadFile(file1))
    {
    std::cerr << "CanReadFile return false on proper vtk file" << std::endl;
    return EXIT_FAILURE;
    }

  polyDataFileSeriesReader->Print(std::cout);
  return EXIT_SUCCESS;
}
