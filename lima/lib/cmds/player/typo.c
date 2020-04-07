/* Do not remove the headers from this file! see /USAGE for more info. */

/*
** typo.c
**
** Converted to use new /std/reporter superclass (Deathblade 4-Sep-94)
** Original by Rust
** Rust made it post news.
*/

//:PLAYERCOMMAND
//$$ see: bug, idea, feedback, question
//USAGE:  typo
//
//This command directs a typo report to the proper place.

inherit CMD;

void create()
{
  ::create();
  no_redirection();
}

private void main(string str)
{
  REPORTER_D->report_something("Typo", str);
}

void player_menu_entry()
{
  bare_init();  
  main("");
  done_outputing();
}
