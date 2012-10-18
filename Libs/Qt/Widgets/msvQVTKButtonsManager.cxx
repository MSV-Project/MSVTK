/*
 *  msvQVTKButtonsManager.cpp
 *  mafPluginVTK
 *
 *  Created by Alberto Losi on 08/08/12.
 *  Copyright 2009 B3C.s All rights reserved.
 *
 *  See License at: http://tiny.cc/QXJ4D
 *
 */

#include "msvQVTKButtonsManager.h"

msvQVTKButtonsManager::msvQVTKButtonsManager(QObject *parent) {

}

msvQVTKButtonsManager* msvQVTKButtonsManager::instance() {
  static msvQVTKButtonsManager manager;
  return &manager;
}

msvQVTKButtonsManager::~msvQVTKButtonsManager() {

}

void msvQVTKButtonsManager::setElementProperty(QString name, QVariant value) {
  for(QVector<msvQVTKButtonsInterface*>::iterator buttonsIt = m_Elements.begin(); buttonsIt != m_Elements.end(); buttonsIt++)
  {
    (*buttonsIt)->setProperty(name.toStdString().c_str(),value);
  }
}
