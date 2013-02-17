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
#include "msvVTKWidgetClusters.h"

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
#include "vtkCamera.h"
#include "vtkMinimalStandardRandomSequence.h"
#include "vtkSmartPointer.h"
#include <vtkWidgetRepresentation.h>
#include <vtkCubeSource.h>
#include <vtkPolyDataMapper.h>
#include <msvVTKProp3DButtonRepresentation.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>

const unsigned int colorCount = 8;
const double colors[colorCount][3] = { {0.925490196,  0.17254902, 0.2},
  {0.070588235, 0.545098039, 0.290196078},
  {0.086274509, 0.364705882, 0.654901961},
  {0.952941176, 0.482352941, 0.176470588},
  {0.396078431, 0.196078431, 0.560784314},
  {0.631372549, 0.109803922, 0.176470588},
  {0.698039216, 0.235294118, 0.576470588},
  {0.003921568, 0.007843137, 0.007843137}};

struct ButtonProp : vtkObjectBase
{
  ButtonProp()
  {
    this->Cube = vtkCubeSource::New();
    this->CubeMapper = vtkPolyDataMapper::New();
    this->CubeMapper->SetInputConnection(Cube->GetOutputPort());
    this->CubeMapper->SetScalarRange(0,19);
    this->CubeActor = vtkActor::New();
    this->CubeActor->SetMapper(CubeMapper);
  }
  
  // vtkProp for the first state of our button
  vtkSmartPointer<vtkCubeSource>      Cube;
  vtkSmartPointer<vtkPolyDataMapper>  CubeMapper;
  vtkSmartPointer<vtkActor>           CubeActor;
};

void getPoints(vtkPoints *points)
{
  int const N = 50;
  points->SetNumberOfPoints(N);
  vtkNew<vtkMinimalStandardRandomSequence> random;

  for (int i = 0; i < N; ++i)
  {
    double center[3] = {0};
    random->Next();
    center[0] = random->GetValue();
    random->Next();
    center[1] = random->GetValue();
    random->Next();
    center[2] = random->GetValue();

    points->SetPoint(i,center);
  }
}


  // -----------------------------------------------------------------------------
int msvVTKWidgetClustersTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
  {
    // Create the RenderWindow and Renderer
    vtkNew<vtkRenderer> render;
    vtkNew<vtkRenderWindow> renderWindow;
    renderWindow->MakeRenderWindowInteractor();
    render->SetRenderWindow(renderWindow.GetPointer());
    vtkCamera* cam = render->MakeCamera();
    render->SetActiveCamera(cam);
    render->ResetCamera();

    // Randomly located points
    vtkNew<vtkPoints> points;
    getPoints(points.GetPointer());

    // Create the msvVTKWidgetClusters
    vtkNew<msvVTKWidgetClusters> widgetClusters;
    widgetClusters->Clear();
    widgetClusters->SetRenderer(render.GetPointer());
    widgetClusters->SetDataSet(0,0,points.GetPointer());
    widgetClusters->UpdateWidgets();


    return EXIT_SUCCESS;
  }

