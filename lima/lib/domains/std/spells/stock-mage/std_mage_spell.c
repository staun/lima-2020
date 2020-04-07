/* Do not remove the headers from this file! see /USAGE for more info. */

/*
** std_mage_spell.c -- standard spell superclass for "stock mages"
**
** Adds delays from casting to effect and adds the requirement that the
** caster must be in the "stock-mage" guild.
*/

#include <assert.h>

inherit SPELL;


private nosave int cast_time;

void do_effects(object target, object reagent);

void setup ()
{
    set_spell_name("standard mage spell");
}

int set_cast_time(int t)
{
    ASSERT(t>=0);
    cast_time = t;
}

int get_spell_level()
{
    return 1000;  // They should probably redefine this function....
}

// Don't allow spells to redefine this now.
// This should drain spell points, etc..
nomask mixed valid_circumstances(object target, object reagent)
{
    return this_body()->query_member_guild("stock-mage") >= get_spell_level();
}

nomask void cast_spell(object target, object reagent)
{
    write("You begin casting...\n");
    call_out((: do_effects :), cast_time, target, reagent);
}
