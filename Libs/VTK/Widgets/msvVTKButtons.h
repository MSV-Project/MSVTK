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
  Module:    msvVTKButtons.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME msvVTKButtons -
// .SECTION Description
//

// .SECTION See Also
//

#include "vtkObject.h"

// VTK_WIDGET includes
#include "msvVTKButtonsInterface.h"

// Forward references
class vtkDataSet;
class vtkImageData;
class vtkRenderWindow;
class vtkCommand;

#ifndef __msvVTKButtons_h
#define __msvVTKButtons_h

//----------------------------------------------------------------------
class MSV_VTK_WIDGETS_EXPORT msvVTKButtons : public msvVTKButtonsInterface
{
public:
  // Instantiate the class.
  static msvVTKButtons *New();

  vtkTypeMacro(msvVTKButtons,msvVTKButtonsInterface);

  // Allow to activate FlyTo animation
  vtkSetMacro(FlyTo,bool);
  vtkGetMacro(FlyTo,bool);

  // Allow to set button position on center or on corner
  vtkSetMacro(OnCenter,bool);
  vtkGetMacro(OnCenter,bool);

  // set bounds
  void SetBounds(double b[6]);

  // Get the button preview image
  vtkImageData* GetPreview(int width, int height);

  // Data for preview
  vtkSetMacro(Data,vtkDataSet*);
  vtkGetMacro(Data,vtkDataSet*);

  // Determine if is a "corner" button
  vtkSetMacro(OnCorner,bool);
  vtkGetMacro(OnCorner,bool);

  // Determine if is a "corner" button
  vtkSetMacro(CornerIndex,int);
  vtkGetMacro(CornerIndex,int);

  //
  vtkSetMacro(YOffset,int);
  vtkGetMacro(YOffset,int);

  // Set the current renderer
  void SetCurrentRenderer(vtkRenderer *renderer);

  // Perform update
  void Update();

  // Calculate position (center or corner)
  void CalculatePosition();

  // Delete offscreen rendering window (usefull in mac osx)
  void DeleteWindow();

  //
  void GetDisplayPosition(double pos[2]);

  //
  void GetRealDisplayPosition(double pos[2]);

protected:
  // Object constructor
  msvVTKButtons();

  // Object destructor.
  virtual ~msvVTKButtons();

  //
  vtkRenderWindow* GetWindow();

  // dataset associated with the button
  vtkDataSet* Data;

  // render window for offscreen rendering
  vtkRenderWindow *Window;

  // Flag to activate FlyTo animation
  bool FlyTo;

  // Flag to set button position on center or on corner
  bool OnCenter;

  //
  //vtkCommand* RWICallback;

  //
  bool OnCorner;

  //
  int CornerIndex;

  //
  int YOffset;

private:
  msvVTKButtons(const msvVTKButtons&);  //Not implemented
  void operator=(const msvVTKButtons&);  //Not implemented
};

#endif
