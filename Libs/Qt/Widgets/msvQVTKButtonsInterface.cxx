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
#include <vtkQImageToImageSource.h>
#include <vtkRenderer.h>

// MSVTK includes
#include "msvQVTKButtonsInterface.h"
#include "msvVTKButtonsInterface.h"

#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//------------------------------------------------------------------------------
class msvQVTKButtonsInterfacePrivate
{
  Q_DECLARE_PUBLIC(msvQVTKButtonsInterface);

protected:
  msvQVTKButtonsInterface* const q_ptr; ///< PIMPL pointer
  msvVTKButtonsInterface* VTKButtonsInterface;
  msvQVTKButtonsAction* Action;

public:
  inline void setLabel(QString label){
    this->vtkButtonsInterface()->SetLabelText(label.toAscii());};

  inline QString label(){
    return QString(this->vtkButtonsInterface()->GetLabelText());};

  inline void setTooltip(QString tooltip){
    this->vtkButtonsInterface()->SetTooltip(tooltip.toAscii());};

  inline QString tooltip(){return QString(
          this->vtkButtonsInterface()->GetTooltip());};

  inline void setShowButton(bool show){
    this->vtkButtonsInterface()->SetShowButton(show);};

  inline bool showButton(){
    return this->vtkButtonsInterface()->GetShowButton();};

  inline void setShowLabel(bool show){
    this->vtkButtonsInterface()->SetShowLabel(show);};

  inline bool showLabel(){
    return this->vtkButtonsInterface()->GetShowLabel();};

  inline void setAction(msvQVTKButtonsAction* action){Action = action;};

  inline msvQVTKButtonsAction* action(){return Action;};

  inline vtkButtonWidget* button(){
    return this->vtkButtonsInterface()->GetButton();};

  inline void setBounds(double bds[6]){
    this->vtkButtonsInterface()->SetBounds(bds);};

  inline void bounds(double bds[6]){this->vtkButtonsInterface()->GetBounds(bds);};

  inline void setVTKButtonsInterface(msvVTKButtonsInterface *buttons){
    this->VTKButtonsInterface = buttons;};

  void setImage(QImage image);
  msvQVTKButtonsInterfacePrivate(msvQVTKButtonsInterface& object);
  virtual msvVTKButtonsInterface* vtkButtonsInterface();
  virtual ~msvQVTKButtonsInterfacePrivate();
};

//------------------------------------------------------------------------------
msvQVTKButtonsInterfacePrivate::msvQVTKButtonsInterfacePrivate(msvQVTKButtonsInterface& object)
  :  q_ptr(&object), VTKButtonsInterface(NULL)
{

}

//------------------------------------------------------------------------------
/*virtual*/ msvVTKButtonsInterface* msvQVTKButtonsInterfacePrivate::vtkButtonsInterface()
{
  return VTKButtonsInterface;
}

//------------------------------------------------------------------------------
msvQVTKButtonsInterfacePrivate::~msvQVTKButtonsInterfacePrivate()
{
  if (this->vtkButtonsInterface())
  {
    this->vtkButtonsInterface()->Delete();
  }
}

//------------------------------------------------------------------------------
void msvQVTKButtonsInterfacePrivate::setImage(QImage image)
{
  vtkQImageToImageSource *imageToVTK = vtkQImageToImageSource::New();
  imageToVTK->SetQImage(&image);
  imageToVTK->Update();
  this->vtkButtonsInterface()->SetImage(imageToVTK->GetOutput());
  imageToVTK->Delete();
}

//------------------------------------------------------------------------------
msvQVTKButtonsInterface::msvQVTKButtonsInterface(QObject *parent)
  : QObject(parent), d_ptr(new msvQVTKButtonsInterfacePrivate(*this))
{

}

//------------------------------------------------------------------------------
msvQVTKButtonsInterface::~msvQVTKButtonsInterface()
{

}

//------------------------------------------------------------------------------
vtkButtonWidget *msvQVTKButtonsInterface::button()
{
  Q_D(msvQVTKButtonsInterface);
  return d->button();
}

//------------------------------------------------------------------------------
void msvQVTKButtonsInterface::setImage(QImage image)
{
  Q_D(msvQVTKButtonsInterface);
  d->setImage(image);
}

//------------------------------------------------------------------------------
void msvQVTKButtonsInterface::bounds(double b[6])
{
  Q_D(msvQVTKButtonsInterface);
  d->bounds(b);
}

//------------------------------------------------------------------------------
void msvQVTKButtonsInterface::setShowButton(bool visible)
{
  Q_D(msvQVTKButtonsInterface);
  d->setShowButton(visible);
  //Q_EMIT(show(visible));
}

//------------------------------------------------------------------------------
void msvQVTKButtonsInterface::setShowTooltip(bool value)
{
  Q_D(msvQVTKButtonsInterface);
  if (value)
  {
    Q_EMIT showTooltip(d->tooltip());
  }
  else
  {
    Q_EMIT hideTooltip();
  }
}

//------------------------------------------------------------------------------
bool msvQVTKButtonsInterface::showButton()
{
  Q_D(msvQVTKButtonsInterface);
  return d->showButton();
}

//------------------------------------------------------------------------------
void msvQVTKButtonsInterface::setShowLabel(bool show)
{
  Q_D(msvQVTKButtonsInterface);
  d->setShowLabel(show);
}

//------------------------------------------------------------------------------
bool msvQVTKButtonsInterface::showLabel()
{
  Q_D(msvQVTKButtonsInterface);
  return d->showLabel();
}

//------------------------------------------------------------------------------
void msvQVTKButtonsInterface::setLabel(QString text)
{
  Q_D(msvQVTKButtonsInterface);
  d->setLabel(text);
}

//------------------------------------------------------------------------------
QString msvQVTKButtonsInterface::label()
{
  Q_D(msvQVTKButtonsInterface);
  return d->label();
}

//------------------------------------------------------------------------------
QString msvQVTKButtonsInterface::toolTip()
{
  Q_D(msvQVTKButtonsInterface);
  return d->tooltip();
}

//------------------------------------------------------------------------------
void msvQVTKButtonsInterface::setToolTip(QString text)
{
  Q_D(msvQVTKButtonsInterface);
  d->setTooltip(text);
}

//------------------------------------------------------------------------------
void msvQVTKButtonsInterface::setBounds(double b[6])
{
  Q_D(msvQVTKButtonsInterface);
  d->setBounds(b);
}

//------------------------------------------------------------------------------
msvVTKButtonsInterface* msvQVTKButtonsInterface::vtkButtonsInterface()
{
  Q_D(msvQVTKButtonsInterface);
  return d->vtkButtonsInterface();
}

//------------------------------------------------------------------------------
void msvQVTKButtonsInterface::setVTKButtonsInterface(msvVTKButtonsInterface* buttons)
{
  Q_D(msvQVTKButtonsInterface);
  return d->setVTKButtonsInterface(buttons);
}
