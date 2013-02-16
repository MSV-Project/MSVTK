/*==============================================================================

  Library: MSVTK

  Copyright (c) SCS s.r.l. (B3C)

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

// VTK includes
#include <vtkCamera.h>
#include <vtkCardinalSpline.h>
#include <vtkDoubleArray.h>
#include <vtkMath.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>

// MSVTK includes
#include "msvVTKAnimatePath.h"

//------------------------------------------------------------------------------
msvVTKAnimatePath::msvVTKAnimatePath()
{
  CameraPositions = vtkDoubleArray::New();
  CameraPositions->SetNumberOfComponents(3);
  FocalPoints = vtkDoubleArray::New();
  FocalPoints->SetNumberOfComponents(3);
  ViewUps = vtkDoubleArray::New();
  ViewUps->SetNumberOfComponents(3);
}

//------------------------------------------------------------------------------
void msvVTKAnimatePath::AddCameraPoint(double position[3], double focalPoint[3], double viewUp[3])
{
  CameraPositions->InsertNextTuple3(position[0],position[1],position[2]);
  FocalPoints->InsertNextTuple3(focalPoint[0],focalPoint[1],focalPoint[2]);
  ViewUps->InsertNextTuple3(viewUp[0],viewUp[1],viewUp[2]);
}

//------------------------------------------------------------------------------
void msvVTKAnimatePath::ClearPoints()
{
  CameraPositions->Reset();
  CameraPositions->SetNumberOfComponents(3);
  FocalPoints->Reset();
  FocalPoints->SetNumberOfComponents(3);
  ViewUps->Reset();
  ViewUps->SetNumberOfComponents(3);
}

//------------------------------------------------------------------------------
void msvVTKAnimatePath::Execute(vtkRenderer *renderer, double bounds[6],
  int numberOfSteps)
{
  assert(renderer);

  (void)bounds; // unused

  vtkCamera *camera = renderer->GetActiveCamera();

  vtkDoubleArray *splineCameraPositions = SplineProcess(CameraPositions,numberOfSteps);
  vtkDoubleArray *splineFocalPoints = SplineProcess(FocalPoints,numberOfSteps);
  vtkDoubleArray *splineViewUps = SplineProcess(ViewUps,numberOfSteps);

  for (int i = 0; i < numberOfSteps; i++)
  {
    camera->SetPosition(splineCameraPositions->GetTuple3(i));
    camera->SetFocalPoint(splineFocalPoints->GetTuple3(i));
    camera->SetViewUp(splineViewUps->GetTuple3(i));

    renderer->ResetCameraClippingRange();
    renderer->GetRenderWindow()->Render();
  }

  splineCameraPositions->Delete();
  splineFocalPoints->Delete();
  splineViewUps->Delete();
}

//------------------------------------------------------------------------------
msvVTKAnimatePath::~msvVTKAnimatePath()
{
  CameraPositions->Delete();
  FocalPoints->Delete();
  ViewUps->Delete();
}

//----------------------------------------------------------------------------
vtkDoubleArray *msvVTKAnimatePath::SplineProcess(vtkDoubleArray *input, int resolution)
//----------------------------------------------------------------------------
{
  vtkDoubleArray *output = vtkDoubleArray::New();
  output->SetNumberOfComponents(3);

  vtkCardinalSpline *splineX = vtkCardinalSpline::New();
  vtkCardinalSpline *splineY = vtkCardinalSpline::New();
  vtkCardinalSpline *splineZ = vtkCardinalSpline::New();

  //generating one spline for each branch (cell) of input polyline
  for (int p=0; p < input->GetNumberOfTuples(); p++)
  {

    double *point = input->GetTuple3(p);
    splineX->AddPoint(p, point[0]);
    splineY->AddPoint(p, point[1]);
    splineZ->AddPoint(p, point[2]);

  }

  for (int i = 0; i < resolution; i++)
  {
    double t = static_cast<double>(i) * static_cast<double>(input->GetNumberOfTuples()) / static_cast<double>(resolution);
    output->InsertNextTuple3(splineX->Evaluate(t),splineY->Evaluate(t),splineZ->Evaluate(t));
  }
  splineX->Delete();
  splineY->Delete();
  splineZ->Delete();
  return output;
}
