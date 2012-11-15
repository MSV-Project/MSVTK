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
  Module:    msvVTKSliderFixedRepresentation2D.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME msvVTKSliderFixedRepresentation2D -
// .SECTION Description
//

// .SECTION See Also
//

// The current class has been overridden...

#ifndef __msvVTKSliderFixedRepresentation2D_h
#define __msvVTKSliderFixedRepresentation2D_h

#include "vtkSliderRepresentation2D.h"

// VTK_WIDGET includes
#include "msvVTKWidgetsExport.h"

class MSV_VTK_WIDGETS_EXPORT msvVTKSliderFixedRepresentation2D : public vtkSliderRepresentation2D
{
public:
  // Description:
  // Instantiate the class.
  static msvVTKSliderFixedRepresentation2D *New();

  // Description:
  // Standard methods for instances of the class.
  vtkTypeMacro(msvVTKSliderFixedRepresentation2D,vtkSliderRepresentation2D);

  //
  void BuildRepresentation();

  vtkSetVectorMacro(Scale,double,2);
  vtkSetVectorMacro(Translate,double,2);

protected:
  msvVTKSliderFixedRepresentation2D();
  ~msvVTKSliderFixedRepresentation2D();

  double Scale[2];
  double Translate[2];

private:
  msvVTKSliderFixedRepresentation2D(const msvVTKSliderFixedRepresentation2D&);  //Not implemented
  void operator=(const msvVTKSliderFixedRepresentation2D&);                     //Not implemented
};

#endif