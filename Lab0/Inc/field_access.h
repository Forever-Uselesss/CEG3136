/*
 * field_access.h
 *
 *  Created on: Sep 15, 2025
 *      Author: jthoz081
 */

#ifndef FIELD_ACCESS_H_
#define FIELD_ACCESS_H_

#define CLEAR_FIELD(REG, FLD) ((REG) &= ~(FLD))
#define SET_FIELD(REG, FLD) ((REG) |= (FLD))

// For pre-aligned value AL_VAL
#define MODIFY_FIELD_AL(REG, FLD, AL_VAL) ((REG) = ((REG) & ~(FLD)) | AL_VAL)


// For unaligned value VAL
#define MODIFY_FIELD(reg, field, value) \
((reg) = ((reg) & ~(field ## _Msk)) | \
(((uint32_t)(value) << field ## _Pos) & field ## _Msk))




#endif /* FIELD_ACCESS_H_ */
