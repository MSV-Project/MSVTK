/*==============================================================================

  Library: MSVTK

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

#ifndef __msvVTKECGButtonsManager_h
#define __msvVTKECGButtonsManager_h

// VTK includes
#include "vtkObject.h"
#include "vtkSmartPointer.h"

// ECG includes
#include "msvECGExport.h"

class vtkPolyData;
class vtkRenderer;

class MSV_ECG_EXPORT msvVTKECGButtonsManager : public vtkObject
{
public:
  static msvVTKECGButtonsManager* New();
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set / Get the Renderer to process widget
  vtkSetMacro(Renderer,vtkRenderer*);
  vtkGetMacro(Renderer,vtkRenderer*);

  // Description:
  // Set / Get the size of the button reprensentation
  void SetButtonWidgetSize(double size) {this->ButtonWidgetSize = size;}
  vtkGetMacro(ButtonWidgetSize,double);

  // Description:
  // Set / get the number of maximum buttons that the manager can handle.
  vtkSetMacro(MaxNumberOfButtonWidgets,int);
  vtkGetMacro(MaxNumberOfButtonWidgets,int);

  // Description:
  // Set / Get the number of widgetButtons in our scene.
  void SetNumberOfButtonWidgets(int);
  vtkGetMacro(NumberOfButtonWidgets,int);

  // Description:
  // Set / get the last selected buttonWidget
  void SetLastSelectedButton(vtkIdType);
  vtkIdType GetLastSelectedButton() const;
  int GetIndexFromButtonId(vtkIdType) const;

  /// Callback using to process the widgets events
  static void ProcessWidgetsEvents(vtkObject *caller,
                                   unsigned long event,
                                   void *clientData,
                                   void *callData);

  virtual void Init(vtkPolyData* points);         // Initialize vtkButtonsWidget
  virtual void Clear();                           // Clear Buttons Manager
  virtual void UpdateButtonWidgets(vtkPolyData*); // Update from the vtkPolyData

protected:
  msvVTKECGButtonsManager();
  virtual ~msvVTKECGButtonsManager();

  int           NumberOfButtonWidgets;
  int           MaxNumberOfButtonWidgets;
  double        ButtonWidgetSize;
  vtkRenderer*  Renderer;

private:
  msvVTKECGButtonsManager(const msvVTKECGButtonsManager&);  // Not implemented.
  void operator=(const msvVTKECGButtonsManager&);           // Not implemented.

  class vtkInternal;
  vtkInternal* Internal;
};
#endif
