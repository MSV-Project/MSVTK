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

// MSVTK includes
#include "msvQVTKButtonsInterface.h"
#include "msvQVTKButtonsManager.h"

//------------------------------------------------------------------------------
class msvQVTKButtonsManagerPrivate
{
  Q_DECLARE_PUBLIC(msvQVTKButtonsManager);

protected:

  msvQVTKButtonsManager* const q_ptr;
  QVector<msvQVTKButtonsInterface*> m_Elements; //< Vector of buttons

public:

  msvQVTKButtonsManagerPrivate(msvQVTKButtonsManager& object);
  ~msvQVTKButtonsManagerPrivate();
  void setElementProperty(QString name, QVariant value);
  msvQVTKButtonsGroup *createGroup();
  msvQVTKButtons *createButtons();
};

//------------------------------------------------------------------------------
msvQVTKButtonsManagerPrivate::msvQVTKButtonsManagerPrivate(msvQVTKButtonsManager& object)
  : q_ptr(&object)
{

}

//------------------------------------------------------------------------------
msvQVTKButtonsManagerPrivate::~msvQVTKButtonsManagerPrivate()
{

}

//------------------------------------------------------------------------------
void msvQVTKButtonsManagerPrivate::setElementProperty(QString name, QVariant value)
{
  for(QVector<msvQVTKButtonsInterface*>::iterator buttonsIt = m_Elements.begin();
    buttonsIt != m_Elements.end(); buttonsIt++)
  {
    (*buttonsIt)->setProperty(name.toStdString().c_str(),value);
  }
}

//------------------------------------------------------------------------------
msvQVTKButtonsGroup *msvQVTKButtonsManagerPrivate::createGroup()
{
  m_Elements.push_back(new msvQVTKButtonsGroup());
  msvVTKButtonsManager::GetInstance()->AddElement(m_Elements.at(m_Elements.size()-1)->vtkButtonsInterface());
  return static_cast<msvQVTKButtonsGroup*>(m_Elements.at(m_Elements.size()-1));
}

//------------------------------------------------------------------------------
msvQVTKButtons *msvQVTKButtonsManagerPrivate::createButtons()
{
  m_Elements.push_back(new msvQVTKButtons());
  msvVTKButtonsManager::GetInstance()->AddElement(m_Elements.at(m_Elements.size()-1)->vtkButtonsInterface());
  return static_cast<msvQVTKButtons*>(m_Elements.at(m_Elements.size()-1));
}

//------------------------------------------------------------------------------
msvQVTKButtonsManager::msvQVTKButtonsManager(QObject *parent)
  : d_ptr(new msvQVTKButtonsManagerPrivate(*this))
{

}

//------------------------------------------------------------------------------
msvQVTKButtonsManager* msvQVTKButtonsManager::instance()
{
  static msvQVTKButtonsManager manager;
  return &manager;
}

//------------------------------------------------------------------------------
msvQVTKButtonsManager::~msvQVTKButtonsManager()
{

}

//------------------------------------------------------------------------------
msvQVTKButtonsGroup *msvQVTKButtonsManager::createGroup()
{
  Q_D(msvQVTKButtonsManager);
  return d->createGroup();
}

//------------------------------------------------------------------------------
msvQVTKButtons *msvQVTKButtonsManager::createButtons()
{
  Q_D(msvQVTKButtonsManager);
  return d->createButtons();
}

//------------------------------------------------------------------------------
void msvQVTKButtonsManager::setShowButton(bool show)
{
  Q_D(msvQVTKButtonsManager);
  d->setElementProperty("showButton",show);
}

//------------------------------------------------------------------------------
void msvQVTKButtonsManager::setShowLabel(bool show)
{
  Q_D(msvQVTKButtonsManager);
  d->setElementProperty("showLabel",show);
}
