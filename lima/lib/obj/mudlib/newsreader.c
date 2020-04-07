/* Do not remove the headers from this file! see /USAGE for more info. */

/*
** newsreader.c
**
** Basic newsreader to interface with the news server.
**
** 941224, Deathblade: created.
** 950702, Rust: added completion and menus for the g command.
** 950811, Deathblade: convert to use class news_msg.
** Ohara: Lots of misc tweaks, chromefixes, bugfixes, etc. RTFDL ;)
** 971103, Tigran:  Lots of changes.  Proper group subscription, and threading
**                  Functionality added.  
** 980313: Tigran:  Added thread toggling.  The newsreader now uses queues
**                  Removed articles are not shown on displays.  And the
**                  it is now possible to mark all articles in the newsgroup
**                  unread
** 980426  Tigran   Improved queueing, added some colouring, fixed misc bugs
*/

#include <classes.h>
#include <edit.h>

inherit M_INPUT;
inherit M_COMPLETE;
inherit M_GRAMMAR;
inherit M_SET;

inherit CLASS_NEWSMSG;

private nosave string   current_group;
private nosave int      current_id;
private nosave int      current_thread;
private nosave int      queue_position;
private nosave int *message_queue=({});
private nosave int *all_messages=({});
private nosave mapping active_messages=([]);
private nosave mapping unread_cache=([]);

#define TOP_PROMPT      ("(...) " + mud_name() + " News [q?lg] > ")

# define SAVE_OB this_user()

#define STANDARD_WRAP_WIDTH     68
#define FOLLOWUP_WRAP_WIDTH     76

private nomask void display_groups_with_new(string arg);
varargs nomask void begin_reading(string arg);
nomask void receive_top_cmd(string cmd);
nomask void receive_grp_cmd(string cmd);
nomask void receive_msg_cmd(string cmd);
private nomask void next_group();

private nomask void quit_news()
{
    /* save the player's news reading state */
    SAVE_OB->save_me();
    modal_pop();
    destruct();
}


/* This caching somehow would be a good thing */
varargs private string get_unread_ids(string group,int update)
{
  int ids;
  /* Construct the smallest set contaning all possible news articles */
  if(member_array(group,keys(unread_cache))==-1||update)
    {
      string unread=set_difference(set_add_range("",1,NEWS_D->get_group_last_id(group) ),
				   SAVE_OB->get_news_id_read(group) );
      ids=active_messages[group];
      /* Add unread messages into the unread set */
      map(ids,(:member_set($1,$(unread))?$(unread)=set_remove($(unread),$1):0 :) );
      unread_cache[group]=unread;
      return unread;
    }
  return unread_cache[group];
}

void add_unread_id(string group,int id)
{
  string unread_set=get_unread_ids(group);
  unread_cache[group]=set_add(unread_set,id);
}

void remove_unread_id(string group,int id)
{
  string unread_set=get_unread_ids(group);
  unread_cache[group]=set_remove(unread_set,id);
}

private int get_lowest_unread_id(string group)
{
  int i;
  i=set_min(get_unread_ids(group));
  if(!i)
    return 0;
  /* If the message doesn't exist it has either been archived or imporperly 
   * removed.  Mark it read and find something different */
  if(member_array(i,NEWS_D->get_messages(group))==-1)
    {
      SAVE_OB->add_news_id_read(group,i);
      i=get_lowest_unread_id(group);
    }
  return i;
}

private int get_lowest_unread_thread_id(string group)
{
  return NEWS_D->get_thread_id(group,
                               get_lowest_unread_id(group) );
}

private void receive_search(int flag, string str) {
    mixed *results;
    if(!sizeof(str))
      {
        write( "You must specify a search.\n");
        return;
      }
    if ( flag )
        results = NEWS_D->search_for_author(str);
    else
        results = NEWS_D->search_for(str);

    if (!sizeof(results)) {
        write("No matches.\n");
        return;
    }
    more(({ sizeof(results) + " matches:", "" }) + map(results, (: $1[0] + ":" + $1[1] :)));
}

private int count_active_messages(string group)
{
  return sizeof(active_messages[group]);
}

private int count_unread_messages(string group)
{
  string unread=get_unread_ids(group);
  int *ids=active_messages[group];
  int i;
  foreach(int id in ids)
    {
      if(member_set(id,unread))
	i++;
    }
  return i;
}

