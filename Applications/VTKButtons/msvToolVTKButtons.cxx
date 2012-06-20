/*
 *  msvToolVTKButtons.cpp
 *  VTKButtons
 *
 *  Created by Roberto Mucci on 13/01/12.
 *  Copyright 2012 B3C. All rights reserved.
 *
 *  See License at: http://tiny.cc/QXJ4D
 *
 */

#include "msvToolVTKButtons.h"
#include <QImage>
#include <QDir>

#include "msvAnimateVTK.h"

#include <vtkSmartPointer.h>
#include <vtkAlgorithmOutput.h>
#include <vtkQImageToImageSource.h>

#include <vtkTextProperty.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>

#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkButtonWidget.h>
#include <vtkTexturedButtonRepresentation.h>
#include <vtkTexturedButtonRepresentation2D.h>
#include <vtkBalloonRepresentation.h>
#include <vtkCommand.h>


#include <vtkEllipticalButtonSource.h>
#include <vtkTexturedButtonRepresentation.h>

#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

// Callback respondign to vtkCommand::StateChangedEvent
class vtkButtonCallback : public vtkCommand {
public:
    static vtkButtonCallback *New() { 
        return new vtkButtonCallback; 
    }

    virtual void Execute(vtkObject *caller, unsigned long, void*) {
        msvAnimateVTK *animateCamera = new msvAnimateVTK();
        if (flyTo) {
            animateCamera->flyTo(renderer, bounds, 200);
        } else {
            renderer->ResetCamera(bounds);
        }
        if(animateCamera) {
            delete animateCamera;
        }
        //selection
    }

    void setBounds(double b[6]) {
        bounds[0] = b[0]; 
        bounds[1] = b[1];
        bounds[2] = b[2];
        bounds[3] = b[3];
        bounds[4] = b[4];
        bounds[5] = b[5];
    }

    void setFlyTo(bool fly) {
        flyTo = fly;
    }

    vtkButtonCallback():toolButton(NULL), renderer(0), flyTo(true) {}
    msvToolVTKButtons *toolButton;
    vtkRenderer *renderer;
    double bounds[6];
    bool flyTo;
};

// Callback respondign to vtkCommand::HighlightEvent
class MSV_VTKButtons_EXPORT vtkButtonHighLightCallback : public vtkCommand {
public:
    static vtkButtonHighLightCallback *New() { 
        return new vtkButtonHighLightCallback; 
    }

    virtual void Execute(vtkObject *caller, unsigned long, void*) {
        vtkTexturedButtonRepresentation2D *rep = reinterpret_cast<vtkTexturedButtonRepresentation2D*>(caller);
        int highlightState = rep->GetHighlightState();
       
        if ( highlightState == vtkButtonRepresentation::HighlightHovering && previousHighlightState == vtkButtonRepresentation::HighlightNormal ) {
            //show tooltip (not if previous state was selecting
            toolButton->showTooltip();        
        } else if ( highlightState == vtkButtonRepresentation::HighlightNormal) {
            //hide tooltip
            toolButton->hideTooltip();
        } 
        previousHighlightState = highlightState;
    }

    vtkButtonHighLightCallback():toolButton(NULL), previousHighlightState(0) {}
    msvToolVTKButtons *toolButton;
    int previousHighlightState;
        
};

msvToolVTKButtons::msvToolVTKButtons() : QObject(), m_ShowLabel(true), m_FlyTo(true), m_OnCenter(false) {
    bool loaded = false;
    VTK_CREATE(vtkTexturedButtonRepresentation2D, rep);
    rep->SetNumberOfStates(1);
    
    //Load image only the first time
    
    QImage image;
    QString iconType = "/Users/dannox/Pictures/testIcon.png";
    image.load(iconType);
    VTK_CREATE(vtkQImageToImageSource, imageToVTK);
    imageToVTK->SetQImage(&image);
    imageToVTK->Update();
    rep->SetButtonTexture(0, imageToVTK->GetOutput());
    
    int size[2]; size[0] = 16; size[1] = 16;
    rep->GetBalloon()->SetImageSize(size);

    buttonCallback = vtkButtonCallback::New();
    buttonCallback->toolButton = this;
    
    highlightCallback = vtkButtonHighLightCallback::New();
    highlightCallback->toolButton = this;

    m_ButtonWidget = vtkButtonWidget::New();
    m_ButtonWidget->SetRepresentation(rep);
    m_ButtonWidget->AddObserver(vtkCommand::StateChangedEvent,buttonCallback);
    rep->AddObserver(vtkCommand::HighlightEvent,highlightCallback);

}

