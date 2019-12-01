/*
It's a little strange because anywhere in the world you would have
a different phase of the moon.

This previously supported multiple moons but I am going to be doing
an Earth based MUD so there's no point to it

--Parnell 2019

*/
#include <daemons.h>
#include <clock.h>

//always ensure full moon is halfway between
#define PHASES ({ "new", "waxing crescent","first quarter","axing gibbous", "full", "waning gibbous", "third quarter", "waning crescent" })
#define MAX_MOON_LIGHT 2.0
private static int moon_light;

varargs string query_phase_name(int x);
varargs int query_phase(string moon);

void create() {
    seteuid(getuid());
}

// let's you pass moon for backward compat for a moment
varargs int query_phase(string moon) {
  return CURRENT_WEEK(time()) % sizeof(PHASES);
}

varargs string query_phase_name(int phase) {
    if(!phase || phase > sizeof(PHASES))
      return PHASES[query_phase()];
    else return PHASES[phase];
}

int query_moon_light() {
  // basically you can get up to MAX_MOON_LIGHT for full moon
  int fullness_level, full_moon_phase;
  float percent_full;
  full_moon_phase = sizeof(PHASES) / 2;
  fullness_level = (query_phase() <= full_moon_phase ? query_phase() : (sizeof(PHASES) - query_phase()));
  percent_full = to_float(fullness_level) / (sizeof(PHASES) / 2);
  return to_int(round(percent_full * MAX_MOON_LIGHT));
}

void la_sky(string str) {
    string tod;
    int ansi;
    if(environment(this_player())->query_property("indoors")) {
      write(DEFAULT_MSG);
      return;
    }
    ansi = (int)this_player()->query_ansi();
    tod = (string)EVENTS_D->query_time_of_day();

    switch(str) {
        case "sun":
          switch(tod) {
            case "dawn":
              write("The sun is hanging low in the dawning eastern sky.\n");
              break;
            case "day":
              write("The sun is shining brightly in the daytime sky.\n");
              break;
            case "twilight":
                write("The sun is falling into the twilight sky.\n");
              break;
            case "night":
              write(DEFAULT_MSG);
              break;
          }
          break;
        case "moon":
          if(tod != "night")
            write(DEFAULT_MSG);
          else {
              write("The sky is dark with night.\n");
            if(query_phase_name() != "new")
              write(sprintf("There is a %s moon.\n", query_phase_name()));
          }
          break;
        case "sky":
          switch(tod) {
            case "dawn":
              write("It is lit with the colours of a brand new day.\n");
              break;
            case "day":
              if(ansi) write(bold(yellow("The sun lights up the daytime sky."))+"\n");
              else write("The sun lights up the daytime sky.\n");
              break;
            case "twilight":
              if(ansi) write(bold(magenta("The sun is fading over the western horizon."))+"\n");
              else write("The sun is fading over the western horizon.\n");
              break;
            case "night":
            if(query_phase_name() == "new")
                write("The night time sky is moonless and dark.\n");
              else {
                write("The sky is darkened with night.\n");
                if(query_phase_name() != "new")
                  write(sprintf("There is a %s moon in the sky.\n",query_phase_name()));

              }
              break;
          }
          break;
	    case "moon":
          if(tod != "night" || query_phase_name() == "new")
            write(DEFAULT_MSG);
          else {
            if(ansi) write(sprintf("The %s moon is hovering in the night time sky.\n",query_phase_name()));
            else write(sprintf("The %s moon is hovering in the night time sky.\n",query_phase_name()));
          }
          break;
    }
}
