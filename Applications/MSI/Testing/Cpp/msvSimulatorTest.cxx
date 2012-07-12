
#include<vtkPolyData.h>
#include<vtkPolyDataReader.h>
#include<vtkXMLHierarchicalBoxDataWriter.h>
#include<vtkHierarchicalBoxDataSet.h>
#include<vtkCompositeDataPipeline.h>

#include "msvFluidSimulator.h"
// Headers for basic PETSc functions
#include <petscsys.h>


int main(int ac, char **av)
{
  std::string filename = "/home/rortiz/projects/MSVTK/Applications/MSI/input3d";
  vtkSmartPointer<vtkPolyDataReader> reader = vtkSmartPointer<vtkPolyDataReader>::New();
  vtkSmartPointer<vtkXMLHierarchicalBoxDataWriter> writer = vtkSmartPointer<vtkXMLHierarchicalBoxDataWriter>::New();
  
  reader->SetFileName("/home/rortiz/Documents/Data/CerebralAneurysm/Morpho/geometry.vtk");
  reader->Update();
  
  vtkPolyData *data = reader->GetOutput();
  
  PetscInitialize(&ac,&av,PETSC_NULL,PETSC_NULL);
  {
    msvFluidSimulator simulator;
    simulator.msvInitializeAMR(filename,4,5,data);
    // AMR writer
//     writer->SetInput(simulator.getAMRDataSet());
//     writer->SetDataModeToBinary();
//     std::string amrfilename("amr_dataset.");
//     amrfilename += writer->GetDefaultFileExtension();
//     writer->SetFileName(amrfilename.c_str());
//     writer->Write();
  }
  PetscFinalize();
  
  return 0;
}