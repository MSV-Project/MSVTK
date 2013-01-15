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

#include "vtkBalloonRepresentation.h"
#include "vtkButtonRepresentation.h"
#include "vtkButtonWidget.h"
#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkDataSetMapper.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkTextProperty.h"
#include "vtkTexturedButtonRepresentation2D.h"
#include "vtkWindowToImageFilter.h"

// MSV includes
#include "msvVTKAnimate.h"
#include "msvVTKButtons.h"

vtkStandardNewMacro(msvVTKButtons);

//------------------------------------------------------------------------------
 // Callback respondign to vtkCommand::StateChangedEvent
class vtkButtonCallback : public vtkCommand
{
public:
  static vtkButtonCallback *New()
  {
    return new vtkButtonCallback;
  }

  virtual void Execute(vtkObject *caller, unsigned long, void*)
  {
    msvVTKAnimate* animateCamera = new msvVTKAnimate();
    if (FlyTo)
    {
      animateCamera->Execute(Renderer, Bounds, 200);
    }
    else
    {
      Renderer->ResetCamera(Bounds);
    }
    if(animateCamera)
    {
      delete animateCamera;
    }
    //selection
  }

  void SetBounds(double b[6])
  {
    Bounds[0] = b[0];
    Bounds[1] = b[1];
    Bounds[2] = b[2];
    Bounds[3] = b[3];
    Bounds[4] = b[4];
    Bounds[5] = b[5];
  }

  void SetFlyTo(bool fly)
  {
    FlyTo = fly;
  }

  vtkButtonCallback():ToolButton(NULL), Renderer(0), FlyTo(true) {}
  msvVTKButtons *ToolButton;
  vtkRenderer *Renderer;
  double Bounds[6];
  bool FlyTo;
};


//----------------------------------------------------------------------
msvVTKButtons::msvVTKButtons() : msvVTKButtonsInterface()
{
  this->Data=NULL;
  this->Window=NULL;
  this->FlyTo=true;
  this->OnCenter=false;

  this->ButtonCallback = vtkButtonCallback::New();
  reinterpret_cast<vtkButtonCallback*>(this->ButtonCallback)->ToolButton = this;

  //this->RWICallback = vtkRWICallback::New();
  //reinterpret_cast<vtkRWICallback*>(this->RWICallback)->ToolButton = this;

  this->GetButton()->AddObserver(vtkCommand::StateChangedEvent,this->ButtonCallback);

  this->OnCorner = false;
  this->CornerIndex = -1;
}

//----------------------------------------------------------------------
msvVTKButtons::~msvVTKButtons()
{
  if(this->Window)
  {
    this->Window->Delete();
  }
}

//----------------------------------------------------------------------
void msvVTKButtons::SetBounds(double b[6])
{
  msvVTKButtonsInterface::SetBounds(b);
  reinterpret_cast<vtkButtonCallback*>(ButtonCallback)->SetBounds(b);
  //reinterpret_cast<vtkRWICallback*>(RWICallback)->SetBounds(b);
  //this->Update();
}

//----------------------------------------------------------------------
void msvVTKButtons::SetCurrentRenderer(vtkRenderer *renderer)
{
  msvVTKButtonsInterface::SetCurrentRenderer(renderer);
  if(renderer)
  {
    //renderer->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent,RWICallback);
    //reinterpret_cast<vtkRWICallback*>(RWICallback)->Renderer = renderer;
    reinterpret_cast<vtkButtonCallback*>(ButtonCallback)->Renderer = renderer;
  }
  else
  {
    //reinterpret_cast<vtkRWICallback*>(RWICallback)->Renderer = NULL;
    reinterpret_cast<vtkButtonCallback*>(ButtonCallback)->Renderer = NULL;
  }
}

