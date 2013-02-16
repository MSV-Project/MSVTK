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

#include<sstream>

#include<vtkPolyData.h>
#include<vtkSmartPointer.h>
#include<vtkNew.h>
#include<vtkXMLPolyDataReader.h>
#include<vtkXMLPolyDataWriter.h>
#include <vtkTestUtilities.h>

#include "msvFluidSimulator.h"

int msvFFSTest1(int ac, char **av)
{
  // Get the data test files
  const char* in_file =
  vtkTestUtilities::ExpandDataFileName(ac,av,"amr_lagrangian_dataset.vtp");
  const char* out_file =
  vtkTestUtilities::ExpandDataFileName(ac,av,"amr_caps_dataset.vtp");
  
  std::cout << in_file << std::endl;
  std::cout << out_file << std::endl;
  
  vtkSmartPointer<vtkXMLPolyDataReader> reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
  vtkSmartPointer<vtkXMLPolyDataWriter> writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  
  reader->SetFileName(in_file);
  reader->Update();
  
  vtkPolyData *data = reader->GetOutput();

  vtkNew<msvFluidSimulator> simulator;
  simulator->Init(data);

  return 0;
}
