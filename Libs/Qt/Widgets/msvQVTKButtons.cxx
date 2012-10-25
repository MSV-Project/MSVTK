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

#include "msvQVTKButtons.h"
#include <QImage>
#include <QDir>

#include "msvQVTKAnimate.h"

#include <vtkSmartPointer.h>
#include <vtkAlgorithmOutput.h>
#include <vtkQImageToImageSource.h>

#include <vtkTextProperty.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>

#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkButtonWidget.h>
#include <vtkTexturedButtonRepresentation.h>
#include <vtkTexturedButtonRepresentation2D.h>
#include <vtkBalloonRepresentation.h>
#include <vtkCommand.h>


#include <vtkEllipticalButtonSource.h>
#include <vtkTexturedButtonRepresentation.h>

#include <vtkRenderWindow.h>
#include <vtkWindowToImageFilter.h>

#include <vtkDataSetMapper.h>
#include <vtkDataSet.h>
#include <vtkCamera.h>
#include <vtkPointData.h>

#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

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
    Q_UNUSED(caller);
    msvQVTKAnimate* animateCamera = new msvQVTKAnimate();
    if (flyTo)
    {
        animateCamera->execute(renderer, bounds, 200);
    }
    else
    {
        renderer->ResetCamera(bounds);
    }
    if(animateCamera)
    {
        delete animateCamera;
    }
    //selection
  }

  void setBounds(double b[6])
  {
    bounds[0] = b[0];
    bounds[1] = b[1];
    bounds[2] = b[2];
    bounds[3] = b[3];
    bounds[4] = b[4];
    bounds[5] = b[5];
  }

  void setFlyTo(bool fly)
  {
    flyTo = fly;
  }

  vtkButtonCallback():toolButton(NULL), renderer(0), flyTo(true) {}
  msvQVTKButtons *toolButton;
  vtkRenderer *renderer;
  double bounds[6];
  bool flyTo;
};

//------------------------------------------------------------------------------
// Callback respondign to vtkCommand::HighlightEvent
class MSV_QT_WIDGETS_EXPORT vtkButtonHighLightCallback : public vtkCommand
{
public:
  static vtkButtonHighLightCallback *New()
  {
    return new vtkButtonHighLightCallback;
  }

  virtual void Execute(vtkObject *caller, unsigned long, void*)
  {
    vtkTexturedButtonRepresentation2D *rep = reinterpret_cast<vtkTexturedButtonRepresentation2D*>(caller);
    int highlightState = rep->GetHighlightState();
    if ( highlightState == vtkButtonRepresentation::HighlightHovering && previousHighlightState == vtkButtonRepresentation::HighlightNormal )
    {
      //show tooltip (not if previous state was selecting
      toolButton->setShowTooltip(true);
    }
    else if ( highlightState == vtkButtonRepresentation::HighlightNormal)
    {
      //hide tooltip
      toolButton->setShowTooltip(false);
    }
    previousHighlightState = highlightState;
  }

  vtkButtonHighLightCallback():toolButton(NULL), previousHighlightState(0) {}
  msvQVTKButtons *toolButton;
  int previousHighlightState;
};

//------------------------------------------------------------------------------
class msvQVTKButtonsPrivate
{
  Q_DECLARE_PUBLIC(msvQVTKButtons);

protected:

  msvQVTKButtons* const q_ptr;

  //QImage m_Image; ///< button image
  vtkDataSet* m_Data; ///< dataset associated with the button
  vtkRenderWindow *m_Window; ///< render window for offscreen rendering
  bool m_FlyTo; ///< Flag to activate FlyTo animation
  bool m_OnCenter; ///< Flag to set button position on center or on corner (can be refactored with a enum??)

public:
  msvQVTKButtonsPrivate(msvQVTKButtons& object);
  virtual ~msvQVTKButtonsPrivate();

  // Getter and setter
  void setData(vtkDataSet* data){m_Data = data;};
  vtkDataSet* data(){return m_Data;};
  void setFlyTo(bool flyTo){m_FlyTo = flyTo;};
  bool flyTo(){return m_FlyTo;};
  void setOnCenter(bool oncenter){m_OnCenter = oncenter;};
  bool onCenter(){return m_OnCenter;};
  vtkRenderWindow* window();

};

msvQVTKButtonsPrivate::msvQVTKButtonsPrivate(msvQVTKButtons& object) : m_FlyTo(true), m_OnCenter(false), m_Window(NULL), q_ptr(&object)
{

}

msvQVTKButtonsPrivate::~msvQVTKButtonsPrivate()
{
  if(m_Window)
  {
    m_Window->Delete();
  }
}

vtkRenderWindow* msvQVTKButtonsPrivate::window()
{
  if(m_Window == NULL)
  {
    m_Window = vtkRenderWindow::New();
  }
  return m_Window;
}

//------------------------------------------------------------------------------
msvQVTKButtons::msvQVTKButtons(QObject *parent) : msvQVTKButtonsInterface(), d_ptr(new msvQVTKButtonsPrivate(*this))
{
  Q_D(msvQVTKButtons);
  m_ButtonCallback = vtkButtonCallback::New();
  reinterpret_cast<vtkButtonCallback*>(m_ButtonCallback)->toolButton = this;
  m_HighlightCallback = vtkButtonHighLightCallback::New();
  reinterpret_cast<vtkButtonHighLightCallback*>(m_HighlightCallback)->toolButton = this;

  button()->AddObserver(vtkCommand::StateChangedEvent,m_ButtonCallback);
  button()->GetRepresentation()->AddObserver(vtkCommand::HighlightEvent,m_HighlightCallback);
}

