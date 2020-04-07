/* Do not remove the headers from this file! see /USAGE for more info. */

// Rust

//:COMMAND
//USAGE:  start
//        start <path>
//
//This command will show you where you are currently starting
//or allow you to change your start location if you supply it with a path.
//
//start
//You start at: /wiz/zifnab/workroom
//
//start /domains/std/wizroom
//OK, you now start at: /domains/std/wizroom

inherit CMD;

private void main(mixed *arg)
{
  string where;
  if (!arg[0])
  {
    outf("You start at: %s\n", this_body()->query_start_location() );
    return;
  }
  if( arg[0]->is_living() )
  {
    out("Yeah, right.\n");
    return;
  }
  where = file_name(arg[0]);
  this_body()->set_start_location( where );
  outf("Ok, you now start at: %s.\n", where );
  return;
}

