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

#ifndef msvQVTKButtonsAction_H
#define msvQVTKButtonsAction_H

// Includes list
#include "msvQtWidgetsExport.h"

// Forward references
class vtkRenderer;

/**
Class name: msvQVTKButtonsAction
Interface abstract class for buttons actions
*/

class MSV_QT_WIDGETS_EXPORT msvQVTKButtonsAction
{

public:
  /// Object constructor.
  msvQVTKButtonsAction();

  /// Animate the camera to zoom on the passed bounding box.
  virtual void execute(vtkRenderer *renderer, double bounds[6], int numberOfSteps = 120)=0;

  /// Object destructor.
  virtual ~msvQVTKButtonsAction();

};

#endif // msvQVTKButtonsAction_H
