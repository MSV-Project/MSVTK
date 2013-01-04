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

// .NAME msvVTKButtonsGroup -
// .SECTION Description
//

// .SECTION See Also
//

#ifndef __msvVTKButtonsGroup_h
#define __msvVTKButtonsGroup_h

// MSVTK includes
#include "msvVTKButtonsInterface.h"

// STD includes
#include <vector>

// Forward references
class vtkSliderInteractionCallback;
class vtkSliderStartInteractionCallback;
class vtkSliderWidget;

class msvVTKButtons;

//----------------------------------------------------------------------
class MSV_VTK_WIDGETS_EXPORT msvVTKButtonsGroup : public msvVTKButtonsInterface
{
public:
  // Instantiate the class.
  static msvVTKButtonsGroup *New();

  // Description:
  // Standard methods for instances of the class.
  vtkTypeMacro(msvVTKButtonsGroup, msvVTKButtonsInterface);

  // Add a buttons to the buttons' vector
  void AddElement(msvVTKButtonsInterface* buttons);

  // Remove a buttons to the buttons' vector
  void RemoveElement(msvVTKButtonsInterface* buttons);

  // Get the specified element
  msvVTKButtonsInterface* GetElement(unsigned int index);

  // Create a new element
  msvVTKButtonsGroup* CreateGroup();

  // Create a new element
  msvVTKButtons* CreateButtons();

  // Allow to show/hide button
  void SetShowButton(bool show);

  // Allow to show/hide label
  void SetShowLabel(bool show);

  // set the icon path
  void SetImages(vtkImageData *image);

  // Get the slider widget
  vtkSliderWidget* GetSlider();

  // Show/hide the slider widget
  void ShowSlider(bool show);

  // Get the position on the path at the specified ratio
  void GetCameraPoistionOnPath(double ratio, double b[6]);

  // Set the position on the path at the specified ratio
  void SetCameraPoistionOnPath(double ratio);

  // Get the interaction callback for slicer
  vtkCommand *GetSliderInteractionCallback() const;

  // Get the start interaction callback for slicer
  vtkCommand *GetSliderStartInteractionCallback() const;

  // Set the current renderer
  virtual void SetCurrentRenderer(vtkRenderer *renderer);

  // Perform update
  void Update();

protected:
  // Object constructor
  msvVTKButtonsGroup();

  // Object destructor.
  virtual ~msvVTKButtonsGroup();

  // Calculate position (center or corner)
  void CalculatePosition();

  // Set the specified property
  ///---> void SetElementProperty(QString name, QVariant value);

  // Slider callback function
  vtkSliderInteractionCallback* SliderInteractionCallback;

  // Slider callback function
  vtkSliderStartInteractionCallback* SliderStartInteractionCallback;

  // Vector of elements
  std::vector<msvVTKButtonsInterface*> Elements;

  // Slider widget
  vtkSliderWidget* SliderWidget;

private:
  // Create element template function
  template <class T>
  T *CreateElement();

  msvVTKButtonsGroup(const msvVTKButtonsGroup&);  //Not implemented
  void operator=(const msvVTKButtonsGroup&);  //Not implemented
};

#endif
