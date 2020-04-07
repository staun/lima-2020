/* Do not remove the headers from this file! see /USAGE for more info. */

#ifdef USE_BODYSLOTS
#include <bodyslots.h>
#endif

inherit CONTAINER;
inherit M_OPENABLE;
inherit M_WEARABLE;
inherit M_GETTABLE;

void setup()
{
  set_id("backpack", "pack");
  set_long("It's a simple leather backpack, with a flap and strap allowing you to close it.");
  add_relation("in");
  set_default_relation("in");
  set_objects( ([
    ]) );
  set_max_capacity(3*MEDIUM);
#ifdef USE_SIZE
  set_size(SMALL);
#endif
#ifdef USE_MASS
  set_mass(SMALL);
#endif
#ifdef USE_BODYSLOTS
  set_slot( TORSO );
#endif
}

mixed ob_state()
{
  return (string) container::ob_state() + (string) m_wearable::ob_state();
}

	    
void remove()
{
  m_wearable::remove();
  container::remove();
}
