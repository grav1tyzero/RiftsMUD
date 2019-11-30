// Language daemon create by Valodin as an experimental racial language
// reference
inherit DAEMON;
mapping language_list;

void create()
{
  ::create();
  language_list = (["human":"american"]);
}

void init_languages(object ob)
{
  string race, tmp;
  int prof;

  race = ob->query_race();
  prof = 10;
  if(sscanf(race, "half-%s", tmp) == 1)
  {
    race = tmp;
    ob->set_lang_prof(language_list["human"], 6);
    prof = 6;
  }
  if(tmp = language_list[race])
  {
    ob->set_lang_prof(tmp, prof);
    ob->set_primary_lang(tmp);
    return;
  }
  else
  {
    ob->set_lang_prof(race + "ish", prof);
    ob->set_primary_lang(race + "ish");
    return;
  }
}

string *all_languages()
{
  return values(language_list);
}

string *all_races()
{
  return keys(language_list);
}
