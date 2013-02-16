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
#include<fstream>

#include<vtkSmartPointer.h>
#include<vtkNew.h>
#include<vtkPolyData.h>
#include<vtkXMLPolyDataReader.h>
#include<vtkXMLPolyDataWriter.h>
#include<vtkXMLHierarchicalBoxDataWriter.h>
#include<vtkHierarchicalBoxDataSet.h>
#include <vtkTestUtilities.h>

#include "msvFluidSimulator.h"
// Headers for basic PETSc functions
#include <petscsys.h>

void dump_boundary(vtkPolyData *data);

int msvFFSTest2(int ac, char **av)
{
  // Get the data test files
  const char* in_file =
  vtkTestUtilities::ExpandDataFileName(ac,av,"amr_lagrangian_dataset.vtp");
  const char* out_file =
  vtkTestUtilities::ExpandDataFileName(ac,av,"amr_grid_dataset.");
  const char* in_database = "Testing/Data/simulator.config";
  
  vtkSmartPointer<vtkXMLPolyDataReader> reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
  vtkSmartPointer<vtkXMLHierarchicalBoxDataWriter> writer = vtkSmartPointer<vtkXMLHierarchicalBoxDataWriter>::New();
  vtkSmartPointer<vtkHierarchicalBoxDataSet> dataset = vtkSmartPointer<vtkHierarchicalBoxDataSet>::New();
  
  reader->SetFileName(in_file);
  reader->Update();dump_boundary(reader->GetOutput());
  
  vtkPolyData *data = reader->GetOutput();
  
  PetscInitialize(&ac,&av,PETSC_NULL,PETSC_NULL);
  {
    vtkNew<msvFluidSimulator> simulator;
    simulator->SetInitFile(in_database);
    simulator->SetMaxLevels(4);
    simulator->SetCoarsestGridSpacing(8);
    simulator->SetAMRDataset(dataset);
    simulator->Init(data);
    // AMR writer
    writer->SetInput(dataset);
    writer->SetDataModeToBinary();
    std::string amrfilename(out_file);
    amrfilename += writer->GetDefaultFileExtension();
    writer->SetFileName(amrfilename.c_str());
    writer->Write();    
  }
  PetscFinalize();
  
  return 0;
}

void dump_boundary(vtkPolyData *data)
{
  std::ofstream file("./data_points.vertex");
  std::cout << "Writting boundary to file... ";
  vtkPoints *points = data->GetPoints();
  vtkIdType size = points->GetNumberOfPoints();
  
  file << size << std::endl;
  file.precision(16);
  for(vtkIdType i = 0; i < size; ++i)
  {
    double *point = points->GetPoint(i);
    file << point[0] << " " << point[1] << " " << point[2] << std::endl;    
  }
  file.close();
  std::cout << "done." << std::endl;
}