private void create_queues()
{
  all_messages=active_messages[current_group];
  queue_position=-1;
  current_id=-1;
  message_queue=filter(all_messages, (: member_set :),get_unread_ids(current_group,1));
}

varargs private void update_queues()
{
  /* First, update the entire active news.  */
  active_messages=([]);
  if(SAVE_OB->query_threading())
    active_messages=NEWS_D->get_all_thread_data();
  else
    {
      foreach(string group in NEWS_D->get_groups())
	active_messages[group]=NEWS_D->get_messages(group,1);
    }
  all_messages=active_messages[current_group];
  message_queue=clean_array(message_queue+filter(all_messages,(:member_set:),get_unread_ids(current_group,1)));
  if(queue_position!=-1)
    queue_position=member_array(current_id,message_queue);
}

/* This is a fairly decent place to set up the queue */
private nomask string format_group_line(string group)
{
    int last_id;
    int all;
    current_group=group;
    all=sizeof(active_messages[group]);
    last_id = NEWS_D->get_group_last_id(group);
    return sprintf("%s  %-40s (%d %s, %d new)%%^RESET%%^",
		   SAVE_OB->check_subscribed(group)?"%^NEWS_GROUP_SUBSCRIBED%^":"%^NEWS_GROUP_UNSUBSCRIBED%^",
                   group,
                   all,
                   all == 1 ? "message" : "messages",
		   count_unread_messages(group));
}

nomask void rcv_group_answer(string *groups,string input)
{
  switch(input)
    {
    case "y":
      SAVE_OB->subscribe_group(groups[0]);
      break;
    case "n":
      SAVE_OB->unsubscribe_group(groups[0]);
      break;
    case "q":
      return;
    default:
      modal_simple( (:rcv_group_answer,groups:),
                    sprintf("Subscribe to %s [ynq]? ",
                            groups[0]) );
      return;
    }
  groups-=({groups[0]});
  if(sizeof(groups))
    modal_simple( (:rcv_group_answer,groups:),
                  sprintf("Subscribe to %s [ynq]? ",
                          groups[0]) );
  else 
    {
      display_groups_with_new("");
      next_group();
    }
  return;
}

varargs private nomask void add_new_groups(string arg)
{
  string * groups;
  if(groups=SAVE_OB->registered_groups())
    groups=NEWS_D->get_groups()-groups;
  else 
    groups=NEWS_D->get_groups();
  if( !sizeof(groups) )
    {
      display_groups_with_new("");
      next_group();
      return;
    }
  modal_simple( (:rcv_group_answer,groups :), sprintf("Subscribe to %s? [ynq] ",
                                                     groups[0]) );
  return;
}

private nomask string grp_cmd_prompt()
{
  int unread_no=count_unread_messages(current_group);
  return sprintf("(%s:%d) %d %s unread of %d [q?lLmgsSputncU] > ",
		 current_group,
		 sizeof(message_queue)>1?current_id+1:0,
		 unread_no,
		 unread_no == 1 ? "msg" : "msgs",
		 count_active_messages(current_group)
		 );
}

private nomask string msg_cmd_prompt()
{
  return sprintf("(%s:%d:#%d) %d unread [q?lLmghsSprRfFncUMDut#g] > ",
		 current_group,
		 queue_position+1,
		 current_id,
		 count_unread_messages(current_group));
}

private nomask void switch_to_top()
{
    modal_push((: receive_top_cmd :), TOP_PROMPT);
}

private nomask void switch_to_group()
{
    modal_func((: receive_grp_cmd :), (: grp_cmd_prompt :));
}

private nomask void switch_to_message()
{
    modal_func((: receive_msg_cmd :), (: msg_cmd_prompt :));
}


private nosave mixed group_selection_menu_items;
private nomask void menu_select_newsgroup(string num)
{
    int index;
    if ( num=="q" )
    {
        write( "Selection aborted.\n");
        if( !current_group) destruct( this_object());
        switch_to_message();
        return;
    }

    if ( !sscanf(num,"%d",index) || index < 1 || index > 
      sizeof(group_selection_menu_items) )
    {
        write("Invalid selection.\n");
        return;
    }
    current_group = group_selection_menu_items[index-1];
    group_selection_menu_items = 0;
    create_queues();
    switch_to_group();
}


private nomask void menu_select_change_header( string header )
{
    NEWS_D->change_header( current_group, current_id, header );
}


