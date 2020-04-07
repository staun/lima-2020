/* Do not remove the headers from this file! see /USAGE for more info. */

/*
** Force another user to perform a command
**
** 950930, Ranma@Koko Wa: added logging
** ???: created by somebody :-)
*/

//:COMMAND
//USAGE:  force <living> <command>
//
//This command will force a player to execute the command you supply
//
//force ohara north
//
//This would force Ohara to move to the north if possible.
//Ohara would like you to note that "possible" and "a good idea"
//are two separate things.
//
//
//***********  WARNING  ************
//
//The admin of this mud will not tolerate any type of toying with players.
//Do not force a player into a certain death situation.
//Doing this is a direct violation of the mud policy and is grounds for
//disciplinary action.
//
//All uses of the command are logged.
//
//Ohara also notes that any toying with him may result in a
//force <insert your name here> dest <insert your name here too>

#include <log.h>

inherit CMD;

void create()
{
  ::create();
  no_redirection();
}

private void main(string orig_input)
{
  int space = strsrch(orig_input, " ");
  string what = orig_input[space+1..];
  object targ_user = find_user(orig_input[0..space-1]);
  string s;

  if ( !targ_user )
  {
    printf("No such user: %s\n", orig_input[0..space-1]);
    return;
  }

  tell(targ_user, this_body()->query_name()+" forced you to: "+what+"\n");
  write("You forced "+targ_user->query_body()->query_name()+" to: "+what+"\n");

  s = sprintf("%s forces %s to (%s) [%s]\n",
      this_user()->query_userid(), targ_user->query_userid(),
      what, ctime(time()));
  LOG_D->log(LOG_FORCE, s);

  targ_user->force_me(what);
}