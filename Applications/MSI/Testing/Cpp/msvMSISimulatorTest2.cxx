
#include<sstream>
#include<fstream>

#include<vtkSmartPointer.h>
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

int msvMSISimulatorTest2(int ac, char **av)
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
    msvFluidSimulator simulator;
    simulator.msvInitializeAMR(in_database,4,5,data);
    simulator.amrToVTK(dataset);
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