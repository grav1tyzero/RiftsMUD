//      /d/st_adjndard/setter.c
//      Starting room for new characters choosing races
//      from the Nightmare Mudlib
//      created by Shadowwolf@Nightmare july 1992
//      modified by Descartes of Borg for the race daemon 10 june 1993

#include <std.h>
#include <security.h>
#include <rooms.h>
#include <daemons.h>
#include <ansi.h>

#define CLS ESC + "[H" + ESC + "[2J"

inherit ROOM;



void do_rolls();
void assign_point(string str, int *points, int pts_left);

void set_ansi();



void create()
{
  ::create();
  set_property("light", 2);
  set_property("indoors", 1);
  set_property("no attack", 1);
  set_property("no scry", 1);
  set_property("no teleport", 1);
  set_property("no summon", 1);
  set_property("no cast", 1);
  set("long",
      "Here would be some elaborate instructions about which help files to read like 'help newbie'. "
      "There should be some sort of action to list races and pick one. Once a race is selected then"
      "the mud is being build now so just type ready");
  skip_obvious();
  set_items(
      (["room":"The nothingness from which you will be born", "list":"A list of races"]));
}

void init()
{
  ::init();
  this_player()->setenv("SCORE", "off");
  this_player()->set("in creation", 1);
  this_player()->set_catch("off");
  //add_action("read", "read");
  add_action("set_ansi", "ready");
}

int pick(string str)
{
}

void set_ansi()
{
  this_player()->setenv("TERM", "ansi");
  this_player()->reset_terminal();
  this_player()->reset_max_exp();
  write("\nOne more thing...\n%^RED%^Is this line red (y or n)?%^RESET%^");
  input_to("set_ansi_two");
}

void set_ansi_two(string str)
{

  str = arrange_string(lower_case(str), 1);
  if (str == "n")
  {
    write("Ok.  Terminal set to \"unknown.\"  See 'help setenv' for more info.");
    this_player()->setenv("TERM", "unknown");
    this_player()->reset_terminal();
    this_player()->move(ROOM_NEWBIE);
    if (wizardp(this_player()) || this_player()->query_property("reincarnate"))
    {
      write("You are done.");
      return;
    }

    return;
  }
  else if (str == "y")
  {
    write("Terminal set to ansi.");
    this_player()->move(ROOM_NEWBIE);
    if (wizardp(this_player()) || this_player()->query_property("reincarnate"))
    {
      write("You are done.");
      return;
    }

    return;
  }
  else
  {
    write("Answer y or n");
    input_to("set_ansi_two");
    return;
  }
}

int read(string str)
{
  if (!str)
  {
    notify_fail("What do you want to read? A list?\n");
    return 0;
  }
  if (str != "list")
  {
    notify_fail("That is not here to be read.\n");
    return 0;
  }
  write("These are the following races available in our reality:");
  write("-----------------------------------------------------------");
  message("Ninfo", format_page((string *)RACE_D->query_races(), 4),
          this_player());
  write("----------------------------------------------------------");
  write("pick <race> will forever make you one of these races");
  write("Type help <race> for more information.");
  return 1;
}
