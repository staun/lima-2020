/* Do not remove the headers from this file! see /USAGE for more info. */

// Quick hack by Beek


inherit CMD;

//:COMMAND
//USAGE: showtree <function name> <object>
//       showtree <object>
//
//Displays the location(s) of the specified function in the specified
//object, together with any intermediate inheritance.
//
//If used without function nams, it displays the full inheritance tree.
//
//>showtree long here
//
//Tracing definitions of long in /domains/std/Wizroom.c
///domains/std/Wizroom.c
//  /std/indoor_room.c
//    /std/base_room.c
//      [defined]
//      /std/container.c
//        [defined]
//        /std/object.c
//          /std/base_obj.c
//            /std/object/description.c
//              [defined]

varargs string print_tree(string file, string func, int indent) {
    object ob;
    string result;
    int found;
    
    if( !file )
    {
        return "You need to specify an object.\n";
    }
    
ob = find_object( absolute_path(file, get_user_variable("pwd")));
    
    if (!ob) {
    return repeat_string("  ", indent) + "Couldn't find object: " + file + "\n";
    }
    result = repeat_string("  ", indent) + file + ":\n";
    if (func && (function_exists(func, ob) + ".c") == file) {
    result += repeat_string("  ", indent) + "  [defined]\n";
	found = 1;
    } else if (!func)
	found = 1;

    indent++;
    foreach (file in inherit_list(ob)) {
	string str = print_tree(file, func, indent);
	if (str != "") {
	    result += str;
	    found = 1;
	}
    }	
    return (found ? result : "");
}

// [str] cfile
private void main(mixed *arg)
{
  string file, func;
  object target;
  if (arg[1])
  {
    target=arg[1];
    file = base_name(target);
    func = arg[0];
    out("Tracing definitions of '" + func + "' in " + file + "\n");
  } else {
    target=arg[0];
    file=base_name(target);
    func = 0;
  }

  out(print_tree(file, func));
}