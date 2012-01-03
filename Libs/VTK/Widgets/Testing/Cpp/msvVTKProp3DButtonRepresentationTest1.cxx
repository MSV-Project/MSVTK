/*==============================================================================

  Program: MSVTK

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

// MSVTK includes
#include "msvVTKProp3DButtonRepresentation.h"

// STD includes
#include <cstdlib>
#include <iostream>

// VTK includes
#include <vtkActor.h>
#include <vtkButtonWidget.h>
#include <vtkCubeSource.h>
#include <vtkNew.h>
#include <vtkPlatonicSolidSource.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp3D.h>
#include <vtkPropCollection.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

// -----------------------------------------------------------------------------
int msvVTKProp3DButtonRepresentationTest1(int, char* [])
{
  // Create the RenderWindow and Renderer
  vtkNew<vtkRenderer> render;
  vtkNew<vtkRenderWindow> renWin;
  renWin->AddRenderer(render.GetPointer());

  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin.GetPointer());

  // Use of vtkButtonWidget through a msvVTKProp3DButtonRepresentation
  vtkNew<vtkPlatonicSolidSource> tet;
  tet->SetSolidTypeToTetrahedron();
  vtkNew<vtkPolyDataMapper> tetMapper;
  tetMapper->SetInputConnection(tet->GetOutputPort());
  tetMapper->SetScalarRange(0,19);
  vtkNew<vtkActor> tetActor;
  tetActor->SetMapper(tetMapper.GetPointer());

  vtkNew<vtkCubeSource> cube;
  vtkNew<vtkPolyDataMapper> cubeMapper;
  cubeMapper->SetInputConnection(cube->GetOutputPort());
  cubeMapper->SetScalarRange(0,19);
  vtkNew<vtkActor> cubeActor;
  cubeActor->SetMapper(cubeMapper.GetPointer());

  vtkNew<msvVTKProp3DButtonRepresentation> prop3DButtonRep;

  double* initBounds = prop3DButtonRep->GetBounds();
  if (initBounds)
    {
    std::cerr << "Error: bounds retrieved from an empty representation."
              << std::endl;
    return EXIT_FAILURE;
    }

  // Check rendering on an empty representation.
  prop3DButtonRep->RenderVolumetricGeometry(render.GetPointer());
  prop3DButtonRep->RenderOpaqueGeometry(render.GetPointer());
  prop3DButtonRep->RenderTranslucentPolygonalGeometry(render.GetPointer());
  prop3DButtonRep->HasTranslucentPolygonalGeometry();

  // Initialize
  prop3DButtonRep->SetNumberOfStates(2);
  prop3DButtonRep->SetButtonProp(-1,tetActor.GetPointer());
  prop3DButtonRep->SetButtonProp(1,cubeActor.GetPointer());
  prop3DButtonRep->SetButtonProp(4,cubeActor.GetPointer());
  prop3DButtonRep->SetPlaceFactor(1);
  prop3DButtonRep->SetState(0);

  // Representations
  vtkProp3D* prop3D = prop3DButtonRep->GetButtonProp(4);
  prop3D = prop3DButtonRep->GetButtonProp(1);
  if (prop3D != cubeActor.GetPointer())
    {
    std::cerr << "Error: vtkProp3D retrieved not the one expected [cubeActor]"
              << std::endl;
    return EXIT_FAILURE;
    }
  prop3D = prop3DButtonRep->GetButtonProp(-1);
  if (prop3D != tetActor.GetPointer())
    {
    std::cerr << "Error: vtkProp3D retrieved not the one expected [tetActor]"
              << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkPropCollection> pc;
  prop3DButtonRep->GetActors(pc.GetPointer());
  if (pc->GetNumberOfItems() != 1) // Number of actors of the current vtkProp
    {
    std::cerr << "Error: Number of actors retrieved different than expected."
              << std::endl;
    return EXIT_FAILURE;
    }

  // Bound and PlaceWidget method
  double bounds1[6] = {-0.75, 0.75, -0.75, 0.75, -0.5, 0.75};
  prop3DButtonRep->PlaceWidget(bounds1);
  double * getBounds1 = prop3DButtonRep->GetBounds();

  if (bounds1[0] != getBounds1[0] || bounds1[1] != getBounds1[1] ||
      bounds1[2] != getBounds1[2] || bounds1[3] != getBounds1[3] ||
      bounds1[4] != getBounds1[4] || bounds1[5] != getBounds1[5])
    {
    std::cerr << "Error: Unexpected bounds after calling PlaceWidget"
              << std::endl;
    return EXIT_FAILURE;
    }

  double bounds2[6] = {0, 1.5, 0, 1.5, 0, 1.5};
  prop3DButtonRep->PlaceWidget(bounds2);
  prop3DButtonRep->PlaceWidget(bounds2); // Must not recompute the bounds
  double * getBounds2 = prop3DButtonRep->GetBounds();
  if (bounds2[0] != getBounds2[0] || bounds2[1] != getBounds2[1] ||
      bounds2[2] != getBounds2[2] || bounds2[3] != getBounds2[3] ||
      bounds2[4] != getBounds2[4] || bounds2[5] != getBounds2[5])
    {
    std::cerr << "Error: Unexpected bounds after calling PlaceWidget method"
              << std::endl;
    return EXIT_FAILURE;
    }

  // Create Widget
  vtkNew<vtkButtonWidget> buttonWidget;
  buttonWidget->SetInteractor(iren.GetPointer());
  buttonWidget->SetRepresentation(prop3DButtonRep.GetPointer());

  // Rendering
  render->SetBackground(0.1, 0.2, 0.4);
  renWin->SetSize(300, 300);

  iren->Initialize();
  renWin->Render();
  buttonWidget->EnabledOn();

  // Compute Rendering on the representation
  prop3DButtonRep->ComputeInteractionState(150,150);
  prop3DButtonRep->RenderVolumetricGeometry(render.GetPointer());
  prop3DButtonRep->RenderOpaqueGeometry(render.GetPointer());
  prop3DButtonRep->RenderTranslucentPolygonalGeometry(render.GetPointer());
  prop3DButtonRep->HasTranslucentPolygonalGeometry();
  prop3DButtonRep->ComputeInteractionState(299,299);

  // Compute Rendering on the representation when following the camera
  prop3DButtonRep->FollowCameraOn();
  if (prop3DButtonRep->GetFollowCamera() != static_cast<int>(true))
    {
    std::cerr << "Error: FollowCameraOn method is not effective" << std::endl;
    return EXIT_FAILURE;
    }

  prop3DButtonRep->ComputeInteractionState(150,150);
  prop3DButtonRep->RenderVolumetricGeometry(render.GetPointer());
  prop3DButtonRep->RenderOpaqueGeometry(render.GetPointer());
  prop3DButtonRep->RenderTranslucentPolygonalGeometry(render.GetPointer());
  prop3DButtonRep->HasTranslucentPolygonalGeometry();
  prop3DButtonRep->ComputeInteractionState(299,299);
  prop3DButtonRep->GetBounds();

  // Test toolkit methods
  vtkNew<msvVTKProp3DButtonRepresentation> prop3DButtonRepCopy;
  prop3DButtonRepCopy->ShallowCopy(prop3DButtonRep.GetPointer());
  prop3DButtonRep->Print(std::cout);

  return EXIT_SUCCESS;
}
