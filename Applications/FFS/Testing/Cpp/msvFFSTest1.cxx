
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