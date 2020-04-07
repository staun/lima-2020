/* Do not remove the headers from this file! see /USAGE for more info. */

inherit INDOOR_ROOM;

void setup(int x, int y, int north, int east, int south, int west) {
    string *dirs = ({ });

    if (north == ' ') {
	add_exit("north", __DIR__ + "labyrinth/" + x + "," + (y-1));
	dirs += ({ "north" });
    }
    if (east == ' ') {
	add_exit("east", __DIR__ + "labyrinth/" + (x+1) + "," + y);
	dirs += ({ "east" });
    }
    if (south == ' ') {
	add_exit("south", __DIR__ + "labyrinth/" + x + "," + (y+1));
	dirs += ({ "south" });
    }
    if (west == ' ') {
	add_exit("west", __DIR__ + "labyrinth/" + (x-1) + "," + y);
	dirs += ({ "west" });
    }
    set_brief("Labyrinth");
    set_long("You are in a small corridor in a large labyrinth.  "+ (sizeof(dirs) > 1 ? "Passages lead " : "A passage leads ") +  format_list(dirs)+".\n");
    set_default_error( "Cold stone bars your way.\n");
}
     
	    
