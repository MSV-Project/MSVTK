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

// It encapsulates the animation control functionality and provides
// slots and signals to manage all the animation process.
// The widget connect himself to a *vtkFilter*, typically at the output of the
// vtk Pipeline (i.e. on a *vtkMapper*) and proceed to the requests.

#ifndef __msvQTimePlayerWidget_h
#define __msvQTimePlayerWidget_h

// Qt includes
#include <QIcon>
#include <QWidget>
#include <QAbstractAnimation>

// ECG includes
#include "msvQtWidgetsExport.h"

// VTK includes
class vtkAlgorithm;
class ctkSliderWidget;
class msvQTimePlayerWidgetPrivate;

class MSV_QT_WIDGETS_EXPORT msvQTimePlayerWidget : public QWidget
{
  Q_OBJECT
  /// This property holds the firstFrame button's icon.
  Q_PROPERTY(QIcon firstFrameIcon READ firstFrameIcon WRITE setfirstFrameIcon)
  /// This property holds the previousFrame button's icon.
  Q_PROPERTY(QIcon previousFrameIcon READ previousFrameIcon WRITE setPreviousFrameIcon)
  /// This property holds the play button's icon.
  Q_PROPERTY(QIcon playIcon READ playIcon WRITE setPlayIcon)
  /// This property holds the play reverse button's icon.
  Q_PROPERTY(QIcon playReverseIcon READ playReverseIcon WRITE setPlayReverseIcon)
  /// This property holds the nextFrame button's icon.
  Q_PROPERTY(QIcon nextFrameIcon READ nextFrameIcon WRITE setNextFrameIcon)
  /// This property holds the lastFrame button's icon.
  Q_PROPERTY(QIcon lastFrameIcon READ lastFrameIcon WRITE setLastFrameIcon)
  /// This property holds the repeat button's icon.
  Q_PROPERTY(QIcon repeatIcon READ repeatIcon WRITE setRepeatIcon)

  /// Enable/Disable the visibility of play reverse button.
  Q_PROPERTY(bool playReverseVisibility READ playReverseVisibility WRITE setPlayReverseVisibility)
  /// Enable/Disable the visibility of the firstFrame and lastFrame buttons.
  Q_PROPERTY(bool boundFramesVisibility READ boundFramesVisibility WRITE setBoundFramesVisibility)
  /// Enable/Disable the visibility of the seekBackward and seekForward buttons.
  Q_PROPERTY(bool goToVisibility READ goToVisibility WRITE setGoToVisibility)
  /// Enable/Disable the visibility of time spinBox
  Q_PROPERTY(bool timeSpinBoxVisibility READ timeSpinBoxVisibility WRITE setTimeSpinBoxVisibility)

  /// This property holds the number of decimal digits for the timeSlider.
  Q_PROPERTY(int sliderDecimals READ sliderDecimals WRITE setSliderDecimals)
  /// This property holds the page step for the timeSlider.
  Q_PROPERTY(double sliderPageStep READ sliderPageStep WRITE setSliderPageStep)
  /// This property holds the single step for the timeSlider.
  /// The automatic mode (by default) compute the step corresponding to one
  /// frame. To get back to the automatic mode, set a negative value.
  Q_PROPERTY(double sliderSingleStep READ sliderSingleStep WRITE setSliderSingleStep)

  /// This property holds the number of the higher frame per seconds rate the
  /// the player will be willing to handle.
  Q_PROPERTY(double maxFramerate READ maxFramerate WRITE setMaxFramerate)
  /// This property holds the time direction on which the widget will do the
  /// animation.
  Q_PROPERTY(QAbstractAnimation::Direction direction READ direction WRITE setDirection NOTIFY directionChanged)
  /// This property holds if the animation is repeated when we reach the end.
  Q_PROPERTY(bool repeat READ repeat WRITE setRepeat)
  /// This property holds the speed factor of the animation
  Q_PROPERTY(double playSpeed READ playSpeed WRITE setPlaySpeed)
  /// This property is an accessor to the widget's current time.
  Q_PROPERTY(double currentTime READ currentTime WRITE setCurrentTime NOTIFY currentTimeChanged)

public:
  typedef QWidget Superclass;
  msvQTimePlayerWidget(QWidget* parent=0);
  virtual ~msvQTimePlayerWidget();

  // Description
  // Set the filter on which we will connect.
  void setFilter(vtkAlgorithm* algo);
  vtkAlgorithm* filter() const;

  // Icons accessors
  void setfirstFrameIcon(const QIcon&);
  QIcon firstFrameIcon() const;
  void setPreviousFrameIcon(const QIcon&);
  QIcon previousFrameIcon() const;
  void setPlayIcon(const QIcon&);
  QIcon playIcon() const;
  void setPlayReverseIcon(const QIcon&);
  QIcon playReverseIcon() const;
  void setNextFrameIcon(const QIcon&);
  QIcon nextFrameIcon() const;
  void setLastFrameIcon(const QIcon&);
  QIcon lastFrameIcon() const;
  void setRepeatIcon(const QIcon&);
  QIcon repeatIcon() const;

  /// Visibility access
  void setPlayReverseVisibility(bool visible);
  bool playReverseVisibility() const;
  void setBoundFramesVisibility(bool visible);
  bool boundFramesVisibility() const;
  void setGoToVisibility(bool visible);
  bool goToVisibility() const;
  void setTimeSpinBoxVisibility(bool visible);
  bool timeSpinBoxVisibility() const;

  /// Slider widget
  void setSliderDecimals(int decimals);
  int sliderDecimals() const;
  void setSliderPageStep(double pageStep);
  double sliderPageStep() const;
  void setSliderSingleStep(double singleStep);
  double sliderSingleStep() const;

  /// Playback
  void setDirection(QAbstractAnimation::Direction playDirection);
  QAbstractAnimation::Direction direction() const;
  void setRepeat(bool repeat);
  bool repeat() const;
  void setMaxFramerate(double);
  double maxFramerate() const;
  double currentTime() const;
  double playSpeed() const;

public slots:
  virtual void setCurrentTime(double timeInMs);
  virtual void setPlaySpeed(double speedCoef);
  virtual void goToFirstFrame();
  virtual void goToPreviousFrame();
  virtual void goToNextFrame();
  virtual void goToLastFrame();

  virtual void play();
  virtual void pause();
  void play(bool playPause);
  void onPlay(bool);
  void onPlayReverse(bool);
  void stop();

  virtual void updateFromFilter();

protected slots:
  virtual void onTick();

signals:
  // emitted when the time has been changed
  void currentTimeChanged(double);

  // emitted when the internal timer send a timeout
  void onTimeout();

  // emitted with playing(true) when play begins and
  // playing(false) when play ends.
  void playing(bool);

  // emitted when the player loops the animation
  void loop();

  // emitted when the sense of the playback is changed
  void directionChanged(QAbstractAnimation::Direction);

protected:
  QScopedPointer<msvQTimePlayerWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(msvQTimePlayerWidget);
  Q_DISABLE_COPY(msvQTimePlayerWidget);
};

#endif
