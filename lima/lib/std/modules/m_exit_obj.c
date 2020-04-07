/* Do not remove the headers from this file! see /USAGE for more info. */

inherit M_EXIT;

private nosave mixed def_dest = 0;
private nosave mixed def_exit_msg = 0;
private nosave mixed def_enter_msg = 0;
private nosave mixed def_check = 0;
private nosave mixed default_desc = 0;
private nosave mapping prep_dest = ([]);
private nosave mapping prep_exit_msg = ([ ]);
private nosave mapping prep_enter_msg = ([ ]);
private nosave mapping prep_check = ([ ]);
private nosave mapping prep_desc = ([ ]);
private nosave string go_verb = "go";
private nosave string base = 0;
private nosave mapping prep_aliases = ([ ]);

string query_go_method();
void set_default_enter_msg(mixed);
void set_default_exit_msg(mixed);

void alias_prep(string src, string dest) {
    if (!prep_dest[src]) {
        error(sprintf("***Preposition doesn't exist to alias from: %O", src));
    }
    prep_dest[dest] = prep_dest[src];
    prep_exit_msg[dest] = prep_exit_msg[src];
    prep_enter_msg[dest] = prep_enter_msg[src];
    prep_check[dest] = prep_check[src];
    prep_desc[dest]  = prep_desc[src];
}

void setup_default_prep_aliases() {
    foreach (string k, string value in prep_aliases) {
        if (prep_dest[k] && !prep_dest[value]) {
            alias_prep(k, value);
        }
    }
}

mixed prep_default_msg(string prep) {
    if (stringp(prep))
        return "$N $v" + query_go_method() + " " + prep + " " + 
           this_object()->the_short() + ".";
    else
        return "$N $v" + query_go_method() + " " +
            this_object()->the_short() + ".";
}

string query_go_method() {
    return go_verb;
}

void set_go_method(string g) {
    go_verb = g;
}

mixed query_default_destination() {
    mixed dirs = keys(prep_dest) + query_exit_directions(0);
    if (def_dest == 0)
        if (member_array(dirs[0], query_exit_directions(0)) != -1)
            return query_exit_destination(dirs[0], base);
        else
            return eval_dest(prep_dest[dirs[0]], 0, base);
    return eval_dest(def_dest, 0, base);
}

mixed query_default_exit_msg() {
    return evaluate(def_exit_msg);
}

mixed query_default_enter_msg() {
    return evaluate(def_enter_msg);
}

void set_default_destination(mixed arg) {
   def_dest = arg;
    def_check = 1;
    if ( def_exit_msg == 0 ) set_default_exit_msg( (: prep_default_msg :) );
    if ( def_enter_msg == 0 ) set_default_enter_msg( (: prep_default_msg :) );
}

void set_default_exit_msg(mixed arg) {
    def_exit_msg = arg;
}

void set_default_enter_msg(mixed arg) {
    def_enter_msg = arg;
}

void set_default_check(mixed arg) {
    def_check = arg;
}

void set_prep_exit_msg(string prep, mixed msg) {
    prep_exit_msg[prep] = msg;
}

void set_prep_enter_msg(string prep, mixed msg) {
    prep_enter_msg[prep] = msg;
}

void set_prep_check(string prep, mixed check) {
    prep_check[prep] = check;
}

//needed so that "enter door" calls the exit check in direct_go_word_obj()
void set_exit_check(string dir, function f) {
    ::set_exit_check(dir, f);
    set_prep_check(dir, f);
}

void set_prep_description(string prep, mixed desc) {
    prep_desc[prep] = desc;
}

void add_prep(string prep, mixed dest) {
    if (stringp(dest) && dest[0] == '#')
        prep_check[prep] = dest[1..];
    else
        prep_check[prep] = 1;
    prep_dest[prep] = dest;
    prep_desc[prep] = default_desc;
    prep_exit_msg[prep] = (: prep_default_msg :);
    prep_enter_msg[prep] = (: prep_default_msg :);
}

