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
  Module:    msvVTKButtons.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkRenderer.h"

// MSVTK includes
#include "msvVTKButtons.h"
#include "msvVTKButtonsInterface.h"
#include "msvVTKButtonsManager.h"

//------------------------------------------------------------------------------
 // Callback respondign to vtkCommand::ModifiedEvent
class vtkCameraCallback : public vtkCommand
{
public:
  static vtkCameraCallback *New()
  {
    return new vtkCameraCallback;
  }

  virtual void Execute(vtkObject *caller, unsigned long, void*)
  {
    int onCornerCount = 0;
    if(Renderer && Renderer->GetActiveCamera())
    {
      for(int i = 0; i < msvVTKButtonsManager::GetInstance()->GetNumberOfElements(); i++)
      {
        msvVTKButtons* toolButton = msvVTKButtons::SafeDownCast(msvVTKButtonsManager::GetInstance()->GetElement(i));
        if(toolButton)
        {
          // expands bounds
          double bounds[6];
          double extBounds[6];
          toolButton->GetBounds(bounds);

          for(int i=0; i< 5; i+=2)
          {
            extBounds[i] = bounds[i] - (bounds[i+1] - bounds[i])/2.;
            extBounds[i+1] = bounds[i+1] + (bounds[i+1] - bounds[i])/2.;
          }

          double cameraPosition[3];
          Renderer->GetActiveCamera()->GetPosition(cameraPosition);
          if(extBounds[0] < cameraPosition[0] && cameraPosition[0] < extBounds[1] &&
             extBounds[2] < cameraPosition[1] && cameraPosition[1] < extBounds[3] &&
             extBounds[4] < cameraPosition[2] && cameraPosition[2] < extBounds[5])
          {
            if(toolButton->GetShowButton() == true)
            {
              onCornerCount++;
              toolButton->SetOnCorner(true);
              toolButton->SetCornerIndex(onCornerCount);
              toolButton->CalculatePosition();
            }
          }
          else //if(toolButton->GetOnCorner() == true)
          {
            toolButton->SetOnCorner(false);
            toolButton->CalculatePosition();
          }
        }
      }
    }
  }

  vtkCameraCallback(): Renderer(0) {}
  vtkRenderer *Renderer;
};

//------------------------------------------------------------------------------
msvVTKButtonsManager::msvVTKButtonsManager()
{
  this->CameraCallback = NULL;
}

//------------------------------------------------------------------------------
msvVTKButtonsManager::~msvVTKButtonsManager()
{

}

//------------------------------------------------------------------------------
msvVTKButtonsGroup *msvVTKButtonsManager::CreateGroup()
{
  Elements.push_back(msvVTKButtonsGroup::New());
  return static_cast<msvVTKButtonsGroup*>(Elements.at(Elements.size()-1));
}

//------------------------------------------------------------------------------
msvVTKButtons *msvVTKButtonsManager::CreateButtons()
{
  Elements.push_back(msvVTKButtons::New());
  return static_cast<msvVTKButtons*>(Elements.at(Elements.size()-1));
}

//------------------------------------------------------------------------------
void msvVTKButtonsManager::AddElement(msvVTKButtonsInterface *element)
{
  // add control to determine if element is already inisde the vector

  // @ToDo change the way the manager retrives the renderer
  /*msvVTKButtons *button = reinterpret_cast<msvVTKButtons*>(element);
  if(button && !this->CameraCallback)
  {
    this->CameraCallback = vtkCameraCallback::New();
    button->GetRenderer()->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent,CameraCallback);
    reinterpret_cast<vtkCameraCallback*>(CameraCallback)->Renderer = button->GetRenderer();
  }*/
  Elements.push_back(element);
}

//------------------------------------------------------------------------------
msvVTKButtonsInterface * msvVTKButtonsManager::GetElement(int index)
{
  // add index control

  return Elements.at(index);
}

//------------------------------------------------------------------------------
msvVTKButtonsManager* msvVTKButtonsManager::GetInstance()
{
  static msvVTKButtonsManager manager;
  return &manager;
}

//------------------------------------------------------------------------------
void msvVTKButtonsManager::SetRenderer(vtkRenderer *renderer)
{
  this->CameraCallback = vtkCameraCallback::New();
  renderer->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent,CameraCallback);
  reinterpret_cast<vtkCameraCallback*>(CameraCallback)->Renderer = renderer;
}
