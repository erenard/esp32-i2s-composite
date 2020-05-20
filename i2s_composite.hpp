#pragma once
#include <driver/i2s.h>

// long names
void i2s_composite_init(int);
void i2s_composite_write_line(unsigned short * line);

// qualified names
extern struct i2s_composite_namespace {
     void (*init)(int);
     void (*write_line)(unsigned short * line);
} i2s_composite;

