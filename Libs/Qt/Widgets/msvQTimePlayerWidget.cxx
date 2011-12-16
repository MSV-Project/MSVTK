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

// Qt includes
#include <QIcon>
#include <QTime>
#include <QTimer>

// MSV includes
#include "msvQTimePlayerWidget.h"
#include "ui_msvQTimePlayerWidget.h"

// VTK includes
#include "vtkAlgorithm.h"
#include "vtkAlgorithmOutput.h"
#include "vtkExecutive.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"
#include "vtkStreamingDemandDrivenPipeline.h"

//------------------------------------------------------------------------------
class msvQTimePlayerWidgetPrivate : public Ui_msvQTimePlayerWidget
{
  Q_DECLARE_PUBLIC(msvQTimePlayerWidget);
protected:
  msvQTimePlayerWidget* const q_ptr;

  vtkSmartPointer<vtkAlgorithm> filter;

  bool   automaticSingleStep;             // Compute singleStep as the time between frames, true by default
  double maxFrameRate;                    // Time Playing speed factor.
  QTimer* timer;                          // Timer to process the player.
  QTime realTime;                         // Time to reference the real one.
  QAbstractAnimation::Direction direction;// Sense of lecture

public:
  msvQTimePlayerWidgetPrivate(msvQTimePlayerWidget& object);
  virtual ~msvQTimePlayerWidgetPrivate();

  virtual void setupUi(QWidget*);
  virtual void updateUi();

  struct PipelineInfoType
    {
    PipelineInfoType();

    int    numberOfTimeSteps;
    double timeRange[2];
    double lastTimeRequest;
    bool   isConnected;

    double clampTimeInterval(double, double) const; // Tranform a frameRate into a time interval
    double frameToTime(int) const;    // Convert a frame index into a time.
    int timeToFrame(double) const;    // Convert a time into its closest frame.
    double timeNextFrame() const;     // Get the time corresponding to the next frame.
    double timePreviousFrame() const; // Get the time corresponding to the previous frame.
    };
  PipelineInfoType retrievePipelineInfo();            // Get pipeline information.
  virtual void processRequest(double);                // Request Data and Update
  virtual bool isConnected();                         // Check if the pipeline is ready
  virtual void processRequest(const PipelineInfoType&, double); // Request Data and Update
  virtual void requestData(const PipelineInfoType&, double);    // Request Data by time
  virtual void updateUi(const PipelineInfoType&);               // Update the widget giving pipeline statut
};

//------------------------------------------------------------------------------
// msvQECGMainWindowPrivate methods

//------------------------------------------------------------------------------
msvQTimePlayerWidgetPrivate::msvQTimePlayerWidgetPrivate
(msvQTimePlayerWidget& object)
  : q_ptr(&object)
{
  this->automaticSingleStep = true;
  this->maxFrameRate = 60;          // 60 FPS by default
}

//------------------------------------------------------------------------------
msvQTimePlayerWidgetPrivate::~msvQTimePlayerWidgetPrivate()
{
}

//------------------------------------------------------------------------------
msvQTimePlayerWidgetPrivate::PipelineInfoType::PipelineInfoType()
{
  this->numberOfTimeSteps = 0;
  this->timeRange[0]      = this->timeRange[1] = 0;
  this->lastTimeRequest   = 0;
  this->isConnected       = false;
}

//------------------------------------------------------------------------------
msvQTimePlayerWidgetPrivate::PipelineInfoType
msvQTimePlayerWidgetPrivate::retrievePipelineInfo()
{
  PipelineInfoType pipeInfo;

  pipeInfo.isConnected = this->isConnected();
  if (!pipeInfo.isConnected)
    return pipeInfo;

  vtkStreamingDemandDrivenPipeline* sdd = vtkStreamingDemandDrivenPipeline::
    SafeDownCast(this->filter->GetExecutive());
  vtkInformation* info = sdd->GetInputInformation(0)->GetInformationObject(0);

  if (!info->Has(vtkStreamingDemandDrivenPipeline::TIME_STEPS()) ||
      !info->Has(vtkStreamingDemandDrivenPipeline::TIME_RANGE()))
    return pipeInfo;

  pipeInfo.numberOfTimeSteps =
    info->Length(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
  info->Get(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), pipeInfo.timeRange);

  if (info->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS())) {
    pipeInfo.lastTimeRequest =
      info->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS())[0];
  }
  return pipeInfo;
}

