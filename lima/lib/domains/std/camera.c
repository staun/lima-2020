/* Do not remove the headers from this file! see /USAGE for more info. */
// Written by Beek.
// Retool by --OH.


inherit CONTAINER;
inherit M_GETTABLE;


void setup()
{
    set_id( "camera" );
    set_adj( "polaroid" );
    set_long( "It's a polaroid camera with a button on top." );
    set_relations("in");
    set_default_relation("in");
    set_objects( ([ "camera/button" : 1, ]));
    set_hide_contents( 1 );
}


void take_picture()
{

    this_body()->simple_action("$N $vtake a picture with $p $o.\nA polaroid picture slides out the front of $o.", this_object());
    give_new_obj( environment(), __DIR__ "camera/picture", 1 );
}


mapping lpscript_attributes()
{
    return container::lpscript_attributes() + m_gettable::lpscript_attributes();
}
