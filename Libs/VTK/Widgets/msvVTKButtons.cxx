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

// VTK includes
#include "vtkButtonRepresentation.h"
#include "vtkButtonWidget.h"
#include "vtkCommand.h"
#include "vtkDataSetMapper.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkTexturedButtonRepresentation2D.h"
#include "vtkWindowToImageFilter.h"

// MSV includes
#include "msvVTKAnimate.h"
#include "msvVTKButtons.h"

vtkStandardNewMacro(msvVTKButtons);

//-----------------------------------------------------------------------------
 // Callback respondign to vtkCommand::StateChangedEvent
class vtkButtonCallback : public vtkCommand
{
public:
  vtkTypeMacro(vtkButtonCallback, vtkCommand)
  static vtkButtonCallback *New()
  {
    return new vtkButtonCallback;
  }

  virtual void Execute(vtkObject *caller, unsigned long, void*)
  {
    msvVTKAnimate* animateCamera = new msvVTKAnimate();
    if (this->FlyTo)
      {
      animateCamera->Execute(this->Renderer, Bounds, 200);
      }
    else
      {
      this->Renderer->ResetCamera(this->Bounds);
      }
    if(animateCamera)
      {
      delete animateCamera;
      }
    //selection
  }

  void SetBounds(double b[6])
  {
    this->Bounds[0] = b[0];
    this->Bounds[1] = b[1];
    this->Bounds[2] = b[2];
    this->Bounds[3] = b[3];
    this->Bounds[4] = b[4];
    this->Bounds[5] = b[5];
  }

  void SetFlyTo(bool fly)
  {
    this->FlyTo = fly;
  }

  vtkButtonCallback():ToolButton(NULL), Renderer(0), FlyTo(true) {}
  msvVTKButtons *ToolButton;
  vtkRenderer *Renderer;
  double Bounds[6];
  bool FlyTo;
};

//-----------------------------------------------------------------------------
msvVTKButtons::msvVTKButtons() : msvVTKButtonsInterface()
{
  this->Data = NULL;
  this->Window = NULL;
  this->FlyTo = true;
  this->OnCenter = false;

  this->ButtonCallback = vtkButtonCallback::New();
  vtkButtonCallback::SafeDownCast(this->ButtonCallback)->ToolButton = this;

  this->GetButton()->AddObserver(vtkCommand::StateChangedEvent,
                                 this->ButtonCallback);
}

//-----------------------------------------------------------------------------
msvVTKButtons::~msvVTKButtons()
{
  if (this->Window)
    {
    this->Window->Delete();
    }
}

//-----------------------------------------------------------------------------
void msvVTKButtons::SetBounds(double b[6])
{
  this->Superclass::SetBounds(b);
  vtkButtonCallback::SafeDownCast(this->ButtonCallback)->SetBounds(b);
  //this->Update();
}

//-----------------------------------------------------------------------------
void msvVTKButtons::SetCurrentRenderer(vtkRenderer *renderer)
{
  this->Superclass::SetCurrentRenderer(renderer);
  vtkButtonCallback::SafeDownCast(ButtonCallback)->Renderer = renderer;
}

//-----------------------------------------------------------------------------
vtkImageData* msvVTKButtons::GetPreview(int width, int height)
{
  if (this->Data)
    {
    double bounds[6];
    this->Data->GetBounds(bounds);

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

//-----------------------------------------------------------------------------
void msvVTKButtons::Update()
{
  this->Superclass::Update();
  this->CalculatePosition();
  if (this->ButtonCallback)
    {
    vtkButtonCallback::SafeDownCast(ButtonCallback)->FlyTo = this->FlyTo;
    if (vtkButtonCallback::SafeDownCast(ButtonCallback)->Renderer)
      {
      vtkButtonCallback::SafeDownCast(
        ButtonCallback)->Renderer->GetRenderWindow()->Render();
      }
    }
}

//-----------------------------------------------------------------------------
void msvVTKButtons::CalculatePosition()
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
    vtkTexturedButtonRepresentation2D::SafeDownCast(
    this->GetButton()->GetRepresentation());

  rep->PlaceWidget(coord, size);
  rep->Modified();
  this->GetButton()->SetRepresentation(rep);
}

//-----------------------------------------------------------------------------
vtkRenderWindow* msvVTKButtons::GetWindow()
{
  if (this->Window == NULL)
    {
    this->Window = vtkRenderWindow::New();
    }
  return Window;
}
