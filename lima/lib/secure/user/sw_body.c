/* Do not remove the headers from this file! see /USAGE for more info. */

#include <config.h>
#include <daemons.h>
#include <security.h>

string query_userid();

void save_me();
void remove();
void initialize_user();
void report_login_failures();

void modal_simple(function input_func, mixed prompt, int secure,int lock);
varargs void modal_push(function input_func,
                        mixed prompt,
                        int secure,
                        function return_to_func);
void modal_pop();

void set_privilege(mixed priv);		// from M_ACCESS
mixed unguarded(mixed priv, function fp);

void start_shell();
void run_login_script();

int query_n_gen();

/*
** The file name for the body object
*/
private string          body_fname;
private string          format;

/*
** The body object once it has been instantiated
*/
nosave private object body;

nomask string query_body_fname()
{
   return body_fname;
}

nomask object query_body()
{
   return body;
}

//### temp hack for upgrading link files. see restore_me()
protected nomask void set_body_fname(string new_body_fname)
{
   body_fname = new_body_fname;
}

private void load_mailer()
{
    object mailbox;
    int idx;

    mailbox = MAILBOX_D->get_mailbox(query_userid());

    idx = mailbox->first_unread_message();
    if ( idx == -1 )
    {
        mailbox->set_message_index(mailbox->query_message_count() - 1);
    }
    else
    {
        mailbox->set_message_index(idx);
        write("\n>>You have new mail<<\n");
    }
}

varargs nomask void switch_body(string new_body_fname, int permanent)
{
   object where;
   object old_body;
//tc("switch_bofy(): new_body_fname: "+identify(new_body_fname));

   if(previous_object() != body && this_body() != body)
      error("security violation: bad body switch attempt\n");
//tc("1");

   where = body ? environment(body) : (mixed)VOID_ROOM;
//tc("2");

   if(permanent && new_body_fname)
   {
      body_fname = new_body_fname;
      save_me();
   }
//tc("3");

   if(!new_body_fname)
      new_body_fname = body_fname;
//tc("4");

   old_body = body;
   body = new(new_body_fname, query_userid());
   master()->refresh_parse_info();
//tc("5");

   if(old_body)
   {
      old_body->move(VOID_ROOM);
      if(old_body)
           catch(destruct(old_body));
   }
//tc("6");

   load_mailer();
//tc("7");
   report_login_failures();
//tc("8");

   /* NOTE: we are keeping the same shell for now... */

   body->su_enter_game(where);
//tc("9");
}


/*
** Functions to get the body set up and the user into the game.
*/
private nomask void incarnate(int is_new, string bfn)
{
//tc("incarnate");
   if(bfn)
      body_fname = bfn;

   body = new(body_fname, query_userid());
   master()->refresh_parse_info();

   LAST_LOGIN_D->register_last(query_userid(), query_ip_name(this_object()));
   if(query_n_gen() != -1)
      body->set_gender(query_n_gen());
   save_me();

   start_shell();
   body->enter_game(is_new);
   run_login_script();

   if(is_new)
   {
#ifdef USE_STATS
      this_body()->init_stats();
#endif
      body->save_me();
      /* This seems to me to be a poor place to put this, but fits with 
       * the default login/new user creation sequence.  -- Tigran */
      initialize_user();
   }
}

void sw_body_handle_existing_logon(int);

private nomask void rcv_try_to_boot(object who, string answer)
{
//tc("rcv_try_to_boot("+identify(who)+", "+identify(answer));
   answer = lower_case(answer);
   if( answer == "yes" || answer == "y" )
   {
     /* Check this again, in case the user quits after the question is asked
      * but before this point. If 'who' exists, give them a message and steal
      * the body, otherwise not. */
     if(who)
       {
	 who->receive_private_msg("You are taken over by yourself, or something.\n");
       	 body=who->query_body();
	 who->steal_body();
	 start_shell();
	 body->reconnect(this_object());
	 return;
       }
     sw_body_handle_existing_logon(0);
     return;
   }
   if(answer == "n" || answer == "no")
   {
      if(wizardp(query_userid()))
      {
         sw_body_handle_existing_logon(1);
         return;
      }

      write("Try another time then.\n");
      destruct(this_object());
   }

   write("please type 'y' or 'n'  >");
   modal_simple((: rcv_try_to_boot, who :),0,0,1);
}