//------------------------------------------------------------------------------
double msvQTimePlayerWidgetPrivate::PipelineInfoType::
clampTimeInterval(double playbackSpeed, double maxFrameRate) const
{
  Q_ASSERT(playbackSpeed > 0.);

  double timeFrame = this->frameToTime(1) / playbackSpeed;
  double maxFrameratePeriod = 1000. / maxFrameRate;

  // Clamp the time interval
  return qMin(timeFrame, maxFrameratePeriod);
}

//------------------------------------------------------------------------------
double msvQTimePlayerWidgetPrivate::PipelineInfoType::frameToTime(int frame) const
{
  double period = this->timeRange[1] - this->timeRange[0];
  if (this->numberOfTimeSteps == 0 || period == 0)
    return vtkMath::Nan();

  return period / static_cast<double>(this->numberOfTimeSteps-1) * frame;
}

//------------------------------------------------------------------------------
int msvQTimePlayerWidgetPrivate::PipelineInfoType::timeToFrame(double time) const
{
  double period = this->timeRange[1] - this->timeRange[0];
  if (this->numberOfTimeSteps == 0 || period == 0)
    return -1;

  return static_cast<int>(((this->numberOfTimeSteps-1)/period)*time);
}

//------------------------------------------------------------------------------
double msvQTimePlayerWidgetPrivate::PipelineInfoType::timePreviousFrame() const
{
  int currentFrame = this->timeToFrame(this->lastTimeRequest);
  return this->frameToTime(currentFrame-1);
}

