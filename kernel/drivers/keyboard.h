/*
 * kernel/drivers/keyboard.h
 * PS/2 keyboard driver (scancode set 1, US QWERTY).
 */
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../include/kernel.h"

#define KB_BUF_SIZE 256

void     keyboard_init(void);
bool     keyboard_has_char(void);
char     keyboard_getchar(void);    /* blocks until a key is available */

#endif /* KEYBOARD_H */