protected nomask void sw_body_handle_existing_logon(int enter_now)
{
   remove_call_out(); /* all call outs */
//tc("sw_body_handle_existing_logon("+identify(enter_now)+")");

   if(!enter_now)
   {
      /*
      ** Okay... an existing user is ready for their body.  Look for other
      ** users currently connected with this userid.  Those other usersmay
      ** be interactive or link-dead.  Do the right thing...
      */
      object *users;
      string *ids;
      int idx;
      object the_user;

      /* adjust the privilege of the user ob */
      if(adminp(query_userid()))
         set_privilege(1);
      else
         set_privilege(query_userid());

      /* check for link-deadedness */
      users = children(USER_OB) - ({ this_object() });
      ids = users->query_userid();
      if((idx = member_array(query_userid(), ids)) != -1)
      {
         if(!interactive(the_user = users[idx]))
         {
            if(body = the_user->query_body())
            {
               master()->refresh_parse_info();
               the_user->steal_body();
               start_shell();
               body->reconnect(this_object());
               return;
            }
         }
         else
         {
            write("\nYou are already logged in!\nThrow yourself off?  ");
            modal_simple((: rcv_try_to_boot, the_user :),0,0,1);
            return;
         }
      }
   }

   load_mailer();
   write("\n"+read_file(MOTD_FILE));

   report_login_failures();
   BIRTHDAY_D->report();

   incarnate(0, 0);
}

/* when a user reconnects, this is used to steal the body back */
nomask void steal_body()
{
   /* only USER_OB can steal the body. */
   if(base_name(previous_object()) != USER_OB )
      error("illegal attempt to steal a body\n");

   body = 0;
   remove();
}

#ifdef USE_RACES
void got_entry(function when_done, string line)
{
   mapping races = RACE_D->query_race_data();

   if(line == "list")
   {
      write("Please select a race from the following list:\n");
      printf(format, implode(keys(races), "\n"));
      return;
   }

   if(races[line])
   {
      modal_pop();
      evaluate(when_done, races[line]);
      return;
   }

   if(sscanf(line, "help %s", line) && races[line])
   {
      write(races[line]->short_description());
      return;
   }

   write("No such race.\n");
}
#endif /* USE_RACES */

void create_body()
{
#ifndef USE_RACES
   incarnate(1, DIR_RACES "/human");
#else
   string *races = RACE_D->query_races();
   function when_done = (: incarnate, 1 :);
   int width = 0;

   if(sizeof(races) == 1)
   {
      string default_race = races[0];
      write("You will be a " + default_race + ".\n");
      incarnate(1, DIR_RACES + "/" + default_race);
   }
   else
   {
      foreach(string name in races)
      {
         if(strlen(name) > width)
            width = strlen(name);
      }

      format = "%#-75." + (75 / (width + 3)) + "s\n\n";

      write("\nPlease select a race from the following list:\n");
      printf(format, implode(races, "\n"));

      write("Type 'help race' for a brief description.  Type 'list' to show the choices again.\n");
      modal_push((: got_entry, when_done :), "Race? ");
   }
#endif /* USE_RACES */
}

/*
** A new character has been created and all inputs have been entered.
** Do a bit of additional work and go for a body.
*/
protected nomask void sw_body_handle_new_logon()
{
//tc("sw_body_handle_new_logon() stack: "+get_stack());
   remove_call_out();   /* all call outs */
//tc("1");

#ifdef AUTO_WIZ
   /* auto-wiz everybody as they are created */
   //tc("2");
   write(">>>>> You've been granted automatic guest wizard status. <<<<<\n");
   //tc("3");
   unguarded(1, (: SECURE_D->create_wizard($(query_userid())) :));
   //tc("4");
#endif

   /* auto-admin the first wizard if there are no admins */
   {
      string *members = SECURE_D->query_domain_members("admin");
//tc("5: sizeof("+identify(members)+"): "+sizeof(members));

      if(!sizeof(members))
      {
//tc("6");
//tc("wizardp("+identify(query_userid())+"): "+wizardp(query_userid()));
         if(!wizardp(query_userid())){ unguarded(1,
                     (: SECURE_D->create_wizard($(query_userid())) :));
         }
         write( ">>>>> You have been made admin. Remember to use admtool. <<<<<\n");
         tc( ">>>>> "+identify(query_userid())+" has been made admin. <<<<<\n");
         unguarded(1, (: SECURE_D->add_domain_member("admin",
                         $(query_userid()), 1) :));
      }
//else tc("6b: apparently an admin existed");
   }
//tc("7");
   /* adjust the privilege of the user ob */
   if(adminp(query_userid())){
      //tc("8");
      set_privilege(1);
}
   else
      set_privilege(query_userid());
//tc("9");
   // pass a lfun pointer so that we don't have to worry about validating
   // the call.
   create_body();
//tc("10");
}
