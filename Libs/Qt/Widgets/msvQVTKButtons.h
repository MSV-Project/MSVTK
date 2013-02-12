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

#ifndef __msvQVTKButtons_h
#define __msvQVTKButtons_h

// MSVTK includes
#include "msvQVTKButtonsInterface.h"

//forward declarations
class vtkRenderer;
class vtkButtonWidget;
class vtkButtonCallback;
class vtkButtonHighLightCallback;
class vtkRenderWindow;
class vtkDataSet;

// Pimpl
class msvQVTKButtonsPrivate;

/// \brief Qt wrapper around msvVTKButtons.
/// It exposes Qt signal/slot mechanism for a msvVTKButtons.
/// \sa msvVTKButtons
class MSV_QT_WIDGETS_EXPORT msvQVTKButtons : public msvQVTKButtonsInterface
{
  Q_OBJECT
  /// This property controls the camera behavior on click.
  /// If true, the camera smoothly "flies" from the current position to the
  /// location of the button, otherwise it "jumps" to its destination.
  /// \sa flyTo(), setFlyTo()
  Q_PROPERTY(bool flyTo READ flyTo WRITE setFlyTo);
  /// This property controls the location of the button with regard to the
  /// bounding box of the data.
  /// If true, the button is located on the center of the data, otherwise it is
  /// on the lower left corner of the data
  /// \sa onCenter(), setOnCenter()
  Q_PROPERTY(bool onCenter READ onCenter WRITE setOnCenter);

public Q_SLOTS:
  /// Allow to execute and update the pipeline when something change.
  /*virtual*/ void update();

  /// Allow to set button position on center or on corner
  /// \sa onCenter, onCenter()
  void setOnCenter(bool onCenter);

public:
  typedef msvQVTKButtonsInterface Superclass;
  /// Object constructor.
  msvQVTKButtons(QObject *parent = 0);

  /// Object destructor.
  virtual ~msvQVTKButtons();

  /// set the icon path
  //void setIconFileName(QString iconFileName);

  // Allow to activate FlyTo animation
  /// \sa flyTo, flyTo()
  void setFlyTo(bool active);

  /// Set the boundaries of the button's data.
  /// \sa onCenter, setData()
  void setBounds(double b[6]);

  /// Get the button preview image.
  QImage getPreview(int width, int height);

  /// Set the data for preview.
  /// \sa getPreview, setBounds()
  void setData(vtkDataSet *data);

  /// Return the flyTo flag.
  /// \sa flyTo, setFlyTo()
  bool flyTo();

  /// Return true if the button is centered, false otherwise.
  /// \sa onCenter, setOnCenter()
  bool onCenter();

  // Set the renderer the button must be added into.
  void setCurrentRenderer(vtkRenderer *renderer);

protected:
  QScopedPointer<msvQVTKButtonsPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(msvQVTKButtons);
  Q_DISABLE_COPY(msvQVTKButtons);
};

#endif // __msvQVTKButtons_h
