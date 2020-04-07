/* Do not remove the headers from this file! see /USAGE for more info. */

inherit OBJ;
inherit M_GETTABLE;

void setup()
{
  set_id("maps");
  set_long("They have scribblings all over them.");
  set_attached();
  set_gettable("#The maps are securely fastened to the cave wall.");
#ifdef USE_SIZE
  set_size(SMALL);
#endif
#ifdef USE_MASS
  set_mass(SMALL);
#endif
}
