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

msvQVTKButtonsManager::msvQVTKButtonsManager(QObject *parent) : msvQVTKButtonsGroup() {

}

msvQVTKButtonsManager* msvQVTKButtonsManager::instance() {
  static msvQVTKButtonsManager manager;
  return &manager;
}

msvQVTKButtonsManager::~msvQVTKButtonsManager() {

}
