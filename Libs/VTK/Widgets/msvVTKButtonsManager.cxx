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
#include "vtkMath.h"
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
      double cameraPosition[3];
      Renderer->GetActiveCamera()->GetPosition(cameraPosition);
      int *intRendererSize;
      intRendererSize= Renderer->GetSize();
      double rendererSize[2];
      rendererSize[0] = static_cast<double>(intRendererSize[0]);
      rendererSize[1] = static_cast<double>(intRendererSize[1]);

      for(int i = 0; i < msvVTKButtonsManager::GetInstance()->GetNumberOfElements(); i++)
      {
        msvVTKButtons* toolButton = msvVTKButtons::SafeDownCast(msvVTKButtonsManager::GetInstance()->GetElement(i));
        if(toolButton && toolButton->GetShowButton())
        {
          double pos[2];
          toolButton->GetRealDisplayPosition(pos);

          // expands bounds
          double bounds[6];
          double extBounds[6];
          toolButton->GetBounds(bounds);

          for(int i=0; i< 5; i+=2)
          {
            extBounds[i] = bounds[i] - (bounds[i+1] - bounds[i])/2.;
            extBounds[i+1] = bounds[i+1] + (bounds[i+1] - bounds[i])/2.;
          }
          double opacity = 1;

          double avgDistance = ((extBounds[1] - extBounds[0]) + (extBounds[3] - extBounds[2]) + (extBounds[5] - extBounds[4]))/6.;
          double bbCenter[3];
          bbCenter[0] = bounds[0] + (bounds[1]-bounds[0])/2.;
          bbCenter[1] = bounds[2] + (bounds[3]-bounds[2])/2.;
          bbCenter[2] = bounds[4] + (bounds[5]-bounds[4])/2.;

          double distance = sqrt(vtkMath::Distance2BetweenPoints(bbCenter,cameraPosition));
          if((extBounds[0] < cameraPosition[0] && cameraPosition[0] < extBounds[1] &&
             extBounds[2] < cameraPosition[1] && cameraPosition[1] < extBounds[3] &&
             extBounds[4] < cameraPosition[2] && cameraPosition[2] < extBounds[5]) ||
             pos[0] >= rendererSize[0] || pos[1] >= rendererSize[1] ||
             pos[0] <= 0 || pos[1] <= 0)
          {

            if(toolButton->GetShowButton() == true)
            {
              // opacity = 1;
              onCornerCount++;
              toolButton->SetOnCorner(true);
              toolButton->SetCornerIndex(onCornerCount);
              toolButton->CalculatePosition();
              opacity = distance / avgDistance;
            }
          }
          else //if(toolButton->GetOnCorner() == true)
          {
            toolButton->SetOnCorner(false);
            toolButton->CalculatePosition();
            opacity = 10 * avgDistance / distance;
            //opacity = 0;//avgDistance - distance / (3*avgDistance) ;
            this->MoveOverlappingButtons();
          }
          toolButton->SetOpacity(1-opacity);


        }
      }
    }
  }

  void MoveOverlappingButtons()
  {
    double xTolerance = 128;
    double yTolerance = 24;
    // Sort buttons along y axis
    std::vector<msvVTKButtons*> sortedElements;
    for(int i = 0; i < msvVTKButtonsManager::GetInstance()->GetNumberOfElements(); i++)
    {
      msvVTKButtons* toolButton = msvVTKButtons::SafeDownCast(msvVTKButtonsManager::GetInstance()->GetElement(i));
      if(toolButton && toolButton->GetShowButton())
      {
        double pos[2];

        toolButton->GetDisplayPosition(pos);
        toolButton->SetYOffset(0);
        bool added = false;

        for(std::vector<msvVTKButtons*>::iterator it = sortedElements.begin(); it != sortedElements.end(); it++)
        {
          double curPos[2];
          (*it)->GetDisplayPosition(curPos);
          if(pos[1] < curPos[1])
          {
            sortedElements.insert(it,toolButton);
            added = true;
            break;
          }
        }
        if(!added)
        {
          sortedElements.push_back(toolButton);
        }
      }
    }

    for(std::vector<msvVTKButtons*>::iterator it = sortedElements.begin()+1; it != sortedElements.end(); it++)
    {
      for(std::vector<msvVTKButtons*>::iterator it2 = sortedElements.begin(); it2 != it; it2++)
      {
        double pos[2];
        (*it)->GetDisplayPosition(pos);
        double prevPos[2];
        (*it2)->GetDisplayPosition(prevPos);
        if(pos[0] >= prevPos[0] - xTolerance && pos[0] <= prevPos[0] + xTolerance &&
           pos[1] >= prevPos[1] - yTolerance && pos[1] <= prevPos[1] + yTolerance)
        {
          (*it)->SetYOffset((*it)->GetYOffset() + yTolerance - (pos[1] - prevPos[1]));
        }
      }
    }
//    for(std::vector<msvVTKButtons*>::iterator it = sortedElements.begin(); it != sortedElements.end(); it++)
//    {
//      (*it)->Update();
//    }
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
