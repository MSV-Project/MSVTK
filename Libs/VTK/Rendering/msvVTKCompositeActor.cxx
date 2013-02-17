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

#include "msvVTKCompositeActor.h"
#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataSet.h"
#include "vtkCompositeDataSetInternals.h"
#include "vtkInformation.h"
#include "vtkMapper.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"

#include <vector>
#include <utility>

//-------------------------------------------------------------------------
struct msvVTKCompositeActor::CompositeProperty
{
  typedef std::vector<CompositeProperty>::iterator CompositePropertyIterator;
  vtkProperty* GetProperty(const CompositePropertyIterator& iter);
  vtkProperty* GetProperty(const vtkCompositeDataSetIndex& index);

  vtkSmartPointer<vtkProperty> Property;
  std::vector<CompositeProperty> Children;
};

//-------------------------------------------------------------------------
vtkProperty* msvVTKCompositeActor::CompositeProperty::GetProperty(const CompositePropertyIterator& iter)
{
  // todo
  //if (iter
  return 0;
}

//-------------------------------------------------------------------------
vtkProperty* msvVTKCompositeActor::CompositeProperty::GetProperty(const vtkCompositeDataSetIndex& index)
{
  vtkProperty* prop = this->Property;
  //const unsigned int indexSize = index.size()
  //for (unsigned int i = 0; i < indexSize; ++i)
  //  {
  //  unsigned int childIndex = index[i];
  //  if (!compositeProperty->Children.size() <
  return prop;
}

//-------------------------------------------------------------------------
vtkStandardNewMacro(msvVTKCompositeActor);

//-------------------------------------------------------------------------
msvVTKCompositeActor::msvVTKCompositeActor()
{
  this->CurrentCompositeIndex = 0;
  this->RootProperty = new CompositeProperty;
  this->RootProperty->Property.TakeReference(this->GetProperty());
}

//-------------------------------------------------------------------------
msvVTKCompositeActor::~msvVTKCompositeActor()
{
  delete this->RootProperty;
}

//-------------------------------------------------------------------------
void msvVTKCompositeActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//-------------------------------------------------------------------------
void msvVTKCompositeActor::SetCurrentCompositeIndex(vtkCompositeDataIterator* iter)
{
  this->CurrentCompositeIndex = iter->GetCurrentFlatIndex();
  this->SetProperty(this->GetCompositeProperty(iter));
}

//-------------------------------------------------------------------------
vtkProperty* msvVTKCompositeActor::GetCompositeProperty(vtkCompositeDataIterator* iter)
{
  return 0;
}

//-------------------------------------------------------------------------
void msvVTKCompositeActor
::SetCompositeProperty(vtkCompositeDataIterator* iter,
                       vtkProperty* prop)
{
}