//------------------------------------------------------------------------------
double msvQTimePlayerWidgetPrivate::PipelineInfoType::timeNextFrame() const
{
  int currentFrame = this->timeToFrame(this->lastTimeRequest);
  return this->frameToTime(currentFrame+1);
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidgetPrivate::setupUi(QWidget* widget)
{
  Q_Q(msvQTimePlayerWidget);

  this->Ui_msvQTimePlayerWidget::setupUi(widget);
  this->timer = new QTimer(widget);

  // Connect Menu ToolBars actions
  q->connect(this->firstFrameButton, SIGNAL(pressed()), q, SLOT(goToFirstFrame()));
  q->connect(this->previousFrameButton, SIGNAL(pressed()), q, SLOT(goToPreviousFrame()));
  q->connect(this->playButton, SIGNAL(toggled(bool)), q, SLOT(onPlay(bool)));
  q->connect(this->playReverseButton, SIGNAL(toggled(bool)), q, SLOT(onPlayReverse(bool)));
  q->connect(this->nextFrameButton, SIGNAL(pressed()), q, SLOT(goToNextFrame()));
  q->connect(this->lastFrameButton, SIGNAL(pressed()), q, SLOT(goToLastFrame()));
  q->connect(this->speedFactorSpinBox, SIGNAL(valueChanged(double)), q, SLOT(setPlaySpeed(double)));

  // Connect the time slider
  q->connect(this->timeSlider, SIGNAL(valueChanged(double)), q, SLOT(setCurrentTime(double)));
  this->timeSlider->setSuffix(" ms");

  // Connect the Timer for animation
  q->connect(this->timer, SIGNAL(timeout()), q, SLOT(onTick()));
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidgetPrivate::updateUi()
{
  PipelineInfoType pipeInfo = this->retrievePipelineInfo();
  this->updateUi(pipeInfo);
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidgetPrivate::updateUi(const PipelineInfoType& pipeInfo)
{
  // Buttons
  this->firstFrameButton->setEnabled((pipeInfo.lastTimeRequest > pipeInfo.timeRange[0]));
  this->previousFrameButton->setEnabled((pipeInfo.lastTimeRequest > pipeInfo.timeRange[0]));
  this->playButton->setEnabled((pipeInfo.numberOfTimeSteps > 1));
  this->playReverseButton->setEnabled((pipeInfo.numberOfTimeSteps > 1));
  this->nextFrameButton->setEnabled((pipeInfo.lastTimeRequest < pipeInfo.timeRange[1]));
  this->lastFrameButton->setEnabled((pipeInfo.lastTimeRequest < pipeInfo.timeRange[1]));

  // Slider
  this->timeSlider->blockSignals(true);
  this->timeSlider->setEnabled((pipeInfo.timeRange[0]!=pipeInfo.timeRange[1]));
  this->timeSlider->setRange(pipeInfo.timeRange[0], pipeInfo.timeRange[1]);
  this->timeSlider->setValue(pipeInfo.lastTimeRequest);

  // Set the slider default singleStep to a frame in automatique mode.
  if (this->automaticSingleStep)
    this->timeSlider->setSingleStep(pipeInfo.frameToTime(1));
  this->timeSlider->blockSignals(false);
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidgetPrivate::requestData(const PipelineInfoType& pipeInfo,
                                              double time)
{
  Q_Q(msvQTimePlayerWidget);

  // We clamp the time requested
  time = qBound( pipeInfo.timeRange[0], time, pipeInfo.timeRange[1]);

  // Abort the request
  if (!pipeInfo.isConnected || time == pipeInfo.lastTimeRequest)
    return;

  vtkStreamingDemandDrivenPipeline* sdd = vtkStreamingDemandDrivenPipeline::
    SafeDownCast(this->filter->GetInputConnection(0,0)->GetProducer()->GetExecutive());

  sdd->SetUpdateTimeStep(0, time);  // Request a time update
  emit q->currentTimeChanged(time); // Emit the change
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidgetPrivate::processRequest(double time)
{
  PipelineInfoType pipeInfo = this->retrievePipelineInfo();
  this->processRequest(pipeInfo, time);
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidgetPrivate::processRequest(const PipelineInfoType& pipeInfo,
                                                 double time)
{
  if (vtkMath::IsNan(time))
    return;

  this->requestData(pipeInfo, time);
  this->updateUi();
}

//------------------------------------------------------------------------------
bool msvQTimePlayerWidgetPrivate::isConnected()
{
  if (this->filter                 &&
      this->filter->HasExecutive() &&
      this->filter->GetExecutive()->GetInputInformation(0) &&
      this->filter->GetExecutive()->GetInputInformation(0)->
        GetInformationObject(0))
    return true;

  return false;
}

//------------------------------------------------------------------------------
// msvQECGMainWindow methods

//------------------------------------------------------------------------------
msvQTimePlayerWidget::msvQTimePlayerWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new msvQTimePlayerWidgetPrivate(*this))
{
  Q_D(msvQTimePlayerWidget);
  d->setupUi(this);
  d->updateUi();
}

//------------------------------------------------------------------------------
msvQTimePlayerWidget::~msvQTimePlayerWidget()
{
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::setFilter(vtkAlgorithm* algo)
{
  Q_D(msvQTimePlayerWidget);

  d->filter = algo;
  d->updateUi();
}

//------------------------------------------------------------------------------
vtkAlgorithm* msvQTimePlayerWidget::filter() const
{
  Q_D(const msvQTimePlayerWidget);
  return d->filter;
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::updateFromFilter()
{
  Q_D(msvQTimePlayerWidget);
  d->updateUi();
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::goToFirstFrame()
{
  Q_D(msvQTimePlayerWidget);

  // Fetch pipeline information
  msvQTimePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  d->processRequest(pipeInfo, pipeInfo.timeRange[0]);
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::goToPreviousFrame()
{
  Q_D(msvQTimePlayerWidget);

  // Fetch pipeline information
  msvQTimePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  d->processRequest(pipeInfo, pipeInfo.timePreviousFrame());
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::goToNextFrame()
{
  Q_D(msvQTimePlayerWidget);

  // Fetch pipeline information
  msvQTimePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  d->processRequest(pipeInfo, pipeInfo.timeNextFrame());
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::goToLastFrame()
{
  Q_D(msvQTimePlayerWidget);

  // Fetch pipeline information
  msvQTimePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  d->processRequest(pipeInfo, pipeInfo.timeRange[1]);
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::play(bool playPause)
{
  if (!playPause)
    this->pause();
  if (playPause)
    this->play();
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::play()
{
  Q_D(msvQTimePlayerWidget);

  // Fetch pipeline information
  msvQTimePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();
  double period = pipeInfo.timeRange[1] - pipeInfo.timeRange[0];

  if (!d->filter || period == 0)
    return;

  if (d->direction == QAbstractAnimation::Forward) {
      d->playReverseButton->blockSignals(true);
      d->playReverseButton->setChecked(false);
      d->playReverseButton->blockSignals(false);

      // Use when set the play by script
      if (!d->playButton->isChecked()) {
        d->playButton->blockSignals(true);
        d->playButton->setChecked(true);
        d->playButton->blockSignals(false);
      }

    // We reset the Slider to the initial value if we play from the end
    if (pipeInfo.lastTimeRequest == pipeInfo.timeRange[1])
      d->timeSlider->setValue(pipeInfo.timeRange[0]);
  }
  else if (d->direction == QAbstractAnimation::Backward) {
      d->playButton->blockSignals(true);
      d->playButton->setChecked(false);
      d->playButton->blockSignals(false);

      // Use when set the play by script
      if (!d->playReverseButton->isChecked()) {
        d->playReverseButton->blockSignals(true);
        d->playReverseButton->setChecked(true);
        d->playReverseButton->blockSignals(false);
      }

    // We reset the Slider to the initial value if we play from the beginning
    if (pipeInfo.lastTimeRequest == pipeInfo.timeRange[0])
      d->timeSlider->setValue(pipeInfo.timeRange[1]);
  }

  double timeInterval =
    pipeInfo.clampTimeInterval(d->speedFactorSpinBox->value(), d->maxFrameRate);
  d->realTime.start();
  d->timer->start(timeInterval);
  emit this->playing(true);
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::pause()
{
  Q_D(msvQTimePlayerWidget);

  if (d->direction == QAbstractAnimation::Forward)
    d->playButton->setChecked(false);
  else if (d->direction == QAbstractAnimation::Backward)
    d->playReverseButton->setChecked(false);

  if (d->timer->isActive()) {
    d->timer->stop();
    emit this->playing(false);
  }

  return;
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::stop()
{
  this->pause();
  this->goToFirstFrame();
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::onPlay(bool play)
{
  this->setDirection(QAbstractAnimation::Forward);
  this->play(play);
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::onPlayReverse(bool play)
{
  this->setDirection(QAbstractAnimation::Backward);
  this->play(play);
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::onTick()
{
  Q_D(msvQTimePlayerWidget);

  // Forward the internal timer timeout signal
  emit this->onTimeout();

  // Fetch pipeline information
  msvQTimePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  double timeRequest = pipeInfo.lastTimeRequest + d->realTime.restart() *
                       d->speedFactorSpinBox->value() *
                       ((d->direction == QAbstractAnimation::Forward) ? 1 : -1);

  if (d->playButton->isChecked() && !d->playReverseButton->isChecked()) {
    if (timeRequest > pipeInfo.timeRange[1] && !d->repeatButton->isChecked()) {
      d->processRequest(pipeInfo, timeRequest);
      this->onPlay(false);
      return;
    }
    else if (timeRequest > pipeInfo.timeRange[1] &&
             d->repeatButton->isChecked()) { // We Loop
      timeRequest = pipeInfo.timeRange[0];
      emit this->loop();
    }
  }
  else if (!d->playButton->isChecked() && d->playReverseButton->isChecked()) {
    if (timeRequest < pipeInfo.timeRange[0] && !d->repeatButton->isChecked()) {
      d->processRequest(pipeInfo, timeRequest);
      this->onPlayReverse(false);
      return;
    }
    else if (timeRequest < pipeInfo.timeRange[0] &&
             d->repeatButton->isChecked()) { // We Loop
      timeRequest = pipeInfo.timeRange[1];
      emit this->loop();
    }
  }
  else
    return; // Undefined statut

  d->processRequest(pipeInfo, timeRequest);
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::setCurrentTime(double time)
{
  Q_D(msvQTimePlayerWidget);
  d->processRequest(time);
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::setPlaySpeed(double speedFactor)
{
  Q_D(msvQTimePlayerWidget);
  speedFactor = speedFactor <= 0. ? 1. : speedFactor;
  d->speedFactorSpinBox->setValue(speedFactor);

  msvQTimePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  double timeInterval =
    pipeInfo.clampTimeInterval(speedFactor, d->maxFrameRate);
  d->timer->setInterval(timeInterval);
}

//------------------------------------------------------------------------------
// msvQECGMainWindow methods -- Widgets Interface

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::setfirstFrameIcon(const QIcon& ico)
{
  Q_D(msvQTimePlayerWidget);
  d->firstFrameButton->setIcon(ico);
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::setPreviousFrameIcon(const QIcon& ico)
{
  Q_D(msvQTimePlayerWidget);
  d->previousFrameButton->setIcon(ico);
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::setPlayIcon(const QIcon& ico)
{
  Q_D(msvQTimePlayerWidget);
  d->playButton->setIcon(ico);
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::setPlayReverseIcon(const QIcon& ico)
{
  Q_D(msvQTimePlayerWidget);
  d->playReverseButton->setIcon(ico);
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::setNextFrameIcon(const QIcon& ico)
{
  Q_D(msvQTimePlayerWidget);
  d->nextFrameButton->setIcon(ico);
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::setLastFrameIcon(const QIcon& ico)
{
  Q_D(msvQTimePlayerWidget);
  d->lastFrameButton->setIcon(ico);
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::setRepeatIcon(const QIcon& ico)
{
  Q_D(msvQTimePlayerWidget);
  d->repeatButton->setIcon(ico);
}

//------------------------------------------------------------------------------
QIcon msvQTimePlayerWidget::firstFrameIcon() const
{
  Q_D(const msvQTimePlayerWidget);
  return d->firstFrameButton->icon();
}

//------------------------------------------------------------------------------
QIcon msvQTimePlayerWidget::previousFrameIcon() const
{
  Q_D(const msvQTimePlayerWidget);
  return d->previousFrameButton->icon();
}

//------------------------------------------------------------------------------
QIcon msvQTimePlayerWidget::playIcon() const
{
  Q_D(const msvQTimePlayerWidget);
  return d->playButton->icon();
}

//------------------------------------------------------------------------------
QIcon msvQTimePlayerWidget::playReverseIcon() const
{
  Q_D(const msvQTimePlayerWidget);
  return d->playReverseButton->icon();
}

//------------------------------------------------------------------------------
QIcon msvQTimePlayerWidget::nextFrameIcon() const
{
  Q_D(const msvQTimePlayerWidget);
  return d->nextFrameButton->icon();
}

//------------------------------------------------------------------------------
QIcon msvQTimePlayerWidget::lastFrameIcon() const
{
  Q_D(const msvQTimePlayerWidget);
  return d->lastFrameButton->icon();
}

//------------------------------------------------------------------------------
QIcon msvQTimePlayerWidget::repeatIcon() const
{
  Q_D(const msvQTimePlayerWidget);
  return d->repeatButton->icon();
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::setPlayReverseVisibility(bool visible)
{
  Q_D(msvQTimePlayerWidget);
  d->playReverseButton->setVisible(visible);
}
//------------------------------------------------------------------------------
void msvQTimePlayerWidget::setBoundFramesVisibility(bool visible)
{
  Q_D(msvQTimePlayerWidget);

  d->firstFrameButton->setVisible(visible);
  d->lastFrameButton->setVisible(visible);
}
//------------------------------------------------------------------------------
void msvQTimePlayerWidget::setGoToVisibility(bool visible)
{
  Q_D(msvQTimePlayerWidget);

  d->previousFrameButton->setVisible(visible);
  d->nextFrameButton->setVisible(visible);
}
//------------------------------------------------------------------------------
void msvQTimePlayerWidget::setTimeSpinBoxVisibility(bool visible)
{
  Q_D(msvQTimePlayerWidget);
  d->timeSlider->setSpinBoxVisible(visible);
}

//------------------------------------------------------------------------------
bool msvQTimePlayerWidget::playReverseVisibility() const
{
  Q_D(const msvQTimePlayerWidget);
  return d->playReverseButton->isVisibleTo(
    const_cast<msvQTimePlayerWidget*>(this));
}
//------------------------------------------------------------------------------
bool msvQTimePlayerWidget::boundFramesVisibility() const
{
  Q_D(const msvQTimePlayerWidget);
  return (d->firstFrameButton->isVisibleTo(
            const_cast<msvQTimePlayerWidget*>(this)) &&
          d->lastFrameButton->isVisibleTo(
            const_cast<msvQTimePlayerWidget*>(this)));
}
//------------------------------------------------------------------------------
bool msvQTimePlayerWidget::goToVisibility() const
{
  Q_D(const msvQTimePlayerWidget);
  return (d->previousFrameButton->isVisibleTo(
            const_cast<msvQTimePlayerWidget*>(this)) &&
          d->nextFrameButton->isVisibleTo(
            const_cast<msvQTimePlayerWidget*>(this)));
}
//------------------------------------------------------------------------------
bool msvQTimePlayerWidget::timeSpinBoxVisibility() const
{
  Q_D(const msvQTimePlayerWidget);
  return d->timeSlider->spinBox()->isVisibleTo(
    const_cast<msvQTimePlayerWidget*>(this));
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::setSliderDecimals(int decimals)
{
  Q_D(msvQTimePlayerWidget);
  d->timeSlider->setDecimals(decimals);
}
//------------------------------------------------------------------------------
void msvQTimePlayerWidget::setSliderPageStep(double pageStep)
{
  Q_D(msvQTimePlayerWidget);
  d->timeSlider->setPageStep(pageStep);
}
//------------------------------------------------------------------------------
void msvQTimePlayerWidget::setSliderSingleStep(double singleStep)
{
  Q_D(msvQTimePlayerWidget);

  if (singleStep < 0) {
    d->automaticSingleStep = true;
    return;
  }

  d->automaticSingleStep = false;
  d->timeSlider->setSingleStep(singleStep);
}

//------------------------------------------------------------------------------
int msvQTimePlayerWidget::sliderDecimals() const
{
  Q_D(const msvQTimePlayerWidget);
  return d->timeSlider->decimals();
}
//------------------------------------------------------------------------------
double msvQTimePlayerWidget::sliderPageStep() const
{
  Q_D(const msvQTimePlayerWidget);
  return d->timeSlider->pageStep();
}

//------------------------------------------------------------------------------
double msvQTimePlayerWidget::sliderSingleStep() const
{
  Q_D(const msvQTimePlayerWidget);
  return d->timeSlider->singleStep();
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::setDirection(QAbstractAnimation::Direction direction)
{
  Q_D(msvQTimePlayerWidget);

  if (d->direction != direction) {
    d->direction = direction;
    emit this->directionChanged(direction);
  }
}

//------------------------------------------------------------------------------
QAbstractAnimation::Direction msvQTimePlayerWidget::direction() const
{
  Q_D(const msvQTimePlayerWidget);
  return d->direction;
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::setRepeat(bool repeat)
{
  Q_D(const msvQTimePlayerWidget);
  d->repeatButton->setChecked(repeat);
}

//------------------------------------------------------------------------------
bool msvQTimePlayerWidget::repeat() const
{
  Q_D(const msvQTimePlayerWidget);
  return d->repeatButton->isChecked();
}

//------------------------------------------------------------------------------
void msvQTimePlayerWidget::setMaxFramerate(double frameRate)
{
  Q_D(msvQTimePlayerWidget);
  // Clamp frameRate min value
  frameRate = (frameRate <= 0) ? 60 : frameRate;
  d->maxFrameRate = frameRate;
}

//------------------------------------------------------------------------------
double msvQTimePlayerWidget::maxFramerate() const
{
  Q_D(const msvQTimePlayerWidget);
  return d->maxFrameRate;
}

//------------------------------------------------------------------------------
double msvQTimePlayerWidget::currentTime() const
{
  Q_D(const msvQTimePlayerWidget);
  return d->timeSlider->value();
}

//------------------------------------------------------------------------------
double msvQTimePlayerWidget::playSpeed() const
{
  Q_D(const msvQTimePlayerWidget);
  return d->speedFactorSpinBox->value();
}
