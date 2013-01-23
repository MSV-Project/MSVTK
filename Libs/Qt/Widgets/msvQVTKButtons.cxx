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

// Qt includes
#include <QImage>
#include <QDir>

// VTK Includes
#include <vtkButtonWidget.h>
#include <vtkCommand.h>
#include <vtkDataSet.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>
#include <vtkQImageToImageSource.h>
#include <vtkTexturedButtonRepresentation2D.h>

// MSVTK includes
#include "msvQVTKButtons.h"
#include "msvVTKButtons.h"


#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//------------------------------------------------------------------------------
// Callback respondign to vtkCommand::HighlightEvent
class vtkButtonHighLightCallback : public vtkCommand
{
public:
  static vtkButtonHighLightCallback *New()
  {
    return new vtkButtonHighLightCallback;
  }

  virtual void Execute(vtkObject *caller, unsigned long, void*)
  {
    vtkTexturedButtonRepresentation2D *rep =
      reinterpret_cast<vtkTexturedButtonRepresentation2D*>(caller);
    int highlightState = rep->GetHighlightState();
    if ( highlightState == vtkButtonRepresentation::HighlightHovering
      && PreviousHighlightState == vtkButtonRepresentation::HighlightNormal )
    {
      //show tooltip (not if previous state was selecting)
      ToolButton->setShowTooltip(true);
      ToolButton->getVTKButtonsInterface()->SetPreviousOpacity(ToolButton->getVTKButtonsInterface()->GetOpacity());
      ToolButton->getVTKButtonsInterface()->SetOpacity(1);
    }
    else if ( highlightState == vtkButtonRepresentation::HighlightNormal )
    {
      // hide tooltip
      ToolButton->setShowTooltip(false);
      ToolButton->getVTKButtonsInterface()->RestorePreviousOpacity();
    }
    PreviousHighlightState = highlightState;
  }

  vtkButtonHighLightCallback()
    : ToolButton(NULL), PreviousHighlightState(0), PreviousOpacity(1) {}
  msvQVTKButtons *ToolButton;
  int PreviousHighlightState;
  double PreviousOpacity;
};

//------------------------------------------------------------------------------
class msvQVTKButtonsPrivate
{
  Q_DECLARE_PUBLIC(msvQVTKButtons);

protected:

  msvQVTKButtons* const q_ptr;
  msvVTKButtons* m_VTKButtons;
  vtkCommand* m_HighlightCallback;

public:
  msvQVTKButtonsPrivate(msvQVTKButtons& object);
  virtual ~msvQVTKButtonsPrivate();

  // Getter and setter
  void setData(vtkDataSet* data){static_cast<msvVTKButtons*>(this->getVTKButtons())->SetData(data);};
  vtkDataSet* data(){return static_cast<msvVTKButtons*>(this->getVTKButtons())->GetData();};
  void setFlyTo(bool flyTo){static_cast<msvVTKButtons*>(this->getVTKButtons())->SetFlyTo(flyTo);};
  bool flyTo(){return static_cast<msvVTKButtons*>(this->getVTKButtons())->GetFlyTo();};
  void setOnCenter(bool onCenter){static_cast<msvVTKButtons*>(this->getVTKButtons())->SetOnCenter(onCenter);};
  bool onCenter(){return static_cast<msvVTKButtons*>(this->getVTKButtons())->GetOnCenter();};
  void setCurrentRenderer(vtkRenderer *renderer){static_cast<msvVTKButtons*>(this->getVTKButtons())->SetCurrentRenderer(renderer);}
  void setBounds(double b[6]){static_cast<msvVTKButtons*>(this->getVTKButtons())->SetBounds(b);}
  void update(){static_cast<msvVTKButtons*>(this->getVTKButtons())->Update();};
  virtual msvVTKButtonsInterface* getVTKButtons();
  vtkImageData* preview(int width,int height){return static_cast<msvVTKButtons*>(this->getVTKButtons())->GetPreview(width,height);};
};

