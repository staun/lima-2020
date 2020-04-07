/* Do not remove the headers from this file! see /USAGE for more info. */

//:PLAYERCOMMAND
//Usage: exits
//Used with no arguments, this command shows you the possible exits for
//the particular room you are in.

inherit CMD;

private void main( string message )
{
  string *exits = environment(this_body())->query_exit_directions( 0 );
  string str = "You can leave ";

  switch( sizeof(exits))
  {
    case 0:
      out("You can't see any exits.\n");
      return;
    default:
      str += format_list(exits);
  }
  str += ".\n";
  out( str );
  return;
}
