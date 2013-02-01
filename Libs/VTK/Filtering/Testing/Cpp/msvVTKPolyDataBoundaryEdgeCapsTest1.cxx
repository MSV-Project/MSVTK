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
#include "msvVTKBoundaryEdgeSources.h"

// VTK includes
#include "vtkNew.h"
#include "vtkTestUtilities.h"
#include "vtkPolyData.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkXMLPolyDataWriter.h"

// STD includes
#include <cstdlib>
#include <string>

// -----------------------------------------------------------------------------
int msvVTKPolyDataBoundaryEdgeCapsTest1(int argc, char* argv[])
{
  // Get the data test files
  const char* in_file =
  vtkTestUtilities::ExpandDataFileName(argc,argv,"geometry.vtp");
  const char* out_file =
  vtkTestUtilities::ExpandDataFileName(argc,argv,"amr_caps_dataset.vtp");
  
  std::cout << in_file << std::endl;
  std::cout << out_file << std::endl;
  
  // Create the PolyDataReader
  vtkNew<vtkXMLPolyDataReader> polyDataReader;
  vtkNew<msvVTKBoundaryEdgeSources> boundaryCaps;
  
  polyDataReader->SetFileName(in_file);
  polyDataReader->Update();
  
  boundaryCaps->SetInputConnection(polyDataReader->GetOutputPort());  
  boundaryCaps->Update();
  
  vtkNew<vtkXMLPolyDataWriter> poly_writer;
  
  poly_writer->SetInputConnection(boundaryCaps->GetOutputPort());
  poly_writer->SetDataModeToAscii();
  poly_writer->SetFileName(out_file);
  poly_writer->Write();  

  return EXIT_SUCCESS;
}
