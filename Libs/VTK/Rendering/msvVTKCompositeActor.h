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
// .NAME msvVTKCompositeActor
// .SECTION Description

#ifndef __msvVTKCompositeActor_h
#define __msvVTKCompositeActor_h

// VTK includes
#include "vtkActor.h"
#include "msvVTKRenderingExport.h"
class vtkCompositeDataIterator;
class vtkCompositeDataIndex;

class MSV_VTK_RENDERING_EXPORT msvVTKCompositeActor
  : public vtkActor
{
public:
  // Description:
  // Method to instantiate class.
  static msvVTKCompositeActor *New();

  // Description;
  // Standard methods for class.
  vtkTypeMacro(msvVTKCompositeActor,vtkActor);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  //
  void SetCompositeProperty(vtkCompositeDataIterator* iter, vtkProperty* prop);
  vtkProperty* GetCompositeProperty(vtkCompositeDataIterator* iter);

  void SetCurrentCompositeIndex(vtkCompositeDataIterator* iter);

protected:
  msvVTKCompositeActor();
  ~msvVTKCompositeActor();

  unsigned int CurrentCompositeIndex;
  struct CompositeProperty;
  CompositeProperty* RootProperty;

private:
  msvVTKCompositeActor(const msvVTKCompositeActor&);  //Not implemented
  void operator=(const msvVTKCompositeActor&);  //Not implemented
};

#endif
