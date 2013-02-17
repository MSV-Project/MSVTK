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
// .NAME msvVTKLODWidget - Select LOD of a piece
// .SECTION Description

#ifndef __msvVTKLODWidget_h
#define __msvVTKLODWidget_h

#include "vtkAbstractWidget.h"
#include "msvVTKWidgetsExport.h"

class MSV_VTK_WIDGETS_EXPORT msvVTKLODWidget : public vtkAbstractWidget
{
public:
  // Description:
  // Method to instantiate class.
  static msvVTKLODWidget *New();

  // Description;
  // Standard methods for class.
  vtkTypeMacro(msvVTKLODWidget,vtkAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create the default widget representation if one is not set.
  virtual void CreateDefaultRepresentation();

protected:
  msvVTKLODWidget();
  ~msvVTKLODWidget();

  // Description:
  // Subclasses generally implement this method. The SelectRegion() method
  // offers a subclass the chance to do something special if the interior
  // of the widget is selected.
  virtual void SelectRegion(double eventPos[2]);

  //processes the registered events
  static void SelectAction(vtkAbstractWidget*);
  static void EndSelectAction(vtkAbstractWidget*);
  static void MouseHoverAction(vtkAbstractWidget *w);

  // Special internal methods to support subclasses handling events.
  // If a non-zero value is returned, the subclass is handling the event.
  virtual int SubclassSelectAction() {return 0;}
  virtual int SubclassEndSelectAction() {return 0;}
  virtual int SubclassMouseHoverAction() {return 0;}

  // helper methods for cursoe management
  virtual void SetCursor(int State);

//BTX
  //widget state
  int WidgetState;
  enum _WidgetState{Start=0, Selectable, Selected};
//ETX
  int SelectX;
  int SelectY;

private:
  msvVTKLODWidget(const msvVTKLODWidget&);  //Not implemented
  void operator=(const msvVTKLODWidget&);  //Not implemented
};

#endif
