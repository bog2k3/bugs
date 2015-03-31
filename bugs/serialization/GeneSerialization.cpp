/*
 * GeneSerialization.cpp
 *
 *  Created on: Mar 31, 2015
 *      Author: bog
 */

#include "BinaryStream.h"
#include "../genetics/Gene.h"

BinaryStream& operator << (BinaryStream &stream, Gene const& gene) {
	stream << (uint32_t)gene.RID << (uint32_t)gene.type;
	stream << gene.chance_to_delete;
	stream << gene.chance_to_swap;
	switch (gene.type) {
	case GENE_TYPE_BODY_ATTRIBUTE:
		break;
	default:
		assert(false); // unknown gene type
	}
}

BinaryStream& operator >> (BinaryStream &stream, Gene &gene) {

}
