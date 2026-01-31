// Display driver
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include "display.h"
#include "i2c.h"
#include "systick.h"
// --------------------------------------------------------
// Display controller
// --------------------------------------------------------
#define ROWS 2 // Number of rows
#define COLS 16 // Number of columns
// Command word
typedef struct {
	uint8_t ctrl; // Control byte
	uint8_t data; // Data byte
} DispCmd_t;

// Initialization command sequence
static const DispCmd_t txInit[] = { { 0x80, 0x02C }, // Function Set: 2-line, display OFF
		{ 0x80, 0x0C }, // Display Control: display ON, cursor OFF, blink OFF
		{ 0x80, 0x01 }, // Display Clear
		{ 0x80, 0x06 } // Entry Mode Set: increment, no shift
};

// Display line
typedef struct {
	DispCmd_t cmd; // Command word to set display line
	uint8_t ctrl; // Last control byte, data bytes to follow
	uint8_t text[COLS]; // ASCII text to write to display
	uint8_t term; // NUL terminator
} DispLine_t;
// Select display line and print text
// (separate transfers for lines 1 and 2, each read left to right)
static DispLine_t txLine[ROWS] = { { { 0x80, 0x80 }, 0x40, { 0 }, 0x00 }, { {
		0x80, 0xC0 }, 0x40, { 0 }, 0x00 } };
static bool updateLine[2] = { false, false };
// I2C transfers
static I2C_Xfer_t DispInit = { &LeafyI2C, 0x7C, (void*) &txInit, 8, 1, 0, NULL };
static I2C_Xfer_t DispLine[ROWS] = { { &LeafyI2C, 0x7C, (void*) &txLine[0], 19,
		1, 0, NULL }, { &LeafyI2C, 0x7C, (void*) &txLine[1], 19, 1, 0, NULL } };
// Enable LCD display
void DisplayEnable(void) {
	I2C_Enable(LeafyI2C);
	I2C_Request(&DispInit);
}
// Print a line of text with optional format specifiers
void DisplayPrint(const int line, const char *msg, ...) {
	va_list args;
	va_start(args, msg);
	// Full buffer with formatted text and space pad the remainder
	int chars = vsnprintf((char*) txLine[line].text, COLS + 1, msg, args);
	for (int i = chars; i < COLS; i++)
		txLine[line].text[i] = ' ';
	updateLine[line] = true;
}
// --------------------------------------------------------
// Backlight controller
// --------------------------------------------------------
typedef struct {
	uint8_t addr; // Address byte
	uint8_t data; // Data byte
} BltCmd_t;
// Transmit data buffers to set brightness of each LED, all off by default
static BltCmd_t txRed = { 0x01, 0x00 };
static BltCmd_t txGreen = { 0x02, 0x00 };
static BltCmd_t txBlue = { 0x03, 0x00 };
static bool updateBlt = true;
// I2C transfers
static I2C_Xfer_t BltRed = { &LeafyI2C, 0x5A, (void*) &txRed, 2, 1, 0, NULL };
static I2C_Xfer_t BltGreen =
		{ &LeafyI2C, 0x5A, (void*) &txGreen, 2, 1, 0, NULL };
static I2C_Xfer_t BltBlue = { &LeafyI2C, 0x5A, (void*) &txBlue, 2, 1, 0, NULL };

// Set new backlight color
void DisplayColor(Color_t color) {
	txRed.data = (color >> 16) & 0xFF;
	txGreen.data = (color >> 8) & 0xFF;
	txBlue.data = (color >> 0) & 0xFF;
	updateBlt = true;
}
// --------------------------------------------------------
// Automatic background updates
// --------------------------------------------------------
// Called from main loop
void UpdateDisplay(void) {
	for (int i = 0; i < ROWS; i++)
		if (!DispLine[i].busy && updateLine[i]) {
			updateLine[i] = false;
			I2C_Request(&DispLine[i]);
		}
	if (!BltBlue.busy && updateBlt) {
		updateBlt = false;
		I2C_Request(&BltRed);
		I2C_Request(&BltGreen);
		I2C_Request(&BltBlue);
	}
}
