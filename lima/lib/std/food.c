/* Do not remove the headers from this file! see /USAGE for more info. */

inherit OBJ;
inherit M_EDIBLE;
inherit M_GETTABLE;

void mudlib_setup()
{
  object::mudlib_setup();
  m_edible::mudlib_setup();
}

mixed direct_eat_obj( object ob )
{
  return m_edible::direct_eat_obj( ob );
}

int is_food(){  return 1;}

string get_extra_long()
{
    switch ((100 * get_num_eats())/ get_original_eats())
    {
    case 1..20: return "It is almost finished.";
    case 21..40: return "Most of it has been eaten.";
    case 41..60: return "It is about half eaten.";
    case 61..70: return "It has been partially eaten.";
    case 71..90: return "A little of it has been eaten.";
    case 91..99: return "It has hardly been touched.";
    case 100: return "It is untouched.";
    }
}