private nomask void menu_select_movegroup(string num)
{
    string to_group;
    int index;
    if( num == "q" )
    {
        write( "Aborting Move.\n" );
        switch_to_message();
        return;
    }
    if( !sscanf( num, "%d", index )|| index < 1 || index >
      sizeof( group_selection_menu_items))
    {
        write( "Invalid selection.\n");
        return;
    }
    to_group = group_selection_menu_items[index - 1];
    group_selection_menu_items = 0;
    NEWS_D->move_post( current_group, current_id, to_group);
    switch_to_message();
}

private nomask void read_group(string group)
{
    string* matches;
    int i;
    matches = complete(group, NEWS_D->get_groups());
    switch ( sizeof(matches) )
    {
    case 0:
        write("No such group.\n");
        if(!current_group) destruct( this_object());
        return;
    case 1:
        current_group = matches[0];
        break;
    default:
        write("\nSelect group by number:\n"
          "----------------------------\n");
        for(i=1; i<=sizeof(matches);i++)
            printf("%-4d%s\n",i,matches[i-1]);
        group_selection_menu_items = matches;
        modal_func((: menu_select_newsgroup:), "[#q] ");
        return;
    }
    create_queues();
    switch_to_group();
}

private nomask void receive_group(mixed group)
{
  if ( group == -1 )
    destruct(this_object());
  
  read_group(trim_spaces(group));
}

private nomask int test_for_new(string group)
{
    if( count_unread_messages( group )>0) return 1;
    return 0;
}

private nomask void display_groups_with_new(string arg)
{
  string * groups=SAVE_OB->subscribed_groups();
    /*
    ** Argument options:
    **   -c   Check news. Display new news and then exit.
    */
  if(sizeof(groups))
    groups = filter_array(SAVE_OB->subscribed_groups(), (: test_for_new :));
  else
    groups=({});
  if(!sizeof(groups))
    {
        write("No new news.\n");

        if ( arg == "-c" )
        {
	  destruct( this_object());
	  return;
        }
        modal_push((: receive_top_cmd :), TOP_PROMPT);
    }
    else
    {
        string * list;
        list = ({ "Groups with new messages:", "" }) +
        map_array(groups, (: format_group_line :));

        /* if -c was given, then quit after displaying the new stuff */
        if ( arg == "-c" )
            more(list, 0, (: quit_news :));
        else
            more(list);
    }
}

private nomask void next_group()
{
    string * groups;
    int max_group;
    int i;
    int next_group;
    
    groups = filter_array(SAVE_OB->subscribed_groups(), (: test_for_new :));
    max_group = sizeof( groups ) - 1;
    if( max_group == -1 )
    {
        write("No more groups with new news.\n");
	/* Hrmm, this looks like a good time to catch up all newsgroups 
	 * to keep that set string from being too nasty */
	map(SAVE_OB->subscribed_groups(),(: SAVE_OB->catch_up_newsgroup($1) :) );
        switch_to_top();
        current_group=0;
    }
    else
    {
      for( i = 0; i <= max_group; i++ )
        {
          if( groups[i] == current_group )
            {
              if( i == max_group )
                next_group = 0;
              else
                next_group = i + 1;
              break;
            }
        }
      current_group = groups[next_group];
      create_queues();
      write("Moving to: " + current_group + "\n");
      switch_to_group();
    }
}

private nomask void previous_message()
{
  queue_position--;
  if(queue_position<0)
    {
      queue_position=0;
      return;
    }
}

private nomask void next_message()
{
  int *old_queue;
  queue_position++;
  if(queue_position>=sizeof(message_queue))
    {
      int i;
      string unread;
      /* First check to see that there is no more news in this group */
      i=sizeof(message_queue);
      old_queue=message_queue;
      update_queues();
      /* This should not have to be done here...
       * make an update type command -- Tigran */
      /* The fact that it is here is more than likely by popular 
       * demand :( -- Tigran */
      if(sizeof(old_queue) != sizeof(message_queue) && !sizeof(old_queue - message_queue))
	{
	  write("Bringing in new articles.\n");
	  next_message();
	  return;
	}
      i=0;
      unread=get_unread_ids(current_group);
      if(!sizeof(message_queue-filter(message_queue, (: member_set($1,$(unread)) :) ) ) )
      /* prolly change this to a sizeof(filter()) check */
      foreach(int id in message_queue)
	{
	  if(member_set(id,unread))
	    i=1;
	}
      if(!i)
	SAVE_OB->catch_up_newsgroup(current_group);
      return;
    }
  return;
}
  
