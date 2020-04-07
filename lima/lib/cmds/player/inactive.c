/* Do not remove the headers from this file! see /USAGE for more info. */

// Jan 10, 94 by Rust
// No longer works
// Megaboz@ZorkMUD added help

//:PLAYERCOMMAND
//USAGE:	inactive
//
//Puts you into inactive mode. This lets people know that you are
//idling.  Hitting return takes you out of it when you are in it.

#include <playerflags.h>

inherit CMD;
inherit M_INPUT;

private nomask void become_active(int start_time, string str)
{
  int num;
  int hours, minutes;

  num = time() - start_time;
  if(num>359)
    hours = num/3600;
  num -= hours*3600;
  if(num>59)
    minutes = num/60;
  num -= minutes*60;
  printf("You return to ACTIVE status after ");
  if(hours)
     printf("%d hours, ",hours);
  if(minutes)
     printf("%d minutes, ",minutes);
  printf("%d seconds.\n",num);
  this_body()->other_action("$N $vreturn to ACTIVE status.");
  this_body()->clear_flag(F_INACTIVE);
}


nomask private void main(string arg)
{
  out("You place yourself in INACTIVE mode.  Press <ENTER> to return.\n");
  this_body()->other_action("$N $vgo into INACTIVE mode.");
  this_body()->set_flag(F_INACTIVE);
  modal_simple((: become_active, time() :));
}


nomask int help()
{
  printf("USAGE:  inactive\n"+
    "This is the best way for you to let people that you are away from your keyboard.  You just have to hit the enter key when you return, and you will leave off where you left off, unless you, of course, exceed the limit for idling.  In which case, you won't.  =->"+"\n");
  return 1;
}