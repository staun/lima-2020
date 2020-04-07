/* Do not remove the headers from this file! see /USAGE for more info. */

// Goto - Nevyn@Woodstock, @GUE

//:COMMAND
//$$ see: wizz, trans
//USAGE: goto <player>
//       goto <room>
//The goto command is a simple method of teleportation which can move you
//either to a specific room or to whatever room a certain player is in.
//
//If the room is in your current directory,
//the filename indicates the destination.
//
//eg	goto workroom.c
//
//If the room is not in your current directory,
//the full path (/directory/filename) format should be used.
//
//eg	goto /wiz/azy/workroom.c

#include <move.h>
#include <security.h>

inherit CMD;

private void main( mixed *arg)
{
  mixed rt;
  string *msgs;
  object prev,env;

  prev = environment(this_body());

  if ( environment(arg[0]) )
    arg[0] = environment(arg[0]);
  rt = this_body()->move(arg[0]);
  env = environment(this_body());

  if ( stringp(rt) )
    out(rt);
  else
  {
    if (rt == MOVE_OK)
    {
      if (prev==env)
      {
        out("You twitch briefly.\n");
        return;
      }
      msgs = this_body()->get_player_message("mleave");
      tell_from_inside(prev, msgs[1]);
      msgs = this_body()->get_player_message("menter");
      tell_from_inside(env, msgs[1], 0, ({ this_body() }));
      this_body()->force_look();
    }
    else           // failure messages
      switch(rt)
      {
        case MOVE_NO_DEST:
          out("Goto: Invalid destination.\n");
          break;

        case MOVE_NOT_RELEASED:
          out("Goto: Your surroundings will not let you leave!\n");
          break;

        case MOVE_NOT_ALLOWED:
          out("Goto: Your destination will not accept you.\n");
          break;
      }
  }
}
