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
#include "msvVTKXMLMultiblockLODReader.h"

// VTK includes
#include "vtkActor.h"
#include "vtkCommand.h"
#include "vtkCompositeDataIterator.h"
#include "vtkCompositePolyDataMapper.h"
#include "vtkXMLMultiBlockDataReader.h"
#include "vtkNew.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPolyDataReader.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStringArray.h"
#include "vtkTestUtilities.h"
#include "vtkXMLReader.h"

// STD includes
#include <cstdlib>
#include <iostream>
#include <string>

//------------------------------------------------------------------------------
// Switch the level of details between the min/max resolution
class vtkSwitchLODCallback : public vtkCommand
{
public:
  static vtkSwitchLODCallback *New(){return new vtkSwitchLODCallback;}
  virtual void Execute(vtkObject *caller, unsigned long, void*)
    {
    vtkRenderWindowInteractor *iren = static_cast<vtkRenderWindowInteractor*>(caller);
    if(vtkStdString(iren->GetKeySym()) == "Control_L" ||
       vtkStdString(iren->GetKeySym()) == "Control_R")
      {
      highResolutionMode = !highResolutionMode;
      if(highResolutionMode)
        {
        std::cout << "High Resolution ON !" << std::endl;
        pReader->SetDefaultLOD(2);
        }
      else
        {
        std::cout << "High Resolution OFF !" << std::endl;
        pReader->SetDefaultLOD(0);
        }
      }
    else if(vtkStdString(iren->GetKeySym()) == "Shift_L")
      {
      std::cout << "Change Given LOD piece" << this->piece << std::endl;
      pReader->SetPieceLOD(this->piece++, 0);
      }
    else if (vtkStdString(iren->GetKeySym()) == "Shift_R")
      {
      std::cout << "Modified the reader without changing its properties."
                << "Check if there is no useless read" << std::endl;
      pReader->Modified();
      }
    }

  // Ctors
  vtkSwitchLODCallback():highResolutionMode(false), pReader(0), piece(0) {}

  // Attributes
  bool highResolutionMode;
  int piece;
  vtkSmartPointer<msvVTKXMLMultiblockLODReader> pReader;
};

// -----------------------------------------------------------------------------
int msvVTKXMLMultiblockLODReaderTest1(int argc, char* argv[])
{
  // Get the data test files
  // const char* file0 =
  //   vtkTestUtilities::ExpandDataFileName(argc,argv,"compositeMetaData.xml");

  const char* file = "/home/michael/HumanAnatomy/humanAnatomy.xml";

  /*--------------------------------------------------------------------------*/
  // Standard rendering classes
  /*--------------------------------------------------------------------------*/
  //
  vtkNew<vtkRenderer> ren1;
  ren1->SetBackground(0.1, 0.2, 0.4);
  ren1->SetBackground2(0.2, 0.4, 0.8);
  ren1->SetGradientBackground(true);

  vtkNew<vtkRenderWindow> renWin;
  renWin->AddRenderer(ren1.GetPointer());

  vtkNew<vtkRenderWindowInteractor> iren;
  vtkNew<vtkInteractorStyleTrackballCamera> irenStyle;
  iren->SetRenderWindow(renWin.GetPointer());
  iren->SetInteractorStyle(irenStyle.GetPointer());


  /*--------------------------------------------------------------------------*/
  // CompositeLODReader
  /*--------------------------------------------------------------------------*/
  // Create the reader
  vtkNew<msvVTKXMLMultiblockLODReader> compositeMultiblocLODReader;
  compositeMultiblocLODReader->SetFileName(file);

  // Callback to the swaper interaction function
  vtkNew<vtkSwitchLODCallback> resolutionMode;
  resolutionMode->pReader = compositeMultiblocLODReader.GetPointer();

  // Link the Swapper to the RenderWindowInteractor
  iren->AddObserver(vtkCommand::KeyPressEvent, resolutionMode.GetPointer());

  // Tests
 if (!compositeMultiblocLODReader->CanReadFile(file))
    {
    std::cerr << "Error: Cannot read the file: " << file
              << std::endl;
    return EXIT_FAILURE;
    }

  // Create actor
  vtkNew<vtkActor> compositeActor;
  vtkNew<vtkCompositePolyDataMapper> compositePolyMapper;

  compositePolyMapper->SetInputConnection(
    compositeMultiblocLODReader->GetOutputPort());
  compositeActor->SetMapper(compositePolyMapper.GetPointer());
  compositeMultiblocLODReader->Update();

  vtkMultiBlockDataSet* dataSet = vtkMultiBlockDataSet::SafeDownCast(
    compositePolyMapper->GetInputDataObject(0,0));
  if (!dataSet)
    {
    std::cerr << "Error: vtkMultiBlockDataSet = null"
              << std::endl;
    return EXIT_FAILURE;
    }

  /*--------------------------------------------------------------------------*/
  // CompositeDataIterator
  /*--------------------------------------------------------------------------*/
  vtkCompositeDataIterator* it = dataSet->NewIterator();
  it->InitTraversal ();
  if (!it)
    {
    std::cerr << "Error: vtkMultiBlockDataSet = null"
              << std::endl;
    return EXIT_FAILURE;
    }

  while ( it->IsDoneWithTraversal() == 0 )
    {
    std::cout << "Active Data [FlatIndex]: "
              << it->GetCurrentFlatIndex() << std::endl;
    it->GoToNextItem();
    }
  compositeMultiblocLODReader->Print(std::cout);

  // Render scene
  iren->Initialize();
  ren1->AddActor(compositeActor.GetPointer());
  ren1->ResetCamera(compositeActor->GetBounds());
  renWin->Render();
  iren->Start();

  return EXIT_SUCCESS;
}