varargs private nomask
string format_message_line(int short_fmt, int id,int all)
{
    class news_msg msg;
    int i=0;
    if(current_id>-1)
      if(current_id==id)
	i=1;
    msg = NEWS_D->get_message(current_group, id);
    return sprintf(short_fmt ? "%s%s%d. %s  [%s on %s]%%^RESET%%^" :
		   "%s%s%4d. %-35s [%-10s on %s]%%^RESET%%^",
		   member_set(id,SAVE_OB->get_news_id_read(current_group))?
		     "%^NEWS_MSG_READ%^":"%^NEWS_MSG_UNREAD%^",
		   i?">":" ",
		   all?id:member_array(id,message_queue)+1,
		   msg->subject[0..34],
		   msg->poster,
		   intp(msg->time) ? ctime(msg->time)[4..9] : msg->time);
}

private nomask void display_messages(int display_all)
{
    int *ids;
    string *lines;
    if ( !display_all )
      {
	ids=message_queue;
	lines=({sprintf("Queued messages on %s are:", current_group)});
      }
    else 
      {
	ids=all_messages;
	lines = ({sprintf("Messages on %s are:", current_group)});
      }
    lines+=map_array(ids,
		     (: format_message_line(0, $1, ($(display_all)?1:0) ) :) );
    more(lines);
}

nomask void wrap_post(string * text)
{
    int i = sizeof(text);
    while ( i-- )
    {
        /* only wrap really long lines */
        if ( sizeof(text[i]) > 78 )
        {
            /* wrap to different widths, based on the text */
            int width = text[i][0]=='>' ? FOLLOWUP_WRAP_WIDTH : STANDARD_WRAP_WIDTH;
            if ( sizeof(text[i]) > width )
                text[i] = wrap(text[i], width);
        }
    }
}

private nomask void receive_post_text(string subject, string * text)
{
    int id;
    if ( !text )
    {
        write("Post aborted.\n");
        return;
    }
    wrap_post(text);
    id = NEWS_D->post(current_group, subject, implode(text, "\n") + "\n");
    write("Posted:  " + format_message_line(1, id) + "\n");
    update_queues();
    switch_to_message();
}

private nomask void receive_post_subject(mixed subject)
{
    if ( subject == -1 )
        destruct(this_object());

    if ( subject == "" )
    {
        write("Post aborted.\n");
        return;
    }

    new(EDIT_OB, EDIT_TEXT, 0, (: receive_post_text, subject :));
}

private nomask void post_message()
{
    if( NEWS_D->is_write_restricted( current_group ))
    {
        write( "You may not post to " + current_group + ".\n");
        return;
    }
    modal_simple((: receive_post_subject :), "Subject: ");
}

nomask void receive_reply_text( string * text)
{
  class news_msg msg = ((class news_msg)NEWS_D->get_message(current_group,
							    current_id));
  if ( !text )
    {
      write("Post aborted.\n");
      return;
    }
    wrap_post(text);

    this_user()->query_mailer()->send_news_reply(
      "Re: " + msg->subject,
      text,
      lower_case( msg->poster )
    );
}

private nomask void reply_to_message()
{
    class news_msg msg = ((class news_msg)NEWS_D->get_message(current_group,
							      current_id));
    if ( !msg )
    {
        write("You may not reply to that message -- it was removed.\n");
        return;
    }
    new(EDIT_OB, EDIT_TEXT, 0, (: receive_reply_text :));
}

private nomask void reply_with_message()
{
    class news_msg msg = ((class news_msg)NEWS_D->get_message(current_group,
							      current_id));
    string * lines;
    if ( !msg )
    {
        write("You may not reply to that message -- it was removed.\n");
        return;
    }
    lines = ({ sprintf("On %s %s wrote post %s in %s:",
		       intp(msg->time) ? ctime(msg->time) : msg->time,
		       msg->poster,
		       msg->subject,
		       current_group) });
    lines += map_array(explode(msg->body, "\n"), (: "> " + $1 :) );
    new(EDIT_OB, EDIT_TEXT, lines, (: receive_reply_text :));
}

nomask void receive_followup_text(string * text)
{
    int id;
    if ( !text )
    {
        write("Post aborted.\n");
        return;
    }
    wrap_post(text);

    /*
    ** -1 to get the "current" message.  The player records the _next_
    ** message to read.
    */
    id = NEWS_D->followup(current_group,
			  current_id,
			  implode(text, "\n") + "\n");
    write("Posted:  " + format_message_line(1, id) + "\n");
}

