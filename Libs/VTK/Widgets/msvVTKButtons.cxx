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
#include "vtkCoordinate.h"
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
    ToolButton->SetPreviousOpacity(0);
    msvVTKAnimate* animateCamera = new msvVTKAnimate();
    if (FlyTo)
    {
      animateCamera->Execute(Renderer, Bounds, 100);
    }
    else
    {
      Renderer->ResetCamera(Bounds);
    }
    if (animateCamera)
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

  this->YOffset = 0;
}

//----------------------------------------------------------------------
msvVTKButtons::~msvVTKButtons()
{
  this->DeleteWindow();
}

//----------------------------------------------------------------------
void msvVTKButtons::SetBounds(double b[6])
{
  Superclass::SetBounds(b);
  reinterpret_cast<vtkButtonCallback*>(ButtonCallback)->SetBounds(b);
  //reinterpret_cast<vtkRWICallback*>(RWICallback)->SetBounds(b);
  //this->Update();
}

//----------------------------------------------------------------------
void msvVTKButtons::SetCurrentRenderer(vtkRenderer *renderer)
{
  Superclass::SetCurrentRenderer(renderer);
  if (renderer)
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
  if (Data)
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
  Superclass::Update();
  this->CalculatePosition();
  if (ButtonCallback)
  {
    reinterpret_cast<vtkButtonCallback*>(ButtonCallback)->FlyTo = FlyTo;
    if (reinterpret_cast<vtkButtonCallback*>(ButtonCallback)->Renderer)
    {
      Renderer->GetRenderWindow()->Render();
    }
  }
}

//------------------------------------------------------------------------------
void msvVTKButtons::CalculatePosition()
{
  double pos[2];
  double bds[6];
  this->GetDisplayPosition(pos);
  bds[0] = pos[0];
  bds[1] = 16;
  bds[2] = pos[1];
  bds[3] = 16;
  bds[4] = 0.0;
  bds[5] = 0.0;
  vtkTexturedButtonRepresentation2D *rep = vtkTexturedButtonRepresentation2D::SafeDownCast(this->GetButton()->GetRepresentation());
  rep->SetPlaceFactor(1.);
  rep->PlaceWidget(bds);
  rep->Modified();
  rep->GetBalloon()->SetImageSize(16,16);
  this->GetButton()->SetRepresentation(rep);
  this->GetButton()->Modified();

}

//------------------------------------------------------------------------------
vtkRenderWindow* msvVTKButtons::GetWindow()
{
  if (Window == NULL)
  {
    Window = vtkRenderWindow::New();
  }
  return Window;
}

//------------------------------------------------------------------------------
void msvVTKButtons::DeleteWindow()
{
  if (this->Window)
  {
    this->Window->Delete();
    this->Window = NULL;
  }
}

//------------------------------------------------------------------------------
void msvVTKButtons::GetDisplayPosition(double pos[2])
{
  if (!this->GetOnCorner())
  {
    this->GetRealDisplayPosition(pos);
  }
  else
  {
    pos[0] = 96;
    pos[1] = (24 * (CornerIndex - 1)) + CornerIndex;
  }
}

//------------------------------------------------------------------------------
void msvVTKButtons::GetRealDisplayPosition(double pos[2])
{
  //modify position of the vtkButton
  double bounds[6];
  double coord[3];
  this->GetBounds(bounds);
  if (this->OnCenter)
  {
    coord[0] = (bounds[1] + bounds[0])*.5;
    coord[1] = (bounds[3] + bounds[2])*.5;
    coord[2] = (bounds[5] + bounds[4])*.5;
  }
  else
  {
  //on the corner of the bounding box of the VME.
  coord[0] = bounds[0];
  coord[1] = bounds[2];
  coord[2] = bounds[4];
  }

  vtkCoordinate* anchor = vtkCoordinate::New();
  anchor->SetCoordinateSystemToWorld();
  anchor->SetValue(coord);

  double *p = anchor->GetComputedDoubleDisplayValue(this->Renderer);
  pos[0] = static_cast<double>(p[0]);
  pos[1] = static_cast<double>(p[1]) + this->YOffset;
  if (pos[0]<0 || pos[1]<0)
  {
    pos[0] = 0;
    pos[1] = 0;
  }
  anchor->Delete();
}
