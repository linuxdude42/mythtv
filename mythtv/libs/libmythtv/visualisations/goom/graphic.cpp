#include "graphic.h"

const Color BLACK = { 0, 0, 0 };
const Color WHITE = { 0xff, 0xff, 0xff };
const Color RED = { 0xff, 0, 0 };
const Color GREEN = { 0, 0xff, 0 };
const Color BLUE = { 0, 0, 0xff };
const Color YELLOW = { 0xff, 0xff, 0x33 };
const Color ORANGE = { 0xff, 0xcc, 0x00 };
const Color VIOLET = { 0x55, 0x00, 0xff };

int    *rand_tab = nullptr;

namespace {
    //unsigned int SIZE;
    //unsigned int HEIGHT;
    //unsigned int WIDTH;

    //unsigned short int rand_pos = 0;
    /*
    inline unsigned int RAND(void)
    {
        rand_pos++;
        return rand_tab[rand_pos];
    }

    inline unsigned int iRAND(int i)
    {
        rand_pos++;
        return (rand_tab[rand_pos])%i;
    }
    */
}
