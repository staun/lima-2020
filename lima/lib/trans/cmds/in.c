/* Do not remove the headers from this file! see /USAGE for more info. */

//:COMMAND
//$$ see: at
//USAGE at <living> <command>
//
//This command allows you to remotely perform a command
//as if you were standing next to the person.
//
//in beek tickle beek
//
//Beek will see:  Zifnab tickles you.

inherit CMD;

void create()
{
  ::create();
  no_redirection();
}

private void main(string orig_input){
  object old=environment(this_body());
  string *parts=explode(orig_input," ");
  object where;
  string cmd = implode(parts[1..], " ");

  old = environment(this_body());
  where=find_body(parts[0]);
  if(!where)
    where=find_object(parts[0]);
  if(!where)
  {
    outf("Could not find object\n");
    return;
  }
  this_body()->move(where);
  this_user()->force_me(cmd);
  if (old)
    this_body()->move(old);
  else
    outf("Could not move back to old room.\n");
}