//------------------------------------------------------------------------------
msvQVTKButtonsPrivate::msvQVTKButtonsPrivate(msvQVTKButtons& object)
  : m_VTKButtons(NULL), q_ptr(&object)
{
    Q_Q(msvQVTKButtons);
    //static_cast<msvVTKButtons*>(this->getVTKButtons());
    //m_VTKButton = msvVTKButtons::New();
    this->m_HighlightCallback = vtkButtonHighLightCallback::New();
    reinterpret_cast<vtkButtonHighLightCallback*>(
      this->m_HighlightCallback)->ToolButton = q;

    static_cast<msvVTKButtons*>(this->getVTKButtons())->GetButton()->GetRepresentation()->AddObserver(vtkCommand::HighlightEvent,this->m_HighlightCallback);
}

//------------------------------------------------------------------------------
/*virtual*/ msvVTKButtonsInterface* msvQVTKButtonsPrivate::getVTKButtons()
{
    Q_Q(msvQVTKButtons);
    if(!this->m_VTKButtons)
    {
        this->m_VTKButtons = msvVTKButtons::New();
        q->setVTKButtonsInterface(this->m_VTKButtons);
    }
    return this->m_VTKButtons;
}

//------------------------------------------------------------------------------
msvQVTKButtonsPrivate::~msvQVTKButtonsPrivate()
{
  static_cast<msvVTKButtons*>(this->getVTKButtons())->Delete();
}

//------------------------------------------------------------------------------
msvQVTKButtons::msvQVTKButtons(QObject *parent)
  : msvQVTKButtonsInterface(), d_ptr(new msvQVTKButtonsPrivate(*this))
{

}

//------------------------------------------------------------------------------
msvQVTKButtons::~msvQVTKButtons()
{

}

//------------------------------------------------------------------------------
void msvQVTKButtons::setCurrentRenderer(vtkRenderer *renderer)
{
  Q_D(msvQVTKButtons);
  msvQVTKButtonsInterface::setCurrentRenderer(renderer);
  d->setCurrentRenderer(renderer);
}

//------------------------------------------------------------------------------
void msvQVTKButtons::setBounds(double b[6])
{
  Q_D(msvQVTKButtons);
  msvQVTKButtonsInterface::setBounds(b);
  d->setBounds(b);
}

//------------------------------------------------------------------------------
void msvQVTKButtons::update()
{
  Q_D(msvQVTKButtons);
  msvQVTKButtonsInterface::update();
  d->update();
}

//------------------------------------------------------------------------------
void msvQVTKButtons::setFlyTo(bool active)
{
  Q_D(msvQVTKButtons);
  d->setFlyTo(active);
  update();
}

//------------------------------------------------------------------------------
QImage msvQVTKButtons::getPreview(int width, int height)
{
  Q_D(msvQVTKButtons);
  if(d->data())
  {
    double bounds[6];
    d->data()->GetBounds(bounds);

    vtkImageData* vtkImage=d->preview(width,height);
    if(!vtkImage)
        return QImage();
    vtkUnsignedCharArray* scalars =
      vtkUnsignedCharArray::SafeDownCast(
        vtkImage->GetPointData()->GetScalars());

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

    return qImage;
  }
  return QImage();
}

//------------------------------------------------------------------------------
void msvQVTKButtons::setData(vtkDataSet *data)
{
  Q_D(msvQVTKButtons);
  d->setData(data);
}

//------------------------------------------------------------------------------
bool msvQVTKButtons::flyTo()
{
  Q_D(msvQVTKButtons);
  return d->flyTo();
}

//------------------------------------------------------------------------------
void msvQVTKButtons::setOnCenter(bool onCenter)
{
  Q_D(msvQVTKButtons);
  d->setOnCenter(onCenter);
  update();
}

//------------------------------------------------------------------------------
bool msvQVTKButtons::onCenter()
{
  Q_D(msvQVTKButtons);
  return d->onCenter();
}
