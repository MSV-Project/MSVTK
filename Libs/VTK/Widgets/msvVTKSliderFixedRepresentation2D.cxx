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
  Module:    msvVTKSliderFixedRepresentation2D.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "msvVTKSliderFixedRepresentation2D.h"
#include "vtkCommand.h"
#include "vtkObjectFactory.h"
#include "vtkActor2D.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkProperty2D.h"
#include "vtkRenderer.h"
#include "vtkMath.h"
#include "vtkLine.h"
#include "vtkEvent.h"
#include "vtkInteractorObserver.h"
#include "vtkWindow.h"
#include "vtkPolyData.h"
#include "vtkTextProperty.h"
#include "vtkTextMapper.h"
#include "vtkTextActor.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"

vtkStandardNewMacro(msvVTKSliderFixedRepresentation2D);

//----------------------------------------------------------------------
msvVTKSliderFixedRepresentation2D::msvVTKSliderFixedRepresentation2D()
{
  Scale[0] = Scale[1] = 100;
  Translate[0] = Translate[1] = 0;
}

//----------------------------------------------------------------------
msvVTKSliderFixedRepresentation2D::~msvVTKSliderFixedRepresentation2D()
{

}

//----------------------------------------------------------------------
void msvVTKSliderFixedRepresentation2D::BuildRepresentation()
{
  if ( this->GetMTime() > this->BuildTime || (this->Renderer && this->Renderer->GetVTKWindow() &&
    this->Renderer->GetVTKWindow()->GetMTime() > this->BuildTime) )
  {
    int *size = this->Renderer->GetSize();
    if (0 == size[0] || 0 == size[1])
    {
      // Renderer has no size yet: wait until the next
      // BuildRepresentation...
      return;
    }

    double t = (this->Value-this->MinimumValue) / (this->MaximumValue-this->MinimumValue);

    // Setup the geometry of the widget (canonical along the x-axis).
    // Later we will transform the widget into place. We take into account the length of
    // the widget here.
    int *p1 = this->Point1Coordinate->GetComputedDisplayValue(this->Renderer);
    //int *p2 = this->Point2Coordinate->GetComputedDisplayValue(this->Renderer);
    int p2[2];
    //p1[1] = p1[1]+30;
    p2[0] = p1[0];
    p2[1] = p1[1]+1;

    double delX = static_cast<double>(p2[0]-p1[0]);
    double delY = static_cast<double>(p2[1]-p1[1]);
    double length = sqrt ( delX*delX + delY*delY );
    length = (length <= 0.0 ? 1.0 : length);
    this->X = 0.5;
    double theta = atan2(delY,delX);

    // Generate the points
    double x[6], y[6];
    x[0] = -this->X;
    x[1] = -this->X + this->EndCapLength;
    x[2] = x[1] + t*(2.0*X - 2.0*this->EndCapLength - this->SliderLength);
    x[3] = x[2] + this->SliderLength;
    x[4] = this->X - this->EndCapLength;
    x[5] = this->X;

    y[0] = -0.5*this->EndCapWidth;
    y[1] = -0.5*this->SliderWidth;
    y[2] = -0.5*this->TubeWidth;
    y[3] =  0.5*this->TubeWidth;
    y[4] =  0.5*this->SliderWidth;
    y[5] =  0.5*this->EndCapWidth;

    this->Points->SetPoint(0, x[0],y[0],0.0);
    this->Points->SetPoint(1, x[1],y[0],0.0);
    this->Points->SetPoint(2, x[1],y[5],0.0);
    this->Points->SetPoint(3, x[0],y[5],0.0);
    this->Points->SetPoint(4, x[1],y[2],0.0);
    this->Points->SetPoint(5, x[4],y[2],0.0);
    this->Points->SetPoint(6, x[4],y[3],0.0);
    this->Points->SetPoint(7, x[1],y[3],0.0);
    this->Points->SetPoint(8, x[2],y[1],0.0);
    this->Points->SetPoint(9, x[3],y[1],0.0);
    this->Points->SetPoint(10, x[3],y[4],0.0);
    this->Points->SetPoint(11, x[2],y[4],0.0);
    this->Points->SetPoint(12, x[4],y[0],0.0);
    this->Points->SetPoint(13, x[5],y[0],0.0);
    this->Points->SetPoint(14, x[5],y[5],0.0);
    this->Points->SetPoint(15, x[4],y[5],0.0);

    // Specify the location of the text. Because the slider can rotate
    // we have to take into account the text height and width.
    int titleSize[2];
    double textSize[2];
    double maxY = (this->SliderWidth > this->TubeWidth ?
      (this->SliderWidth > this->EndCapWidth ? this->SliderWidth : this->EndCapWidth) :
      (this->TubeWidth > this->EndCapWidth ? this->TubeWidth : this->EndCapWidth) );

    if ( ! this->ShowSliderLabel )
    {
      this->LabelActor->VisibilityOff();
    }
    else
    {
      this->LabelActor->VisibilityOn();
      int labelSize[2];
      char label[256];
      sprintf(label, this->LabelFormat, this->Value);
      this->LabelMapper->SetInput(label);
      this->LabelProperty->SetFontSize(10);
      this->LabelMapper->GetSize(this->Renderer, labelSize);
      textSize[0] = static_cast<double>(labelSize[0])/static_cast<double>(400);
      textSize[1] = static_cast<double>(labelSize[1])/static_cast<double>(400);
      double radius = maxY/2.0 + textSize[1]*cos(theta) + textSize[0]*sin(theta);
      this->Points->SetPoint(16, (x[2]+x[3])/2.0, radius, 0.0); //label
    }

    this->TitleProperty->SetFontSize(static_cast<int>(10));
    this->TitleMapper->GetSize(this->Renderer, titleSize);
    textSize[0] = static_cast<double>(titleSize[0])/static_cast<double>(400);
    textSize[1] = static_cast<double>(titleSize[1])/static_cast<double>(400);
    double radius = maxY/2.0 + textSize[1]*cos(theta) + textSize[0]*sin(theta);
    this->Points->SetPoint(17, 0.0,-radius,0.0); //title

    double tx = static_cast<double>((p1[0]+p2[0])/2.0);
    double ty = static_cast<double>((p1[1]+p2[1])/2.0);

    // fixed transform
    // @ToDo Expose API to translate and scale
    this->XForm->Identity();
    this->XForm->Translate(Translate[0],Translate[1],0.0);
    this->XForm->Scale(Scale[0],Scale[1],1.0);
    this->XForm->RotateZ( vtkMath::DegreesFromRadians( theta ) );

    // The transform has done the work of finding the center point for the text.
    // Put the title and label at these points.
    double p16[3], p17[3];
    this->SliderXForm->Update(); //want to get the points that were transformed
    this->SliderXForm->GetOutput()->GetPoints()->GetPoint(16,p16);
    this->SliderXForm->GetOutput()->GetPoints()->GetPoint(17,p17);
    this->LabelActor->SetPosition(p17[0]+12,p17[1]);
    this->TitleActor->SetPosition(p17[0],p17[1]+12);

    this->BuildTime.Modified();
  }
}