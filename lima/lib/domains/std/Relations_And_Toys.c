/* Do not remove the headers from this file! see /USAGE for more info. */

// Beek - a simple room to demonstrate relationships and some toys

inherit ROOM;

void setup()
{
    set_area("wiz_area");

    set_brief("Relations and Toys");
    set_long("Guess what?  It's yet another example room.  The first example room is back to the north.  To the south is an ominous looking cave.  To the east, it looks like the weather changes alot.");
    set_exits( ([
	"north" : "Example_Room1",
	"east" : "Weather_Room",
	"south" : "cult/Foyer",
      ]) );
    set_objects( ([
	"/domains/std/harry" : 1
      ]) );
}

