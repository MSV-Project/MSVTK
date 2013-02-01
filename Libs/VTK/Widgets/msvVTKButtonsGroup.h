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
// .NAME msvVTKButtonsGroup -
// .SECTION Description
//

// .SECTION See Also
//

#include "vtkObject.h"

#include <vector>

// VTK_WIDGET includes
#include "msvVTKButtonsInterface.h"

// Forward references
class vtkSliderInteractionCallback;
class vtkSliderStartInteractionCallback;
class vtkSliderWidget;

class msvVTKButtons;

#ifndef __msvVTKButtonsGroup_h
#define __msvVTKButtonsGroup_h

//----------------------------------------------------------------------
class MSV_VTK_WIDGETS_EXPORT msvVTKButtonsGroup : public msvVTKButtonsInterface
{
public:
  // Description:
  // Instantiate the class.
  static msvVTKButtonsGroup *New();

  vtkTypeMacro(msvVTKButtonsGroup,msvVTKButtonsInterface);

  // Description:
  // Add a buttons to the buttons' vector
  void AddElement(msvVTKButtonsInterface* buttons);

  // Description:
  // Remove a buttons to the buttons' vector
  void RemoveElement(msvVTKButtonsInterface* buttons);

  // Description:
  // Get the specified element
  msvVTKButtonsInterface* GetElement(unsigned int index);

  // Description:
  // Create a new element
  msvVTKButtonsGroup* CreateGroup();

  // Description:
  // Create a new element
  msvVTKButtons* CreateButtons();

  // Description:
  // Allow to show/hide button
  void SetShowButtons(bool show);

  // Description:
  // Allow to show/hide label
  void SetShowLabel(bool show);

  // Description:
  // set the elements image
  void SetImageToElements(vtkImageData *image);

  // Description:
  // Get the slider widget
  vtkSliderWidget* GetSlider();

  // Description:
  // Show/hide the slider widget
  void ShowSlider(bool show);

  // Description:
  // Get the position on the path at the specified ratio
  void GetCameraPositionOnPath(double ratio, double b[6]);

  // Description:
  // Set the position on the path at the specified ratio
  void SetCameraPositionOnPath(double ratio);

  // Description:
  // Get the interaction callback for slicer
  vtkCommand *GetSliderInteractionCallback() const;

  // Description:
  // Get the start interaction callback for slicer
  vtkCommand *GetSliderStartInteractionCallback() const;

  // Description:
  // Set the current renderer
  void SetCurrentRenderer(vtkRenderer *renderer);

  // Description:
  // Perform update
  void Update();

  // Description:
  // Calculate position (center or corner)
  void CalculatePosition();

protected:
  // Object constructor
  msvVTKButtonsGroup();

  // Object destructor.
  virtual ~msvVTKButtonsGroup();

  // Slider callback function
  vtkSliderInteractionCallback* SliderInteractionCallback;

  // Slider callback function
  vtkSliderStartInteractionCallback* SliderStartInteractionCallback;

  // Vector of elements
  std::vector<msvVTKButtonsInterface*> Elements;

  // Slider widget
  vtkSliderWidget* SliderWidget;

private:
  // Description:
  // Create element template function
  template <class T>
  T *CreateElement();

  msvVTKButtonsGroup(const msvVTKButtonsGroup&);  //Not implemented
  void operator=(const msvVTKButtonsGroup&);  //Not implemented
};

#endif
