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

#ifndef msvQVTKButtonsManager_H
#define msvQVTKButtonsManager_H

// Includes list

#include "msvQtWidgetsExport.h"
#include "msvQVTKButtons.h"
#include "msvQVTKButtonsGroup.h"

/**
Class name: msvQVTKButtonsManager
Manager class to manage groups of msvQVTKButtons and msvVTKButtonsGroup
*/

class MSV_QT_WIDGETS_EXPORT msvQVTKButtonsManager : public QObject
{
  Q_OBJECT

public:
  /// Object destructor.
  virtual ~msvQVTKButtonsManager();

  /// Get the singleton instance of the manager
  static msvQVTKButtonsManager* instance();

  /// Create a group
  inline msvQVTKButtonsGroup *createGroup();

  /// Create a button
  inline msvQVTKButtons *createButtons();

  /// Set show button property for children elements
  inline void setShowButton(bool show);

  /// Set show label property for children elements
  inline void setShowLabel(bool show);

private:
  /// Object constructor.
  msvQVTKButtonsManager(QObject *parent = 0);

  /// Set the specified property for children elements
  void setElementProperty(QString name, QVariant value);

  QVector<msvQVTKButtonsInterface*> m_Elements; //< Vector of buttons
};

/////////////////////////////////////////////////////////////
// Inline methods
/////////////////////////////////////////////////////////////

inline msvQVTKButtonsGroup *msvQVTKButtonsManager::createGroup()
{
  m_Elements.push_back(new msvQVTKButtonsGroup());
  return static_cast<msvQVTKButtonsGroup*>(m_Elements.at(m_Elements.size()-1));
}
inline msvQVTKButtons *msvQVTKButtonsManager::createButtons()
{
  m_Elements.push_back(new msvQVTKButtons());
  return static_cast<msvQVTKButtons*>(m_Elements.at(m_Elements.size()-1));
}

inline void msvQVTKButtonsManager::setShowButton(bool show)
{
  setElementProperty("showButton",show);
}

inline void msvQVTKButtonsManager::setShowLabel(bool show)
{
  setElementProperty("showLabel",show);
}

#endif // msvQVTKButtonsManager_H
