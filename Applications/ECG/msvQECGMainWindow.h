/*==============================================================================

  Library: MSVTK

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

#ifndef __msvECGMainWindow_h
#define __msvECGMainWindow_h

// Qt includes
#include <QMainWindow>
class QListWidgetItem;

// CTK includes
#include <ctkVTKObject.h>

// ECG includes
#include "msvECGExport.h"

class msvQECGMainWindowPrivate;

/// \brief Main window for the ECG demo.
class MSV_ECG_EXPORT msvQECGMainWindow : public QMainWindow
{
  Q_OBJECT
  QVTK_OBJECT
public:
  typedef QMainWindow Superclass;
  msvQECGMainWindow(QWidget *parent=0);
  virtual ~msvQECGMainWindow();

public slots:
  /// Load carto data.
  void openData();
  /// Clear the views of all the loaded data.
  void closeData();
  /// Open About dialog.
  void aboutApplication();
  /// Refresh the view with the new data.
  void updateView();
  /// Set the current ECG signal.
  void setCurrentSignal(int pointId);

protected slots:
  void onPointSelected();
  void onCurrentTimeChanged(double);
  void onECGSelectionChanged();
  void onCurrentItemChanged(QListWidgetItem* current,
                            QListWidgetItem* previous);

protected:
  QScopedPointer<msvQECGMainWindowPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(msvQECGMainWindow);
  Q_DISABLE_COPY(msvQECGMainWindow);
};

#endif