msvToolVTKButtons::~msvToolVTKButtons() {
    m_ButtonWidget->Delete();
}

void msvToolVTKButtons::setCurrentRenderer(vtkRenderer *renderer) {
    if(renderer) {
        m_ButtonWidget->SetInteractor(renderer->GetRenderWindow()->GetInteractor());
        m_ButtonWidget->SetCurrentRenderer(renderer); //to check
        buttonCallback->renderer = renderer;
        m_ButtonWidget->EnabledOn();
    } else {
        m_ButtonWidget->SetInteractor(NULL);
        m_ButtonWidget->SetCurrentRenderer(NULL); //to check
        buttonCallback->renderer = NULL;
        m_ButtonWidget->EnabledOff();
    }
}

void msvToolVTKButtons::setBounds(double b[6]) {
    buttonCallback->setBounds(b);
    calculatePosition();
}

void msvToolVTKButtons::calculatePosition() {
    //modify position of the vtkButton 
    double bds[3];
    if (m_OnCenter) {
        bds[0] = (bounds[1] + bounds[0])*.5;
        bds[1] = (bounds[3] + bounds[2])*.5;
        bds[2] = (bounds[5] + bounds[4])*.5;
    } else {
        //on the corner of the bounding box of the VME.
        bds[0] = bounds[0];
        bds[1] = bounds[2]; 
        bds[2] = bounds[4];
    }
    int size[2]; size[0] = 16; size[1] = 16;
    vtkTexturedButtonRepresentation2D *rep = static_cast<vtkTexturedButtonRepresentation2D *>(m_ButtonWidget->GetRepresentation());
    
    rep->PlaceWidget(bds, size);
    rep->Modified();
    m_ButtonWidget->SetRepresentation(rep);
}

void msvToolVTKButtons::resetTool() {
    //removeWidget(m_ButtonWidget);
}

void msvToolVTKButtons::graphicObjectInitialized() {
    // Graphic widget (render window, interactor...) has been created and initialized.
    // now can add the widget.
    //addWidget(m_ButtonWidget);
}

void msvToolVTKButtons::updatePipe(double t) {
    /*mafVME *vme = input();
    if(vme == NULL) {
        return;
    }
    
    //Get absolute pose matrix from mafVMEManager
    mafObjectBase *obj = vme;
    mafMatrixPointer absMatrix = NULL;
    mafEventArgumentsList argList;
    argList.append(mafEventArgument(mafCore::mafObjectBase *, obj));
    QGenericReturnArgument ret_val = mafEventReturnArgument(mafResources::mafMatrixPointer, absMatrix);
    mafEventBusManager::instance()->notifyEvent("maf.local.resources.vme.absolutePoseMatrix", mafEventTypeLocal, &argList, &ret_val);

    //Transform bounds with absMatrix
    double b[6];
    vme->bounds(b, t);
    mafBounds *newBounds = new mafBounds(b);
    if (absMatrix) {
        newBounds->transformBounds(absMatrix);
        delete absMatrix;
        absMatrix = NULL;
    }

    ///////////-------- BUTTON WIDGET -----------////////////
    vtkTexturedButtonRepresentation2D *rep = reinterpret_cast<vtkTexturedButtonRepresentation2D*>(m_ButtonWidget->GetRepresentation());

    //Load image only the first time
    if (m_IconFileName.isEmpty()) {
        QImage image;
        QString iconType = vme->property("iconType").toString();
        m_IconFileName = mafIconFromObjectType(iconType);
        image.load(m_IconFileName);
        VTK_CREATE(vtkQImageToImageSource, imageToVTK);
        imageToVTK->SetQImage(&image);
        imageToVTK->Update();
        rep->SetButtonTexture(0, imageToVTK->GetOutput());
    }

    int size[2]; size[0] = 16; size[1] = 16;
    rep->GetBalloon()->SetImageSize(size);

    if (m_ShowLabel) {
        //Add a label to the button and change its text property
        QString vmeName = vme->property("objectName").toString();
        rep->GetBalloon()->SetBalloonText(vmeName.toAscii());
        vtkTextProperty *textProp = rep->GetBalloon()->GetTextProperty();
        rep->GetBalloon()->SetPadding(2);
        textProp->SetFontSize(13);
        textProp->BoldOff();
        //textProp->SetColor(0.9,0.9,0.9);

        //Set label position
        rep->GetBalloon()->SetBalloonLayoutToImageLeft();

        //This method allows to set the label's background opacity
        rep->GetBalloon()->GetFrameProperty()->SetOpacity(0.65);
    } else {
        rep->GetBalloon()->SetBalloonText("");
    }

    //modify position of the vtkButton 
    double bds[3];
    if (m_OnCenter) {
        newBounds->center(bds);
    } else {
        //on the corner of the bounding box of the VME.
        bds[0] = newBounds->xMin();
        bds[1] = newBounds->yMin(); 
        bds[2] = newBounds->zMin();
    }
    rep->PlaceWidget(bds, size);
    rep->Modified();
    m_ButtonWidget->SetRepresentation(rep);
    ///////////-------- BUTTON WIDGET -----------////////////

   
    buttonCallback->graphicObject = this->m_GraphicObject;
    buttonCallback->setBounds(newBounds);
    buttonCallback->setFlyTo(m_FlyTo);
    updatedGraphicObject();*/
}

