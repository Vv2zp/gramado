

// todo: 
// the font support.


#include <gws.h>



int gwssrv_init_font(void)
{
    // Font
    gfontSize   = 8;
    
    
    // Fonte atual.
    gws_currentfont_address = (unsigned long) &font_nelson_cole2[0]; 
    
    //gws_currentfont_address = (unsigned long) &font_lt8x8[0];   //inverted.
    
    //...
    
    //See: nc2.h
    gws_eye_sprite_address = (unsigned long) &eye_sprite[0];

    // ...
    
    return 0;
}



