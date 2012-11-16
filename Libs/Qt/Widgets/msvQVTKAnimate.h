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

#ifndef msvQVTKAnimate_H
#define msvQVTKAnimate_H

// MSVTK includes
#include "msvQtWidgetsExport.h"
#include "msvQVTKButtonsAction.h"

// forward references
class vtkRenderer;

/**
Class name: msvQVTKAnimate
This is an utility class to animate VTKCamera.
*/

class MSV_QT_WIDGETS_EXPORT msvQVTKAnimate : public msvQVTKButtonsAction
{

public:
  /// Object constructor.
  msvQVTKAnimate();

  /// Animate the camera to zoom on the passed bounding box.
  void execute(vtkRenderer *renderer, double bounds[6],
    int numberOfSteps = 120);

  /// Object destructor.
  virtual ~msvQVTKAnimate();

};

#endif // msvQVTKAnimate_H