void set_preps(mapping p) {
    mixed key;
   if (mapp(p)) {
        prep_dest += p;
        foreach (key in keys(p)) {
            if (stringp(p[key]) && p[key][0] == '#')
                prep_check[key] = p[key][1..];
            else
                prep_check[key] = 1;
            prep_desc[key] = default_desc;
            prep_exit_msg[key] = (: prep_default_msg :);
            prep_enter_msg[key] = prep_exit_msg[key];
        }
    }
}

mixed query_prep_exit_msg(string prep) {
    prep = PREPOSITION_D->translate_preposition(prep);
    return evaluate(prep_exit_msg[prep], prep);
}

mixed query_prep_enter_msg(string prep) {
    prep = PREPOSITION_D->translate_preposition(prep);
    return evaluate(prep_enter_msg[prep], prep);
}

mixed query_prep_description(string prep) {
    prep = PREPOSITION_D->translate_preposition(prep);
    return evaluate(prep_desc[prep]);
}

mixed query_prep_destination(string prep) {
    prep = PREPOSITION_D->translate_preposition(prep);
    return eval_dest(prep_dest[prep], 0, base);
}

// prep_only only adds the direction to the object, 
// and NOT to the room.. 
// i.e. ("up","blah.c", 1) means that just "up"/"go up" won't work
// you have to say "go up id"
varargs void add_exit(mixed dir, mixed dest, int prep_only) {
    add_prep(dir, dest);
    if (!prep_only) 
        ::add_exit(dir, dest);
}

mixed direct_go_word_obj(string prep) {
    prep = PREPOSITION_D->translate_preposition(prep);
    if (undefinedp(prep_dest[prep]))
        return "You can't " + query_go_method() + " " + prep + " that, only " +
               format_list(keys(prep_dest), "or") + ".";
    //prep and this_body() are needed because of set_exit_check()
    return evaluate(prep_check[prep], prep, this_body());
}

mixed direct_go_wrd_obj(string prep) {
    // The driver calls this instead of direct_go_word_obj() for WRD
    // OBJ rules.. bizarre...
    return direct_go_word_obj(prep);
}

mixed direct_go_obj() {
    mixed dirs = keys(prep_dest);
    if (def_dest == 0) {
        switch (sizeof(dirs)) {
	case 0:
            return environment(this_object())->get_default_exit();
	case 1:
            return direct_go_word_obj(dirs[0]);
        default:
            return "Do you want to " + query_go_method() + " " +
                   format_list(keys(prep_dest), "or") + " that?";
        }
    }
    return evaluate(def_check);
}

mixed direct_enter_obj()
{
   if(query_go_method() != "enter")
      return 0;

   return direct_go_obj();
}

mixed direct_climb_obj()
{
   if(query_go_method() != "climb")
      return 0;

   return direct_go_obj();
}

mixed direct_climb_wrd_obj(string prep)
{
   if(query_go_method() != "climb")
      return 0;

   return direct_go_word_obj(prep);
}

mixed direct_board_obj()
{
   if(query_go_method() != "board")
      return 0;

   return direct_go_obj();
}

mixed direct_dismount_obj()
{
   if(query_go_method() != "mount")
      return 0;
   return 1;
}

void propogate_exits_up() {
    string *dirs = query_exit_directions(1);
    string dir;
    foreach (dir in dirs) {
        if (member_array(dir, 
              environment(this_object())->query_exit_directions()) == -1)
            environment(this_object())->add_exit(dir, this_object());
        else
            error("*Exit already exists in room: " + dir);
    }
    dirs = query_hidden_exits();
    foreach (dir in dirs) {
         environment(this_object())->add_hidden_exit(dir, this_object());
    }
}

void delete_exits_up() {
    string *dirs = query_exit_directions(0);
    string dir;
    foreach (dir in dirs) {
        environment(this_object())->delete_exit(dir);
    }
    dirs = query_hidden_exits();
    foreach (dir in dirs) {
        environment(this_object())->remove_hidden_exit(dir);
        environment(this_object())->delete_exit(dir);
    }
}

varargs void on_clone() {
    propogate_exits_up();
    base = environment(this_object())->query_base();
    setup_default_prep_aliases(); 
}

void remove() {
    delete_exits_up();
}
