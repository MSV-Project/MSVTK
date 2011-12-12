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
  Module:    vtkProp3DButtonRepresentation.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkProp3DButtonRepresentation - defines a representation for a vtkButtonWidget
// .SECTION Description
// This class implements one type of vtkButtonRepresentation. Each button
// state can be represented with a separate instance of vtkProp3D. Thus
// buttons can be represented with vtkActor, vtkImageActor, volumes (e.g.,
// vtkVolume) and/or any other vtkProp3D. Also, the class invokes events when
// highlighting occurs (i.e., hovering, selecting) so that appropriate action
// can be taken to highlight the button (if desired).
//
// To use this representation, always begin by specifying the number of
// button states.  Then provide, for each state, an instance of vtkProp3D.
//
// This widget representation uses the conventional placement method. The
// button is placed inside the bounding box defined by PlaceWidget by translating
// and scaling the vtkProp3D to fit (each vtkProp3D is transformed). Therefore,
// you must define the number of button states and each state (i.e., vtkProp3D)
// prior to calling vtkPlaceWidget.

// .SECTION See Also
// vtkButtonWidget vtkButtonRepresentation vtkButtonSource vtkEllipticalButtonSource
// vtkRectangularButtonSource

// The current class has been overided because of a bug found in the original
// class of VTK. The PlaceWidget method has been modified. It now fits the actor
// bounds in the place bounds by tampering with its transform if one.

#ifndef __msvVTKProp3DButtonRepresentation_h
#define __msvVTKProp3DButtonRepresentation_h

#include "vtkButtonRepresentation.h"

// VTK_WIDGET includes
#include "msvVTKWidgetsExport.h"

class vtkPropPicker;
class vtkProp3D;
class vtkProp3DFollower;
class vtkPropArray; //PIMPLd

class MSV_VTK_WIDGETS_EXPORT msvVTKProp3DButtonRepresentation : public vtkButtonRepresentation
{
public:
  // Description:
  // Instantiate the class.
  static msvVTKProp3DButtonRepresentation *New();

  // Description:
  // Standard methods for instances of the class.
  vtkTypeMacro(msvVTKProp3DButtonRepresentation,vtkButtonRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Add the ith texture corresponding to the ith button state.
  // The parameter i should be (0 <= i < NumberOfStates).
  void SetButtonProp(int i, vtkProp3D *prop);
  vtkProp3D *GetButtonProp(int i);

  // Description:
  // Specify whether the button should always face the camera. If enabled,
  // the button reorients itself towards the camera as the camera moves.
  vtkSetMacro(FollowCamera,int);
  vtkGetMacro(FollowCamera,int);
  vtkBooleanMacro(FollowCamera,int);

  // Description:
  // Extend the vtkButtonRepresentation::SetState() method.
  virtual void SetState(int state);

  // Description:
  // Provide the necessary methods to satisfy the vtkWidgetRepresentation API.
  virtual int ComputeInteractionState(int X, int Y, int modify=0);
  virtual void BuildRepresentation();

  // Description:
  // This method positions (translates and scales the props) into the
  // bounding box specified. Note all the button props are scaled.
  virtual void PlaceWidget(double bounds[6]);

  // Description:
  // Provide the necessary methods to satisfy the rendering API.
  virtual void ShallowCopy(vtkProp *prop);
  virtual double *GetBounds();
  virtual void GetActors(vtkPropCollection *pc);
  virtual void ReleaseGraphicsResources(vtkWindow*);
  virtual int RenderOpaqueGeometry(vtkViewport*);
  virtual int RenderVolumetricGeometry(vtkViewport*);
  virtual int RenderTranslucentPolygonalGeometry(vtkViewport*);
  virtual int HasTranslucentPolygonalGeometry();

protected:
  msvVTKProp3DButtonRepresentation();
  ~msvVTKProp3DButtonRepresentation();

  // The current vtkProp3D used to represent the button
  vtkProp3D *CurrentProp;

  // Follow the camera if requested
  vtkProp3DFollower *Follower;
  int FollowCamera;

  // Keep track of the props associated with the N
  // states of the button. This is a PIMPLd stl map.
  vtkPropArray *PropArray;

  // For picking the button
  vtkPropPicker *Picker;

private:
  msvVTKProp3DButtonRepresentation(const msvVTKProp3DButtonRepresentation&);  //Not implemented
  void operator=(const msvVTKProp3DButtonRepresentation&);                    //Not implemented
};

#endif
