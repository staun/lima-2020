/* Do not remove the headers from this file! see /USAGE for more info. */

inherit ROOM;


void setup() 
{
  set_area("pirate");
  set_brief("Sloping tunnel");

  set_long("Nothing yet.\n");
  set_light(0);
  set_exits( ([
	       "south" : "Inside_Cave",
	       ]) );
  set_hidden_exits();
  set_default_exit ("You can't find an exit in that direction.\n");

  set_objects( ([

		 ]) );
}


