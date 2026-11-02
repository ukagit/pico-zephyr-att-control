#include "att_codec.h"
#include "att_config.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
int main(void)
{
    for (unsigned db=0; db<=127; ++db) {
        uint8_t m=0; assert(att_encode(db,&m)==0); assert(att_decode(m)==db);
        uint8_t repeat; assert(att_encode(db,&repeat)==0); assert(repeat==m);
    }
    for (unsigned m=0; m<256; ++m) assert(att_decode((uint8_t)m)<=127);
    uint8_t m=42;
    assert(att_encode(128,&m)<0 && m==42);
    assert(att_encode(UINT_MAX,&m)<0);
    assert(att_encode(1,NULL)<0);
    const char *bad[]={"","-1","+1","1.5","128","12junk"," 12","12 ","999999999999999999999"};
    for(unsigned i=0;i<sizeof(bad)/sizeof(bad[0]);++i) {unsigned db=999;assert(att_parse_db(bad[i],&db)<0 && db==999);}
    for (unsigned db=0;db<=127;++db) {char s[8];unsigned got;snprintf(s,sizeof(s),"%u",db);assert(!att_parse_db(s,&got) && got==db);}
    const unsigned pins[] = ATT_GPIO_PINS;
    const unsigned expected_pins[] = {18,19,20,21,22,26,27,28};
    const unsigned expected_db[] = {32,1,2,32,4,8,16,32};
    unsigned selected = 0;
    for (unsigned i=0;i<ATT_CELL_COUNT;++i) {
        assert(pins[i]<30 && !(selected & (1u<<pins[i])));
        selected |= 1u<<pins[i];
    }
    for (unsigned db=0;db<=127;++db) {
        uint8_t m; assert(!att_encode(db,&m));
        unsigned physical=0, sum=0;
        for(unsigned i=0;i<ATT_CELL_COUNT;++i)
            if(m & (1u<<i)) physical |= 1u<<pins[i];
        for(unsigned i=0;i<8;++i)
            if(physical & (1u<<expected_pins[i])) sum += expected_db[i];
        assert(sum==db);
    }
    puts("PASS: 128 attenuation values, 256 masks, stable mapping, strict input bounds, GPIO mapping");
}