private nomask void followup_to_message()
{
    class news_msg msg = ((class news_msg)NEWS_D->get_message(current_group,
							      current_id));
    if ( !msg->body )
    {
        write("You may not followup to that message -- it was removed.\n");
        return;
    }
    if( NEWS_D->is_write_restricted( current_group ))
    {
        write( "You may not post to " + current_group + ".\n");
        return;
    }
    new(EDIT_OB, EDIT_TEXT, 0, (: receive_followup_text :));
}

private nomask void followup_with_message()
{
    class news_msg msg = ((class news_msg)NEWS_D->get_message(current_group,
							      current_id));
    string * lines;
    if ( !msg->body )
    {
        write("You may not followup to that message -- it was removed.\n");
        return;
    }
    if( NEWS_D->is_write_restricted( current_group ))
    {
        write( "You may not post to " + current_group + ".\n");
        return;
    }
    lines = ({ sprintf("On %s %s wrote post %s:",
        intp(msg->time) ? ctime(msg->time) : msg->time,
        msg->poster, msg->subject) });

    lines += map_array(explode(msg->body, "\n"), (: "> " + $1 :) );

    new(EDIT_OB, EDIT_TEXT, lines, (: receive_followup_text :));
}

private nomask void read_message(string group,int id)
{
    class news_msg msg;
    string post;
    msg = NEWS_D->get_message(current_group,id);
    /*
    ** Set the next-to-read id.  Always move to the message prompt.
    */
    SAVE_OB->add_news_id_read(current_group,id);
    remove_unread_id(current_group,id);
    if ( msg )
    {
        post = sprintf("Time:    %%^NEWS_TIME%%^%-40s%%^RESET%%^Post-id: %d (%d Last)\n"
                       "Poster:  %%^NEWS_POSTER%%^%s%%^RESET%%^\n"
                       "Subject: %%^NEWS_SUBJECT%%^%s%%^RESET%%^\n"
                       "\n"
                       "%s",
                       intp(msg->time) ? ctime(msg->time) : msg->time,
                       id,
                       NEWS_D->get_group_last_id(current_group),
                       msg->poster,
                       msg->subject,
                       msg->body ? msg->body : "*** REMOVED ***");
    }
    else
    {
      write("No such message\n");
      return;
    }
    queue_position=member_array(id,message_queue);
    current_id=id;
    switch_to_message();
    more(post);
}

private nomask void toggle_subscription(string group)
{
  if(member_array(group,NEWS_D->get_groups())==-1)
    {
      write("No such newsgroup\n");
      return 0;
    }
  if(SAVE_OB->check_subscribed(group))
    {
      SAVE_OB->unsubscribe_group(group);
      printf("Group %s unsubscribed.\n",group);
      return;
    }
  SAVE_OB->subscribe_group(group);
  printf("Group %s subscribed\n",group);
}

private nomask void global_commands(string cmd)
{
  cmd=trim_spaces(cmd);
  if( cmd == "q" )
    {
        quit_news();
    }
    else if ( cmd == "s" )
    {
        modal_simple((: receive_search, 0 :), "Search for message: ");
    }
    else if ( cmd == "S" )
    {
        modal_simple((: receive_search, 1 :), "Search for author: ");
    }
    else if ( cmd == "t" )
      {
	if(SAVE_OB->query_threading())
	  {
	    write("No longer threading news.\n");
	    SAVE_OB->set_threading(0);
	  }
	else
	  {
	    write("Now threading news.\n");
	    SAVE_OB->set_threading(1);
	  }
      }
    else if ( cmd == "")
    {
        if ( sizeof(filter_array(SAVE_OB->subscribed_groups(),
              (: test_for_new :))) )
        {
            modal_pop();
            begin_reading();
            return;
        }
        else
            quit_news();
    }
    else if ( cmd[0] == 'g' )
    {
        if ( strlen(cmd) > 1 )
        {
            read_group(trim_spaces(cmd[1..]));
        }
        else
        {
            modal_func((: receive_group :),
              "Which group (Or enter for a menu)? ");
        }
    }
    else
    {
        write("Unknown command. Type ? for help.\n");
    }
}