//----------------------------------------------------------------------
vtkImageData* msvVTKButtons::GetPreview(int width, int height)
{
  if(Data)
  {
    double bounds[6];
    Data->GetBounds(bounds);

    // create pipe
    vtkWindowToImageFilter *previewer = vtkWindowToImageFilter::New();
    vtkDataSetMapper *mapper = vtkDataSetMapper::New();
    vtkActor *actor = vtkActor::New();
    vtkRenderer *renderer = vtkRenderer::New();

    mapper->SetInput(Data);
    mapper->Update();
    actor->SetMapper(mapper);

    // offscreen rendering
    this->GetWindow()->OffScreenRenderingOn();
    this->GetWindow()->AddRenderer(renderer);
    this->GetWindow()->Start();
    this->GetWindow()->SetSize(width,height);
    renderer->AddActor(actor);
    renderer->ResetCamera(bounds);

    // Extract the image from the 'hidden' renderer
    previewer->SetInput(Window);
    previewer->Modified();
    previewer->Update();

    // Convert image data to QImage and return
    vtkImageData* vtkImage = vtkImageData::New();
    vtkImage->DeepCopy( previewer->GetOutput() );
    vtkImage->Modified();
    vtkImage->Update();

    previewer->SetInput(NULL);
    previewer->Delete();

    mapper->SetInput(NULL);
    mapper->Delete();

    actor->SetMapper(NULL);
    actor->Delete();

    //this->GetWindow()->RemoveRenderer(renderer);
    //destroy pipe
    renderer->Delete();

    //this->GetWindow()->Delete();

    return vtkImage;
  }
  return NULL;
}

//------------------------------------------------------------------------------
void msvVTKButtons::Update()
{
  msvVTKButtonsInterface::Update();
  this->CalculatePosition();
  if(ButtonCallback)
  {
    reinterpret_cast<vtkButtonCallback*>(ButtonCallback)->FlyTo = FlyTo;
    if(reinterpret_cast<vtkButtonCallback*>(ButtonCallback)->Renderer)
    {
      Renderer->GetRenderWindow()->Render();
    }
  }
}

//------------------------------------------------------------------------------
void msvVTKButtons::CalculatePosition()
{
  if(!this->GetOnCorner())
  {
    //modify position of the vtkButton
    double bds[6];
    double coord[3];
    this->GetBounds(bds);
    if (this->OnCenter)
    {
      coord[0] = (bds[1] + bds[0])*.5;
      coord[1] = (bds[3] + bds[2])*.5;
      coord[2] = (bds[5] + bds[4])*.5;
    }
    else
    {
      //on the corner of the bounding box of the VME.
      coord[0] = bds[0];
      coord[1] = bds[2];
      coord[2] = bds[4];
    }
    int size[2]; size[0] = 16; size[1] = 16;
    vtkTexturedButtonRepresentation2D *rep =
        static_cast<vtkTexturedButtonRepresentation2D*>(
          this->GetButton()->GetRepresentation());

    rep->PlaceWidget(coord, size);
    vtkTextProperty *textProp = rep->GetBalloon()->GetTextProperty();
    //textProp->BoldOff();
    rep->Modified();
    this->GetButton()->SetRepresentation(rep);
  }
  else
  {
    double bds[6];
    int *RWSize;
    RWSize = Renderer->GetSize();

    bds[0] = 96;
    bds[1] = 16;
    bds[2] = (22 * (CornerIndex - 1)) + CornerIndex;
    bds[3] = 16;
    bds[4] = 0;
    bds[5] = 0;

    vtkTexturedButtonRepresentation2D *rep = static_cast<vtkTexturedButtonRepresentation2D *>(this->GetButton()->GetRepresentation());
    vtkTextProperty *textProp = rep->GetBalloon()->GetTextProperty();
    //textProp->BoldOn();
    rep->PlaceWidget(bds);
    rep->Modified();
    this->GetButton()->SetRepresentation(rep);
  }
}

//------------------------------------------------------------------------------
vtkRenderWindow* msvVTKButtons::GetWindow()
{
  if(Window == NULL)
  {
    Window = vtkRenderWindow::New();
  }
  return Window;
}
