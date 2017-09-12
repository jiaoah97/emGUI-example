#ifndef __GUI_H
#define __GUI_H

#include "emGUI.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <iostream>

using namespace Gdiplus;

#define SCREEN_WIDTH LCD_SizeX
#define SCREEN_HEIGHT LCD_SizeY

//events

ARGB convertColor(uint16_t color);
bool bGUI_InitInterfce();
bool bGUIOnWindowCloseHandlerMain(xWidget *);
bool bGUIOnWindowCloseHandler(xWidget *);
void vGUIsetCurrentHDC(Graphics * gr);
void vGUIpaintEventHandler();
void vGUIpushClickHandler(LPARAM lParam);
void vGUIpopClickHandler(LPARAM lParam);
void vGUIeraseBackgroudHandler();

void vGUIWriteToLabel(char * msg);

xPlotData_t * pxGUIGetPlotData();

#endif