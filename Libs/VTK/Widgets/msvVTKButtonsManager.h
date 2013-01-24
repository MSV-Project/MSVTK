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
  Module:    msvVTKButtonsManager.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME msvVTKButtonsManager -
// .SECTION Description
// Manager class to manage groups of msvVTKButtons and msvVTKButtonsGroup

// .SECTION See Also
//

#ifndef __msvVTKButtonsManager_h
#define __msvVTKButtonsManager_h

#include "vtkObject.h"

// MSVTK includes
#include "msvVTKButtons.h"
#include "msvVTKButtonsGroup.h"

class vtkCameraCallback;

class  msvVTKButtonsManager : public vtkObject
{
public:
  // Description:
  // Object constructor
  msvVTKButtonsManager();

  // Description:
  // Object destructor
  ~msvVTKButtonsManager();

  // Description:
  // Get the singleton instance of the manager
  static msvVTKButtonsManager* GetInstance();

  // Description:
  // Create a group
  msvVTKButtonsGroup *CreateGroup();

  // Description:
  // Create a button
  msvVTKButtons *CreateButtons();

  // Description:
  // Set show button property for children elements
  void SetShowButton(bool show);

  // Description:
  // Set show label property for children elements
  void SetShowLabel(bool show);

  // Description:
  // Add an element
  void AddElement(msvVTKButtonsInterface *element);

  // Description:
  // Get an element
  msvVTKButtonsInterface * GetElement(int index);

  // Description:
  // Get the number of elements
  inline int GetNumberOfElements(){return Elements.size();};

  // Description:
  // Set the renderer
  void SetRenderer(vtkRenderer* renderer);

private:
  // Vector of elements
  std::vector<msvVTKButtonsInterface*> Elements;

  // Callback for camera modified event
  vtkCameraCallback* CameraCallback;
};

#endif // __msvVTKButtonsManager_h
