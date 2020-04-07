/* Do not remove the headers from this file! see /USAGE for more info. */


inherit VERB_OB;

mixed direct_smell_obj(object ob) {
    return 1;
}

void do_smell_obj(object ob)
{
    if ( !ob->do_smell() )
	printf("It smells pretty much like %s.\n", ob->a_short());
}

void do_smell()
{
  environment(this_body())->do_smell();
}

void create()
{
  add_rules( ({ "", "OBJ" }), ({ "sniff" }) );
}
