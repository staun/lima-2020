/* Do not remove the headers from this file! see /USAGE for more info. */

inherit OBJ;
inherit M_DIGGER;
inherit M_GETTABLE;
inherit M_INPUT;

void setup()
{
  set_adj("plastic");
  set_id("shovel");
  set_long ("It's a plastic shovel.");
#ifdef USE_SIZE
  set_size (SMALL);
#endif
#ifdef USE_MASS
  set_mass (SMALL);
#endif
}
