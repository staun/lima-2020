/* Do not remove the headers from this file! see /USAGE for more info. */

// Based on passwd.c and admtool's nuke.
// --OH.

//:PLAYERCOMMAND
//USAGE suicide
//
//This would allow you to completely obliterate your character from the game,
//leaving no trace.
//However, we're sure you wouldn't really want to do that,
//so we've disabled it.
//If you really really do want to be removed completely and irrevocably,
//then mail the request to one of the admins.

inherit M_INPUT;
inherit CMD;

private nomask void nuke_player()
{
    object o;
    mixed err;
    string userid = this_user()->query_userid();
 
    if ( o = find_user(userid) )
    {
    o->receive_private_msg("Farewell.\n");
        o->quit();
    }
 
    MAILBOX_D->get_mailbox(userid)->nuke_mailbox(1);
    MAILBOX_D->unload_mailbox(userid);
 
    /* remove a bunch of files. note: some might not exist. */
    err = rm(LINK_PATH(userid) + __SAVE_EXTENSION__);
    err = rm(USER_PATH(userid) + __SAVE_EXTENSION__);
    err = rm(PSHELL_PATH(userid) + __SAVE_EXTENSION__);
    err = rm(WSHELL_PATH(userid) + __SAVE_EXTENSION__);
 
    LAST_LOGIN_D->remove_user(userid, 1);
 
    err = SECURE_D->delete_wizard(userid);
 
    SECURE_D->set_protection(WIZ_DIR "/" + userid, 1, -1);
}


private nomask void confirm_current_password(string s)
{
    write("\n");
 
    if ( !this_user()->matches_password(s) )
    {
        write("Invalid password.\nAborting.\n");
    }
    else
    {
    nuke_player();
    }
}
 
private void main()
{
    write("Sorry, suicide has been disabled.\n");
    return;
    modal_simple((: confirm_current_password :),
                 "Enter your current password to confirm suicide: ", 1);
}
