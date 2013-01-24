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
      ToolButton->vtkButtonsInterface()->SetPreviousOpacity(
            ToolButton->vtkButtonsInterface()->GetOpacity());
      ToolButton->vtkButtonsInterface()->SetOpacity(1);
    }
    else if ( highlightState == vtkButtonRepresentation::HighlightNormal )
    {
      // hide tooltip
      ToolButton->setShowTooltip(false);
      ToolButton->vtkButtonsInterface()->RestorePreviousOpacity();
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
  msvVTKButtons* VtkButton;
  vtkCommand* HighlightCallback;

public:
  msvQVTKButtonsPrivate(msvQVTKButtons& object);
  virtual ~msvQVTKButtonsPrivate();

  // Getter and setter
  inline void setData(vtkDataSet* data){
    msvVTKButtons::SafeDownCast(this->vtkButtons())->SetData(data);};

  inline vtkDataSet* data(){
    msvVTKButtons::SafeDownCast(this->vtkButtons())->GetData();};

  inline void setFlyTo(bool flyTo){
    msvVTKButtons::SafeDownCast(this->vtkButtons())->SetFlyTo(flyTo);};

  inline bool flyTo(){
    msvVTKButtons::SafeDownCast(this->vtkButtons())->GetFlyTo();};

  void setOnCenter(bool onCenter){
    msvVTKButtons::SafeDownCast(this->vtkButtons())->SetOnCenter(onCenter);};

  inline bool onCenter(){
    msvVTKButtons::SafeDownCast(this->vtkButtons())->GetOnCenter();};

  inline void setCurrentRenderer(vtkRenderer *renderer){
    static_cast<msvVTKButtons*>(
          this->vtkButtons())->SetCurrentRenderer(renderer);}

  inline void setBounds(double b[6]){
    static_cast<msvVTKButtons*>(this->vtkButtons())->SetBounds(b);};

  inline void update(){static_cast<msvVTKButtons*>(this->vtkButtons())->Update();};

  inline vtkImageData* preview(int width,int height){
    msvVTKButtons::SafeDownCast(
          this->vtkButtons())->GetPreview(width,height);};

  virtual msvVTKButtonsInterface* vtkButtons();
};

//------------------------------------------------------------------------------
msvQVTKButtonsPrivate::msvQVTKButtonsPrivate(msvQVTKButtons& object)
  : VtkButton(NULL), q_ptr(&object)
{
  Q_Q(msvQVTKButtons);
  this->HighlightCallback = vtkButtonHighLightCallback::New();
  reinterpret_cast<vtkButtonHighLightCallback*>(
  this->HighlightCallback)->ToolButton = q;

  q->setVTKButtonsInterface(this->vtkButtons());

  static_cast<msvVTKButtons*>(
        this->vtkButtons())->GetButton()->GetRepresentation()->AddObserver(
        vtkCommand::HighlightEvent,this->HighlightCallback);
}

//------------------------------------------------------------------------------
/*virtual*/ msvVTKButtonsInterface* msvQVTKButtonsPrivate::vtkButtons()
{
  if (!this->VtkButton)
  {
    this->VtkButton = msvVTKButtons::New();
  }
  return this->VtkButton;
}

//------------------------------------------------------------------------------
msvQVTKButtonsPrivate::~msvQVTKButtonsPrivate()
{
  static_cast<msvVTKButtons*>(this->vtkButtons())->Delete();
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
  //msvQVTKButtonsInterface::setCurrentRenderer(renderer);
  d->setCurrentRenderer(renderer);
}

//------------------------------------------------------------------------------
void msvQVTKButtons::setBounds(double b[6])
{
  Q_D(msvQVTKButtons);
  //msvQVTKButtonsInterface::setBounds(b);
  d->setBounds(b);
}

//------------------------------------------------------------------------------
void msvQVTKButtons::update()
{
  Q_D(msvQVTKButtons);
  //msvQVTKButtonsInterface::update();
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
  if (d->data())
  {
    double bounds[6];
    d->data()->GetBounds(bounds);

    vtkImageData* vtkImage=d->preview(width,height);
    if (!vtkImage)
        return QImage();
    vtkUnsignedCharArray* scalars =
      vtkUnsignedCharArray::SafeDownCast(
        vtkImage->GetPointData()->GetScalars());

    if (!width || !height || !scalars)
        return QImage();
    QImage qImage(width, height, QImage::Format_ARGB32);
    vtkIdType tupleIndex=0;
    int qImageBitIndex=0;
    QRgb* qImageBits = (QRgb*)qImage.bits();
    unsigned char* scalarTuples = scalars->GetPointer(0);
    for (int j=0; j<height; ++j)
    {
      for (int i=0; i<width; ++i)
      {
        unsigned char* tuple = scalarTuples+(++tupleIndex*3);
        QRgb color = qRgba(tuple[0], tuple[1], tuple[2], 255);
        *(qImageBits+(++qImageBitIndex))=color;
      }
    }
    vtkImage->Delete();
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
