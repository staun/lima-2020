/* Do not remove the headers from this file! see /USAGE for more info. */

inherit RACE;

/*
 * This is the name of the race.  It is used in the initial selection screen,
 * and also in who info etc.
 */
string query_race()
{
    return "troll";
}

/*
 * This is the description shown when the user types 'help race' during the
 * selection process
 */
string short_description()
{
    return @LONG
Trolls are extremely strong and powerful creatures; probably the toughest
race available. Their skin is extremely thick and conventional means of
damage do not help much in killing them. Trolls aren't very intelligent
or quick on their feet, but have an amazingly high willpower and just
don't know when to quit. They are not exactly social outcasts, but tend
not to do very well politically. They are definitely a physical force to
be reckoned with though, unless you carry a torch. Trolls have a terrible
weakness for fire and are highly susceptible to its flames. Trolls often
live in dark, wet areas (such as under bridges!) where they can keep
their skin moist. Trolls do not band together in tribes or form communities
and so they can come from any part of the land. Their chief deities are
those related to water and nature.
LONG;
}

/*
 * These are the bonuses for the derived statistics.  If this is zero, the
 * Normal derivation is used, however, if a bonus exists, then the stat
 * is scaled into that range.
 *
 * For example, consider a race with a racial con bonus of 50.
 * 
 * The derived constitution is a weighted average of 3 base stats.  This
 * gives a number in the range [0..100].  The racial bonus says that that
 * number should be adjusted into the range [50..100].  This has three effects:
 *
 * (1) All members of the race will have at least 50 constitution.
 * (2) 100 is the max for all races.
 * (3) Races that start out with a bonus advance slower (since the range is
 *     compressed).  For example, if your base stats go up by 10, then your
 *     con would only increase by 5.
 */
#ifdef USE_STATS
int racial_con_bonus() {
    return 10;
}

int racial_wis_bonus() {
    return 0;
}

int racial_cha_bonus() {
    return 0; // Documentation is a beautiful thing :-)
}

class stat_roll_mods query_roll_mods() {
    class stat_roll_mods ret = new(class stat_roll_mods);
    
/*
 * These give the adjustments and ranges for base statistics.
 * 
 * The actual score is in the range:
 *
 * BASE_VALUE + adjust +- range/2
 *
 * where BASE_VALUE is in /std/modules/bodystat.c (currently 20)
 *
 * Here are some examples:
 *
 * adjust        range         possible values
 * -10		  20		 0 .. 20
 * -15		   5		 3 .. 8
 *  25		  30		30 .. 60
 *   5            10		20 .. 30
 *   0		  40		 0 .. 40
 */
    ret->str_adjust = 10;
    ret->str_range = 20;
    
    ret->agi_adjust = -5;
    ret->agi_range = 10;
    
    ret->int_adjust = -10;
    ret->int_range = 10;
    
    ret->wil_adjust = 5;
    ret->wil_range = 10;

    return ret;
}
#endif
/*
 * Note that this object is used as the player's body object, so you can
 * overload any player functions below.
 */
void create(string userid) {
    ::create(userid);
    set_to_hit_bonus(0);
    //#if COMBAT_STYLE == COMBAT_TRADITIONAL
    set_weapon_class(10);
    //#endif
}

#if 0
void intrinsic_resistance(class combat_result result) {
     if (result->kind == "fire" && result->damage)
         result->damage *= 2;
     else
         result->damage--;
}
#endif

class event_info health_modify_event(class event_info evt)
{
   if(!arrayp(evt->data))
      return evt;
   if(member_array("fire", evt->data) != -1)
      evt->data[sizeof(evt->data)-1] *= 2;
   else
      evt->data[sizeof(evt->data)-1]--;
   return evt;
}
