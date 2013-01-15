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
  // Instantiate the class.
  static msvVTKButtonsInterface *New();

  // Description:
  // Standard methods for instances of the class.
  vtkTypeMacro(msvVTKButtonsInterface,vtkObject);

  // Description:
  // Allow to show/hide button
  vtkSetMacro(ShowButton,bool);
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
  void SetLabel(const char* label);
  //vtkSetMacro(Label,const char*);
  const char* GetLabel(){return LabelText;}

  // Description:
  // Specify the tooltip text
  vtkSetMacro(Tooltip,const char*);
  vtkGetMacro(Tooltip,const char*);

  // Description:
  // Represents the element bounds
  vtkGetVectorMacro(Bounds,double,6);
  vtkSetVectorMacro(Bounds,double,6)

  // Description:
  // Retrieve button pointer.
  vtkButtonWidget *GetButton();

  // Description:
  // add vtk button to Renderer
  void SetCurrentRenderer(vtkRenderer *renderer);

  // Description:
  // update graphic objects
  void Update();

  //
  inline vtkRenderer* GetRenderer(){return Renderer;};

protected:
  // Object constructor
  msvVTKButtonsInterface();

  // Object destructor.
  virtual ~msvVTKButtonsInterface();

  // Callback called by picking on vtkButton
  vtkCommand * ButtonCallback;

  // Callback called by hovering over the button.
  vtkCommand * HighlightCallback;

  // Label of the button
  char * LabelText;

  // Tooltip associated to the button
  const char * Tooltip;

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

  //
  int BalloonLayout;

  //
  vtkRenderer* Renderer;

private:
  msvVTKButtonsInterface(const msvVTKButtonsInterface&);  //Not implemented
  void operator=(const msvVTKButtonsInterface&);  //Not implemented
};

#endif
