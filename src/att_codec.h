#pragma once
#include <stdint.h>
int att_encode(unsigned db, uint8_t *mask);
unsigned att_decode(uint8_t mask);
int att_parse_db(const char *text, unsigned *db);
