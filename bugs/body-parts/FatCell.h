/*
 * FatCell.h
 *
 *  Created on: Jan 3, 2018
 *      Author: bog
 */

#ifndef BODY_PARTS_FATCELL_H_
#define BODY_PARTS_FATCELL_H_

#include "BodyPart.h"

class FatCell: public BodyPart {
public:
	FatCell(BodyPartContext const& context, BodyCell& cell);
	virtual ~FatCell();
};

#endif /* BODY_PARTS_FATCELL_H_ */
