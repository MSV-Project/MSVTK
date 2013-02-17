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
  Module:    msvVTKButtonsInterface.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME msvVTKButtonsInterface -
// .SECTION Description
//

// .SECTION See Also
//

#include "vtkObject.h"

// VTK_WIDGET includes
#include "msvVTKWidgetsExport.h"

// Forward references
class vtkButtonWidget;
class vtkImageData;
class vtkRenderer;

#ifndef __msvVTKButonsInterface_h
#define __msvVTKButonsInterface_h

//----------------------------------------------------------------------
class MSV_VTK_WIDGETS_EXPORT msvVTKButtonsInterface : public vtkObject
{
public:
  // Description:
  // Instantiate the class.
  static msvVTKButtonsInterface *New();

  // Description:
  // Standard methods for instances of the class.
  vtkTypeMacro(msvVTKButtonsInterface,vtkObject);

  // Description:
  // Allow to show/hide button
  void SetShowButton(bool show);
  vtkGetMacro(ShowButton,bool);

  // Description:
  // Allow to show/hide label
  vtkSetMacro(ShowLabel,bool);
  vtkGetMacro(ShowLabel,bool);

  // Description:
  // Specify the button's icon filename
  void SetImage(vtkImageData* image);
  vtkGetMacro(Image,vtkImageData*);

  // Description:
  // Specify the button's label
  vtkSetStringMacro(LabelText);
  vtkGetStringMacro(LabelText);

  // Description:
  // Specify the tooltip text
  vtkSetStringMacro(Tooltip);
  vtkGetStringMacro(Tooltip);

  // Description:
  // Represents the element bounds
  vtkGetVectorMacro(Bounds,double,6);
  vtkSetVectorMacro(Bounds,double,6)

  // Description:
  // Get the opacity value (in 0 1 range)
  vtkGetMacro(Opacity, double);

  // Description:
  // Set the previous opacity
  vtkSetMacro(PreviousOpacity,double);

  // Description:
  // Retrieve button widget pointer
  vtkButtonWidget *GetButton();

  // Description:
  // Add vtk button to Renderer
  virtual void SetCurrentRenderer(vtkRenderer *renderer);

  // Description:
  // Update graphic objects and render the view if requested.
  virtual void Update(bool render = true);

  // Description:
  // Get the current renderer
  inline vtkRenderer* GetRenderer(){return Renderer;};

  // Description:
  // Set the opacity value (in 0 1 range)
  void SetOpacity(double opacity);

  // Description:
  // Restore the previous opacity
  void RestorePreviousOpacity();

protected:
  // Description:
  // Object constructor
  msvVTKButtonsInterface();

  // Description:
  // Object destructor.
  virtual ~msvVTKButtonsInterface();

  // Callback called by picking on vtkButton
  vtkCommand * ButtonCallback;

  // Callback called by hovering over the button.
  vtkCommand * HighlightCallback;

  // Label of the button
  char * LabelText;

  // Tooltip associated to the button
  char * Tooltip;

  // Flag to show/hide button
  bool ShowButton;

  // Flag to show/hide label
  bool ShowLabel;

  // VTK button widget
  vtkButtonWidget * ButtonWidget;

  // Button image
  vtkImageData * Image;

  // Bounds of the data related to the buttonWin
  double Bounds[6];

  // Layout of the button's balloon
  int BalloonLayout;

  // Current renderer
  vtkRenderer* Renderer;

  // Button opacity
  double Opacity;

  // Previous button opacity
  double PreviousOpacity;

private:
  msvVTKButtonsInterface(const msvVTKButtonsInterface&);  //Not implemented
  void operator=(const msvVTKButtonsInterface&);  //Not implemented
};

#endif
