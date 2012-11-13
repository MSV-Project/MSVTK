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

class msvQVTKButtonsManagerPrivate;

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
  msvQVTKButtonsGroup *createGroup();

  /// Create a button
  msvQVTKButtons *createButtons();

  /// Set show button property for children elements
  void setShowButton(bool show);

  /// Set show label property for children elements
  void setShowLabel(bool show);

protected:
  QScopedPointer<msvQVTKButtonsManagerPrivate> d_ptr;
  msvQVTKButtonsManager(QObject *parent = 0);

private:
  Q_DECLARE_PRIVATE(msvQVTKButtonsManager);
  Q_DISABLE_COPY(msvQVTKButtonsManager);
};

/////////////////////////////////////////////////////////////
// Inline methods
/////////////////////////////////////////////////////////////

#endif // msvQVTKButtonsManager_H
