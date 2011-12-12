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

// MSVTK includes
#include "msvVTKECGButtonsManager.h"

// STD includes
#include <cstdlib>

// VTK includes
#include "vtkButtonWidget.h"
#include "vtkDoubleArray.h"
#include "vtkIntArray.h"
#include "vtkNew.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkRenderer.h"

// -----------------------------------------------------------------------------
int msvVTKECGButtonsManagerTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  // Create the RenderWindow and Renderer
  vtkNew<vtkRenderer> render;

  // Create a float array which represents the points.
  vtkNew<vtkDoubleArray> pCoords;
  pCoords->SetNumberOfComponents(3);
  pCoords->SetNumberOfTuples(4);
  float pts[4][3] = { {0.0, 0.0, 0.0}, {0.0, 1.0, 0.0},
                     {1.0, 0.0, 0.0}, {1.0, 1.0, 0.0} };
  for (int i=0; i<4; ++i)
   {
   pCoords->SetTuple(i, pts[i]);
   }

  // Create vtkPoints and assign pCoords as the internal data array.
  vtkNew<vtkPoints> points;
  points->SetData(pCoords.GetPointer());

  vtkNew<vtkPolyData> polydata;              // create a vtkPolyData
  polydata->SetPoints(points.GetPointer());  // Assign points

  // Create the msvVTKECGButtonsManager
  vtkNew<msvVTKECGButtonsManager> buttonsManager;
  buttonsManager->Clear();
  buttonsManager->SetRenderer(render.GetPointer());
  buttonsManager->UpdateButtonWidgets(polydata.GetPointer());
  buttonsManager->Init(polydata.GetPointer());
  buttonsManager->UpdateButtonWidgets(polydata.GetPointer());

  // Process on a random button outside the manager
  vtkNew<vtkButtonWidget> buttonWidget;
  buttonsManager->ProcessWidgetsEvents(buttonWidget.GetPointer(), 0,
                                       buttonsManager.GetPointer(), NULL);

  return EXIT_SUCCESS;
}