msvQVTKButtons::~msvQVTKButtons()
{
//   if(m_Window)
//   {
//     m_Window->Delete();
//   }
}

void msvQVTKButtons::setCurrentRenderer(vtkRenderer *renderer)
{
  Q_D(msvQVTKButtons);
  msvQVTKButtonsInterface::setCurrentRenderer(renderer);
  if(renderer)
  {
    reinterpret_cast<vtkButtonCallback*>(m_ButtonCallback)->renderer = renderer;
  }
  else
  {
    reinterpret_cast<vtkButtonCallback*>(m_ButtonCallback)->renderer = NULL;
  }
}

void msvQVTKButtons::setBounds(double b[6])
{
  reinterpret_cast<vtkButtonCallback*>(m_ButtonCallback)->setBounds(b);
  update();
}

void msvQVTKButtons::update()
{
  Q_D(msvQVTKButtons);
  calculatePosition();
  msvQVTKButtonsInterface::update();
  if(m_ButtonCallback)
  {
    reinterpret_cast<vtkButtonCallback*>(m_ButtonCallback)->flyTo = d->m_FlyTo;
    if(reinterpret_cast<vtkButtonCallback*>(m_ButtonCallback)->renderer)
    {
      reinterpret_cast<vtkButtonCallback*>(m_ButtonCallback)->renderer->GetRenderWindow()->Render();
    }
  }
}

void msvQVTKButtons::setFlyTo(bool active)
{
  Q_D(msvQVTKButtons);
  d->setFlyTo(active);
  update();
}

QImage msvQVTKButtons::getPreview(int width, int height)
{
  Q_D(msvQVTKButtons);
  if(d->data())
  {
    double bounds[6];
    d->data()->GetBounds(bounds);

    // create pipe
    vtkWindowToImageFilter *previewer = vtkWindowToImageFilter::New();
    vtkDataSetMapper *mapper = vtkDataSetMapper::New();
    vtkActor *actor = vtkActor::New();
    vtkRenderer *renderer = vtkRenderer::New();
    if(d->m_Window == NULL)
    {
      d->window()->SetDisplayId(NULL);
    }

    mapper->SetInput(d->m_Data);
    mapper->Update();
    actor->SetMapper(mapper);

    // offscreen rendering
    d->window()->OffScreenRenderingOn();
    d->window()->AddRenderer(renderer);
    d->window()->Start();
    d->window()->SetSize(width,height);
    renderer->AddActor(actor);
    renderer->ResetCamera(bounds);

    // Extract the image from the 'hidden' renderer
    previewer->SetInput(d->window());
    previewer->Modified();
    previewer->Update();

    // Convert image data to QImage and return
    vtkImageData* vtkImage = previewer->GetOutput();
    if(!vtkImage)
        return QImage();
    vtkUnsignedCharArray* scalars = vtkUnsignedCharArray::SafeDownCast(vtkImage->GetPointData()->GetScalars());
    if(!width || !height || !scalars)
        return QImage();
    QImage qImage(width, height, QImage::Format_ARGB32);
    vtkIdType tupleIndex=0;
    int qImageBitIndex=0;
    QRgb* qImageBits = (QRgb*)qImage.bits();
    unsigned char* scalarTuples = scalars->GetPointer(0);
    for(int j=0; j<height; j++)
    {
      for(int i=0; i<width; i++)
      {
        unsigned char* tuple = scalarTuples+(tupleIndex++*3);
        QRgb color = qRgba(tuple[0], tuple[1], tuple[2], 255);
        *(qImageBits+(qImageBitIndex++))=color;
      }
    }

    previewer->SetInput(NULL);
    previewer->Delete();

    mapper->SetInput(NULL);
    mapper->Delete();

    actor->SetMapper(NULL);
    actor->Delete();

    d->window()->RemoveRenderer(renderer);
    //destroy pipe
    renderer->Delete();

    return qImage;
  }
  return QImage();
}

void msvQVTKButtons::setData(vtkDataSet *data)
{
  Q_D(msvQVTKButtons);
  d->setData(data);
}

bool msvQVTKButtons::flyTo()
{
  Q_D(msvQVTKButtons);
  return d->flyTo();
}

void msvQVTKButtons::setOnCenter(bool onCenter)
{
  Q_D(msvQVTKButtons);
  d->setOnCenter(onCenter);
}

bool msvQVTKButtons::onCenter()
{
  Q_D(msvQVTKButtons);
  return d->onCenter();
}

void msvQVTKButtons::calculatePosition()
{
  Q_D(msvQVTKButtons);

  //modify position of the vtkButton
  double bds[6];
  double coord[3];
  bounds(bds);
  if (d->onCenter())
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
  vtkTexturedButtonRepresentation2D *rep = static_cast<vtkTexturedButtonRepresentation2D *>(button()->GetRepresentation());

  rep->PlaceWidget(bds, size);
  rep->Modified();
  button()->SetRepresentation(rep);
}