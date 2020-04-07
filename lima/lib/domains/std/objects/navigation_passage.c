/* Do not remove the headers from this file! see /USAGE for more info. */

inherit OBJ;

void setup() 
{
  set_id("passage");
  set_id("outline");
  set_long("There is nothing special about the passage.");
  set_attached(1);
#ifdef USE_SIZE
  set_size(MEDIUM);
#endif
#ifdef USE_MASS
  set_mass(MEDIUM);
#endif
}









