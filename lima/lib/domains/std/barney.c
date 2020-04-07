/* Do not remove the headers from this file! see /USAGE for more info. */


inherit LIVING;
inherit M_ACTIONS;

string *strings = ({
    "The wrath of the almighty Barney!",
    "hap*"
});

string *emotes, *adverbs;

string find_target() {
    object *who = users() & all_inventory(environment());
    if (!sizeof(who)) return this_object()->query_id()[0];
    return who[random(sizeof(who))]->query_userid();
}

void do_my_thing() {
    string which = emotes[random(sizeof(emotes))];
    string *rules = keys(SOUL_D->query_emote(which));
    string rule = rules[random(sizeof(rules))];
    object *who = users();
    string comm, token;
    
    if (!environment()) return;

    simple_action("$N $vdecide to try the '$o $o1' emote.",
		  which, rule);
    
    comm = which;
    foreach (token in explode(rule, " ")) {
	comm += " ";
	switch (token) {
	case "OBJ":
	case "LIV":
	    comm += find_target();
	    break;
	case "STR":
	    if (random(2))
		comm += strings[random(sizeof(strings))];
	    else
		comm += adverbs[random(sizeof(adverbs))];
	    break;
	default:
	    comm += token;
	}
    }
    do_game_command(comm);
    call_out( (: do_my_thing :), 5);
}

void setup() {
    set_name("Barney");
    set_gender(1);
    set_proper_name("Barney the Dinosaur");
    set_in_room_desc("Barney the dinosaur");
    set_long("It's everyone's favorite purple dinosaur!");
    emotes = SOUL_D->list_emotes();
    adverbs = SOUL_D->get_adverbs();
    call_out( (: do_my_thing :), 5);
}
     
