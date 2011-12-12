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
#include "msvVTKECGCartoPointsReader.h"

// VTK includes
#include "vtkNew.h"
#include "vtkTestUtilities.h"

// STD includes
#include <cstdlib>
#include <string>

// -----------------------------------------------------------------------------
int msvVTKECGCartoPointsReaderTest1(int argc, char* argv[])
{
  // Get the data test files
  const char* file0 =
    vtkTestUtilities::ExpandDataFileName(argc,argv,"Polydata00.vtk");
  const char* file1 =
    vtkTestUtilities::ExpandDataFileName(argc,argv,"Polydata00.vtk");

  // Create the fileSeriesReader
  vtkNew<vtkPolyDataReader> polyDataReader;
  vtkNew<msvVTKECGCartoPointsReader> cartoPointsReader;

  cartoPointsReader->SetReader(polyDataReader.GetPointer());

  cartoPointsReader->GetMTime();
  cartoPointsReader->RemoveAllFileNames();

  cartoPointsReader->AddFileName(file0);
  cartoPointsReader->AddFileName(file1);

  if (cartoPointsReader->GetNumberOfFileNames() != 2)
    {
    std::cerr << "Error: NumberOfFileNames != to the number of file added"
              << std::endl;
    return EXIT_FAILURE;
    }

  cartoPointsReader->GetFileName(5);
  if (strcmp(cartoPointsReader->GetFileName(1),file1) != 0)
    {
    std::cerr << "Error: GetFileName different than expected: " << std::endl
              << "Expected fileName: " << file1 << std::endl
              << "Filename retrived: " << cartoPointsReader->GetFileName(1)
              << std::endl;
    return EXIT_FAILURE;
    }

  // Set the default reader
  cartoPointsReader->CanReadFile(file1);
  cartoPointsReader->SetReader(polyDataReader.GetPointer());
  cartoPointsReader->GetMTime();

  if (!cartoPointsReader->CanReadFile(file1))
    {
    std::cerr << "CanReadFile return false on proper vtk file" << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
