/*
 *  msvQVTKButtonsGroup.cpp
 *  mafPluginVTK
 *
 *  Created by Alberto Losi on 08/08/12.
 *  Copyright 2009 B3C.s All rights reserved.
 *
 *  See License at: http://tiny.cc/QXJ4D
 *
 */

#include "msvQVTKButtonsGroup.h"
#include "msvQVTKButtons.h"

msvQVTKButtonsGroup::msvQVTKButtonsGroup(QObject *parent) : msvQVTKButtonsInterface() {

}

void msvQVTKButtonsGroup::setElementProperty(QString name, QVariant value) {
  for(QVector<msvQVTKButtonsInterface*>::iterator buttonsIt = m_Elements.begin(); buttonsIt != m_Elements.end(); buttonsIt++)
  {
    (*buttonsIt)->setProperty(name.toStdString().c_str(),value);
  }
}

void msvQVTKButtonsGroup::setCurrentRenderer(vtkRenderer *renderer) {
  for(QVector<msvQVTKButtonsInterface*>::iterator buttonsIt = m_Elements.begin(); buttonsIt != m_Elements.end(); buttonsIt++)
  {
    (*buttonsIt)->setCurrentRenderer(renderer);
  }
}

void msvQVTKButtonsGroup::addElement(msvQVTKButtonsInterface* buttons) {
  for(QVector<msvQVTKButtonsInterface*>::iterator buttonsIt = m_Elements.begin(); buttonsIt != m_Elements.end(); buttonsIt++)
  {
    if(*buttonsIt == buttons)
    {
      return;
    }
  }
  m_Elements.push_back(buttons);
}

void msvQVTKButtonsGroup::removeElement(msvQVTKButtonsInterface* buttons) {
  int index = 0;
  for(QVector<msvQVTKButtonsInterface*>::iterator buttonsIt = m_Elements.begin(); buttonsIt != m_Elements.end(); buttonsIt++)
  {
    if(*buttonsIt == buttons)
    {
      m_Elements.remove(index);
      return;
    }
    index++;
  }
}

msvQVTKButtonsGroup::~msvQVTKButtonsGroup() {

}

msvQVTKButtonsInterface* msvQVTKButtonsGroup::getElement(int index) {
  if(index > m_Elements.size() - 1) {
    return NULL;
  }
  return m_Elements.at(index);
}

msvQVTKButtonsGroup* msvQVTKButtonsGroup::createGroup() {
  msvQVTKButtonsGroup* element = new msvQVTKButtonsGroup();
  addElement(element);
  return element;
}

msvQVTKButtons* msvQVTKButtonsGroup::createButtons() {
  msvQVTKButtons* element = new msvQVTKButtons();
  addElement(element);
  return element;
}