private nomask void group_commands(string cmd)
{
    int id;
    if ( cmd == "l" || cmd == "L" )
    {
        display_messages(cmd == "L");
    }
    else if ( cmd == "m" )
    {
        switch_to_top();
    }
    else if ( cmd == "p" )
    {
        post_message();
    }
    else if ( cmd == "n" )
    {
        next_group();
    }
    else if ( cmd == "c" )
    {
        SAVE_OB->catch_up_newsgroup(current_group);
        write("All posts marked as read.\n");
        next_group();
    }
    else if ( cmd == "U")
      {
	SAVE_OB->mark_newsgroup_unread(current_group);
	write("All posts marked unread.\n");
	create_queues();
      }
    else if ( cmd == "h")
    {
        modal_simple((: menu_select_change_header :), "Change subject to: ");
    }
    else if (cmd=="u")
      {
	toggle_subscription(current_group);
      }
    
    else if (cmd[0]=='#')
      {
	int num;
	num=to_int(cmd[1..]);
	if(!NEWS_D->get_message(current_group,num))
	  {
	    write("No such message.\n");
	    return;
	  }
	read_message(current_group,num);
	return;
      }
    else if ( cmd == "<")
      {
	int old=queue_position;
	previous_message();
	if(old==queue_position)
	  {
	    write("No previous messages in queue.\n");
	    return;
	  }
	read_message(current_group,message_queue[queue_position]);
	return;
      }
    else if ( cmd == "" || cmd == ">" )
      {
	int old=queue_position;
	string group=current_group;
	if(!sizeof(message_queue))
	  {
	    write("No queued messages.\n");
	    next_group();
	    return;
	  }
	next_message();
	if(old==queue_position)
	  next_group();
	if(current_group!=group)
	  return;
	read_message(current_group,message_queue[queue_position]);
      }
    else if( id = to_int(trim_spaces(cmd)))
      {
        id-=1;
        if(!sizeof(message_queue))
	  {
	    write("No queued messages.\n");
	    return;
	  }
        if ( id < 0 )
            id = 0;
        else if ( id >= sizeof(message_queue))
	  {
	    write("No such message.\n");
	    return;
	  }
	read_message(current_group,message_queue[id]);
    }
    else
    {
        global_commands(cmd);
    }
}

private nomask void receive_remove_verify(string str)
{
  string group=current_group;
    if ( str[0] != 'y' && str[0] != 'Y' )
    {
        write("Removal aborted.\n");
        return;
    }
    NEWS_D->remove_post(current_group, current_id);
    update_queues();
    switch_to_group();
}

private nomask void get_move_group(string str)
{
    string * matches;
    int i;
    matches = complete (str, NEWS_D->get_groups());
    switch ( sizeof(matches) )
    {
    case 0:
        write("No such group.\n");
        return;
    case 1:
        NEWS_D->move_post(current_group,
			  current_id,
			  matches[0]);
        break;
    default:
        write("\n"
          "Select group by number:\n"
          "--------------------------------\n");
        for ( i = 1; i <= sizeof(matches); i++)
            printf("%-4d%s\n",i,matches[i-1]);
        group_selection_menu_items = matches;
        modal_func((: menu_select_movegroup:), "[#q] ");
        return;
    }
}

private nomask void receive_move_verify( string str )
{
    if( str[0] != 'y' && str[0] != 'Y' )
    {
        write( "Move aborted.\n");
        return;
    }
    modal_simple( (: get_move_group :), "Move post to: ");
}

private nomask void remove_message()
{
    class news_msg msg;
    msg=((class news_msg)NEWS_D->get_message(current_group,current_id));
    if ( !msg )
    {
        write("This post has already been removed.\n");
        return;
    }
    if ( !adminp(this_user()) &&
      msg->userid != this_user()->query_userid() )
    {
        write("You are not allowed to remove that post.\n");
        return;
    }
    printf("Deleting: %s\nAre you sure? [yn] > ",
      format_message_line(1, current_id,1));
    modal_simple((: receive_remove_verify :));
}


private nomask void receive_top_cmd(mixed cmd)
{
    if ( cmd == -1 )
        destruct(this_object());

    cmd = trim_spaces(cmd);
    if ( cmd == "?" )
    {
        write("\n" +
          "This is the prompt for commands that apply before "
          "you have picked a newsgroup.  The commands that "
          "apply are:" +
          "\n"
          "  q   quit reading news\n"
          "  ?   this help\n"
          "  l   list newsgroups\n"
          "  s   search for post\n"
          "  S   search for author\n"
          "  g   go to a newsgroup\n"
          "\n"
          "Just type a command at the prompt and hit return.\n"
        );
    }
    else if ( cmd == "l" )
    {
        string * list;
	
        list = map_array(NEWS_D->get_groups(), (: format_group_line :));
        list = ({ "", "Available groups are:" }) + list;
        more(list);
    }
    else
    {
        global_commands(cmd);
    }
}