void msvToolVTKButtons::showTooltip() {
    /*mafVME *vme = input();
    if(vme == NULL) {
        return;
    }

    QString matrixString = vme->dataSetCollection()->itemAtCurrentTime()->poseMatrixString();
    QString text("<b>Data type</b>: ");
    text.append(vme->dataSetCollection()->itemAtCurrentTime()->externalDataType());
    text.append("<br>");

     QStringList list = matrixString.split(" ");
     int numElement = list.count();
     int i = 0;

     text.append("<b>Pose Matrix</b>:");
     text.append("<table border=\"0.2\">");
     for ( ; i < numElement; i++ ) {
         text.append("<tr>");
         text.append("<td>" + list[i] +"</td>");
         i++;
         text.append("<td>" + list[i] +"</td>");
         i++;
         text.append("<td>" + list[i] +"</td>");
         i++;
         text.append("<td>" + list[i] +"</td>");
         text.append("</tr>");
     }
     text.append("</table>");

    mafBounds *bounds = vme->dataSetCollection()->itemAtCurrentTime()->bounds();
    text.append("<b>Bounds: (min - max)</b>:");
    text.append("<table border=\"0.2\">");
    text.append("<tr>");
    text.append("<td>" + QString::number(bounds->xMin()) +"</td>");
    text.append("<td>" + QString::number(bounds->xMax()) +"</td>");
    text.append("</tr>");
    text.append("<tr>");
    text.append("<td>" + QString::number(bounds->yMin()) +"</td>");
    text.append("<td>" + QString::number(bounds->yMax()) +"</td>");
    text.append("</tr>");
    text.append("<tr>");
    text.append("<td>" + QString::number(bounds->zMin()) +"</td>");
    text.append("<td>" + QString::number(bounds->zMax()) +"</td>");
    text.append("</tr>");
    text.append("</table>");
 
    mafEventBus::mafEventArgumentsList argList;
    argList.append(mafEventArgument(QString , text));
    mafEventBus::mafEventBusManager::instance()->notifyEvent("maf.local.gui.showTooltip", mafEventBus::mafEventTypeLocal, &argList);*/
}

void msvToolVTKButtons::hideTooltip() {
    //mafEventBus::mafEventArgumentsList argList;
    //mafEventBus::mafEventBusManager::instance()->notifyEvent("maf.local.gui.hideTooltip", mafEventBus::mafEventTypeLocal, &argList);
}

/*void msvToolVTKButtons::selectVME() {
    //select the VME associated to the button pressed
    mafVME *vme = input();
    if(vme == NULL) {
        return;
    }
    mafCore::mafObjectBase *objBase = vme;
    mafEventArgumentsList argList;
    argList.append(mafEventArgument(mafCore::mafObjectBase*, objBase));
    mafEventBusManager::instance()->notifyEvent("maf.local.resources.vme.select", mafEventTypeLocal, &argList);
}*/
