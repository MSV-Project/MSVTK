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
/*=========================================================================

  Program:   Visualization Toolkit
  Module:    msvVTKAnimate.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef msvVTKAnimate_H
#define msvVTKAnimate_H

// MSVTK includes
#include "msvVTKWidgetsExport.h"
#include "msvVTKButtonsAction.h"

// forward references
class vtkRenderer;

/**
Class name: msvQVTKAnimate
This is an utility class to animate VTKCamera.
*/

class MSV_VTK_WIDGETS_EXPORT msvVTKAnimate : public msvVTKButtonsAction
{

public:
  /// Object constructor.
  msvVTKAnimate();

  /// Animate the camera to zoom on the passed bounding box.
  void Execute(vtkRenderer *renderer, double bounds[6],
    int numberOfSteps = 120);

  /// Object destructor.
  virtual ~msvVTKAnimate();

};

#endif // msvVTKAnimate_H
