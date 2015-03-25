/*
 * GuiTheme.cpp
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#include "GuiTheme.h"
#include "DefaultTheme.h"

std::shared_ptr<GuiTheme> GuiTheme::activeTheme_ = std::make_shared<DefaultTheme>();
