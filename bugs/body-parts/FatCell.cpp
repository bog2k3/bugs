/*
 * FatCell.cpp
 *
 *  Created on: Jan 3, 2018
 *      Author: bog
 */

#include "FatCell.h"
#include "BodyConst.h"

FatCell::~FatCell() {
	// TODO Auto-generated destructor stub
}

FatCell::FatCell(BodyPartContext const& context, BodyCell& cell)
	: BodyPart(BodyPartType::FAT, context, cell)
{
	// TODO Auto-generated constructor stub

}

float FatCell::getDensity(BodyCell const& cell) {
	return BodyConst::FatDensity;
}

