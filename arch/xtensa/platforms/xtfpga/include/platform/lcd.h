/*
 * arch/xtensa/platform/xtavnet/include/platform/lcd.h
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2001, 2006 Tensilica Inc.
 */

#ifndef __XTENSA_XTAVNET_LCD_H
#define __XTENSA_XTAVNET_LCD_H

/*                                                */
void lcd_disp_at_pos(char *str, unsigned char pos);

/*                                                      */
void lcd_shiftleft(void);
void lcd_shiftright(void);
#endif
