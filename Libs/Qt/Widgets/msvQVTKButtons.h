/*
 *  msvQVTKButtons.h
 *  
 *
 *  Created by Daniele Giunchi on 13/01/12.
 *  Copyright 2011 B3C. All rights reserved.
 *
 *  See License at: http://tiny.cc/QXJ4D
 *
 */

#ifndef msvQVTKButtons_H
#define msvQVTKButtons_H

#include "msvQtWidgetsExport.h"

#include <QObject>

//forward declarations
class vtkButtonWidget;
class vtkButtonCallback;
class vtkButtonHighLightCallback;
class vtkRenderer;

/**
 Class name: msvQVTKButtons
 This is the tool representing a VTK buttons.
 */
class MSV_QT_WIDGETS_EXPORT msvQVTKButtons : public QObject {
    Q_OBJECT
    
public Q_SLOTS:
    /// Allow to execute and update the pipeline when something change.
    /*virtual*/ void update();

public:
     /// Object constructor.
    msvQVTKButtons(QObject *parent = 0);

    /// Allow to show/hide button
    void setShowButton(bool show);

    /// Return showLabel flag
    bool showButton() const;

    /// Allow to show/hide label
    void setShowLabel(bool show);
    
    /// set the icon path
    void setIconFileName(QString &iconFileName);

    /// Return showLabel flag
    bool showLabel() const;
    
    /// set the text label
    void setLabel(QString &text);

    /// Allow to activate FlyTo animation
    void setFlyTo(bool active);

    /// Return FlyTo flag
    bool FlyTo() const;

    /// Allow to set button position on center or on corner
    void setOnCenter(bool onCenter);

    /// Return OnCenter flag
    bool OnCenter() const;
    
    /// set the tooltip string
    void setToolTip(QString &text);

    /// add vtk button to Renderer
    void setCurrentRenderer(vtkRenderer *renderer);
    
    /// set bounds
    void setBounds(double b[6]);
    
    /// set the show/hide signal
    void setShowTooltip(bool value);
    
    /// Object destructor.
    virtual ~msvQVTKButtons();

signals:
    /// signal launched with shown tooltip
    void showTooltip(QString text);
    
    /// signal launched with shown tooltip
    void hideTooltip();

private:
    /// calculate Position (center or corner)
    void calculatePosition();
    
    vtkButtonWidget *m_ButtonWidget; ///< VTK button widget.
    vtkButtonCallback *buttonCallback; ///< Callback called by picking on vtkButton
    vtkButtonHighLightCallback *highlightCallback; ///< Callback called by hovering over the button.
    
    QString m_Label; ///< label of the button
    QString m_Tooltip; ///< tooltip associated to the button
    QString m_IconFileName; ///< File name of the image to be applied to the button.
    bool m_ShowButton; ///< Flag to show/hide button
    bool m_ShowLabel; ///< Flag to show/hide label
    bool m_FlyTo; ///< Flag to activate FlyTo animation
    bool m_OnCenter; ///< Flag to set button position on center or on corner (can be refactored with a enum??)
    double bounds[6]; ///< bounds of the data related to the button
};

/////////////////////////////////////////////////////////////
// Inline methods
/////////////////////////////////////////////////////////////

inline void msvQVTKButtons::setShowButton(bool show) {
    m_ShowButton = show;
}

inline bool msvQVTKButtons::showButton() const {
    return m_ShowButton;
}

inline void msvQVTKButtons::setShowLabel(bool show) {
    m_ShowLabel = show;
}

inline bool msvQVTKButtons::showLabel() const {
    return m_ShowLabel;
}

inline bool msvQVTKButtons::FlyTo() const {
    return m_FlyTo;
}

inline void msvQVTKButtons::setOnCenter(bool onCenter) {
    m_OnCenter = onCenter;
}

inline bool msvQVTKButtons::OnCenter() const {
    return m_OnCenter;
}

inline void msvQVTKButtons::setLabel(QString &text) {
    m_Label = text;
}

inline void msvQVTKButtons::setShowTooltip(bool value) {
    if(value) {
        Q_EMIT showTooltip(m_Tooltip);
    } else {
        Q_EMIT hideTooltip();
    }
}

#endif // msvQVTKButtons_H