private nomask void receive_grp_cmd(mixed cmd)
{
    if ( cmd == -1 )
        destruct(this_object());
    cmd = trim_spaces(cmd);
    if ( cmd == "?" )
      {
        write("\n"
	      "This is the prompt for commands that apply to the "
	      "current newsgroup you are reading (" + current_group +
	      ").  The commands that apply are:"
	      "\n"
	      "  q   quit reading news\n"
	      "  ?   this help\n"
	      "  l   list queued messages\n"
	      "  L   list all messages\n"
	      "  m   main news menu\n"
	      "  g   go to a newsgroup\n"
	      "  s   search for a message\n"
	      "  S   search for an author\n"
	      "  p   post message\n"
	      "  u   toggle subscription to this group\n"
	      "  t   toggle threading\n"
	      "  n   next newsgroup with new news\n"
	      "  c   mark all messages as read (catch up)\n"
	      "  U   mark all messages as unread\n"
	      "\n"
	      "  #num      goto message with id 'num'\n"
	      "  g group   goes to newsgroup \"group\"\n"
	      "  <return>  read messages or go to next newsgroup with new news\n"
	      "\n"
	      "Just type a command at the prompt and hit return.\n"
	      );
      }
    else
    {
        group_commands(cmd);
    }
}

private nomask void receive_msg_cmd(mixed cmd)
{
    if (cmd == -1) {
        destruct(this_object());
        return;
    }
    cmd = trim_spaces(cmd);
    switch(cmd)
      {
      case "?":
        write("\n"
              "This is the prompt for commands that apply to the "
              "current newsgroup you are reading (" + current_group +
              ").  The commands that apply are:"
              "\n"
              "  q   quit reading news\n"
              "  ?   this help\n"
              "  l   list queued messages\n"
              "  L   list all messages\n"
              "  m   main news menu\n"
              "  g   go to a newsgroup\n"
              "  h   change subject of a post\n"
              "  s   search for a message\n"
              "  S   search for an author\n"
              "  p   post message\n"
              "  r   reply to this message\n"
              "  R   reply to this message, quoting it\n"
              "  f   post a followup to this message\n"
              "  F   post a followup to this message, quoting it\n"
              "  n   next newsgroup with new news\n"
              "  c   mark all messages as read (catch up)\n"
	      "  U   mark all messages as unread\n"
              "  M   move post to a different group [owner/admin]\n"
              "  D   remove this message\n"
	      "  u   toggle subscription to this group\n"
	      "  t   toggle threading\n"
	      "  #num      Goto message with id 'num'\n"
              "  g group   goes to newsgroup \"group\"\n"
              "  <return>  read messages or go to next newsgroup with new news\n"
              "\n"
              "Just type a command at the prompt and hit return.\n");
        break;
      case "D":
        remove_message();
        break;
      case "r":
        reply_to_message();
        break;
      case "R":
        reply_with_message();
        break;
      case "f":
        followup_to_message();
        break;
      case "F":
        followup_with_message();
        break;
      case "s":
        modal_simple((: receive_search :), "Search for message: ");
        break;
      case "M":
        modal_simple((: receive_move_verify :), "Move post? [y/N] ");
        break;
      default:
        group_commands(cmd);
      }
}

varargs nomask void begin_reading(string arg)
{
  /* Pull in the active newsgroups */
  if(SAVE_OB->query_threading())
    active_messages=NEWS_D->get_all_thread_data();
  else
    {
      foreach(string group in NEWS_D->get_groups())
	active_messages[group]=sort_array(NEWS_D->get_messages(group,1),1);
    }
  if ( !sizeof(NEWS_D->get_groups()) )
    {
      printf( "%s has no newsgroups right now.\n", mud_name());
      destruct();
      return;
    }
  /* Initialize the modal */
  switch_to_top();
  SAVE_OB->validate_groups();
  if ( sizeof(arg) && arg[0] != '-' )
    {
      read_group(arg);
      return;
    }
  add_new_groups();
}
