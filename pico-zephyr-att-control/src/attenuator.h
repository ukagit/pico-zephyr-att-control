#pragma once
#include <stddef.h>
int att_init(void);
int att_set(unsigned db);
int att_json(char *out, size_t size);
