/* Do not remove the headers from this file! see /USAGE for more info. */

inherit OBJ;
inherit M_READABLE;
inherit M_GETTABLE;

void mudlib_setup()
{
    set_id("sign");
    set_gettable( "#It's best left alone." );
}
