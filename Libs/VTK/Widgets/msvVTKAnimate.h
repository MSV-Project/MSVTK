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

#ifndef __msvVTKAnimate_h
#define __msvVTKAnimate_h

// MSVTK includes
#include "msvVTKButtonsAction.h"

// forward references
class vtkRenderer;

// .NAME msvQVTKAnimate - a utility class to animate VTKCamera.
// .SECTION Description
//
class MSV_VTK_WIDGETS_EXPORT msvVTKAnimate : public msvVTKButtonsAction
{

public:
  /// Object constructor.
  msvVTKAnimate();

  /// Object destructor.
  virtual ~msvVTKAnimate();

  /// Animate the camera to zoom on the passed bounding box.
  virtual void Execute(vtkRenderer *renderer, double bounds[6],
                       int numberOfSteps = 120);

};

#endif // __msvVTKAnimate_h
