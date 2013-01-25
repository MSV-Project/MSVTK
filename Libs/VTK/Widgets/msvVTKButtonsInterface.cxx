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
  Module:    msvVTKButtonsInterface.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkBalloonRepresentation.h"
#include "vtkButtonWidget.h"
#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkProperty2D.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkTextProperty.h"
#include "vtkTexturedButtonRepresentation2D.h"

// MSV includes
#include "msvVTKButtonsInterface.h"

vtkStandardNewMacro(msvVTKButtonsInterface);

//----------------------------------------------------------------------
msvVTKButtonsInterface::msvVTKButtonsInterface()
{
  this->LabelText=NULL;
  this->Tooltip=NULL;
  this->ShowButton=true;
  this->ShowLabel=true;
  this->ButtonWidget=NULL;
  this->Image=NULL;

  // Bounds of the data related to the buttonWin
  double Bounds[6];
  vtkTexturedButtonRepresentation2D* rep = vtkTexturedButtonRepresentation2D::New();
  rep->SetNumberOfStates(1);
  GetButton()->SetRepresentation(rep);
  rep->Delete();

  this->BalloonLayout = vtkBalloonRepresentation::ImageLeft;
  this->Renderer = NULL;
  this->Opacity=1;
  this->PreviousOpacity=1;
}

//----------------------------------------------------------------------
msvVTKButtonsInterface::~msvVTKButtonsInterface()
{
  if (NULL!=this->LabelText)
    {
    delete[] this->LabelText;
    this->LabelText = NULL;
    }
}

//----------------------------------------------------------------------
vtkButtonWidget *msvVTKButtonsInterface::GetButton()
{
  if (this->ButtonWidget == NULL)
    {
    this->ButtonWidget = vtkButtonWidget::New();
    }
  return this->ButtonWidget;
}

//----------------------------------------------------------------------
void msvVTKButtonsInterface::SetCurrentRenderer(vtkRenderer* renderer)
{
  Renderer = renderer;
  if (renderer)
    {
    this->GetButton()->SetInteractor(
      renderer->GetRenderWindow()->GetInteractor());
    this->GetButton()->SetCurrentRenderer(renderer); //to check
    this->GetButton()->EnabledOn();
    }
  else
    {
    this->GetButton()->SetInteractor(NULL);
    this->GetButton()->SetCurrentRenderer(NULL); //to check
    this->GetButton()->EnabledOff();
    }
}

//----------------------------------------------------------------------
void msvVTKButtonsInterface::Update()
{
  vtkTexturedButtonRepresentation2D *rep =
    vtkTexturedButtonRepresentation2D::SafeDownCast(
    this->GetButton()->GetRepresentation());

  if (this->GetShowLabel())
    {
    //Add a label to the button and change its text property
    rep->GetBalloon()->SetBalloonText(this->GetLabelText());
    vtkTextProperty *textProp = rep->GetBalloon()->GetTextProperty();
    rep->GetBalloon()->SetPadding(2);
    textProp->SetFontSize(13);
    textProp->SetFontFamilyToArial();
    textProp->BoldOff();
    textProp->ShadowOn();
    textProp->SetColor(1,1,1);

    //Set label position
    rep->GetBalloon()->SetBalloonLayout(BalloonLayout);
    //This method allows to set the label's backgroun
    rep->GetBalloon()->GetFrameProperty()->SetColor(.5,.5,.5);

    rep->GetBalloon()->GetTextProperty()->SetOpacity(0.3 + Opacity * 0.7);
    rep->GetBalloon()->GetFrameProperty()->SetOpacity(.2 + Opacity * 0.3);
    rep->GetProperty()->SetOpacity(0.3 + Opacity * 0.7);
    rep->Modified();
    }
  else
    {
    rep->GetBalloon()->SetBalloonText("");
    }

  if (this->GetShowButton())
    {
    this->GetButton()->GetRepresentation()->SetVisibility(true);
    this->GetButton()->EnabledOn();
    }
  else
    {
    this->GetButton()->GetRepresentation()->SetVisibility(false);
    this->GetButton()->EnabledOff();
    }
}

//----------------------------------------------------------------------
void msvVTKButtonsInterface::SetImage(vtkImageData* image)
{
  Image = image;
  vtkTexturedButtonRepresentation2D *rep =
    vtkTexturedButtonRepresentation2D::SafeDownCast(
    this->GetButton()->GetRepresentation());
  rep->SetButtonTexture(0,Image);
  int size[2]; size[0] = 16; size[1] = 16;
  rep->GetBalloon()->SetImageSize(size);
  //this->Update();
}

//----------------------------------------------------------------------
void msvVTKButtonsInterface::SetOpacity(double opacity)
{
  if (opacity > 1) opacity=1;
  if (opacity < 0) opacity=0;

  Opacity = opacity;

  this->Update();
}

//----------------------------------------------------------------------
void msvVTKButtonsInterface::RestorePreviousOpacity()
{
  this->SetOpacity(PreviousOpacity);
}

//----------------------------------------------------------------------
void msvVTKButtonsInterface::SetShowButton(bool show)
{
  this->ShowButton = show;
  if (this->Renderer)
    {
    this->Renderer->GetActiveCamera()->Modified();
    }
}
