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

#ifndef __msvVTKAnimatePath_h
#define __msvVTKAnimatePath_h

// MSVTK includes
#include "msvVTKButtonsAction.h"

// forward references
class vtkRenderer;
class vtkDoubleArray;

// .NAME msvQVTKAnimate - a utility class to animate VTKCamera.
// .SECTION Description
//
class MSV_VTK_WIDGETS_EXPORT msvVTKAnimatePath : public msvVTKButtonsAction
{

public:
  // Description:
  // Object constructor.
  msvVTKAnimatePath();

  // Description:
  // Object destructor.
  virtual ~msvVTKAnimatePath();

  // Description:
  // Animate the camera to zoom on the passed bounding box.
  virtual void Execute(vtkRenderer *renderer, double bounds[6],
                       int numberOfSteps = 120);

  // Description:
  // Add camera brakpoint to animation
  void AddCameraPoint(double position[3], double focalPoint[3], double viewUp[3]);

  // Description:
  // Clear all camera breakponints
  void ClearPoints();

  // Description:
  // Perform spline on the specified array
  vtkDoubleArray * SplineProcess(vtkDoubleArray *input, int resolution);

private:
  // Array of camera positions
  vtkDoubleArray * CameraPositions;

  // Array of focal points
  vtkDoubleArray * FocalPoints;

  // Array of view up
  vtkDoubleArray * ViewUps;

};

#endif // __msvVTKAnimatePath_h
