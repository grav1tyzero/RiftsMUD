//	/std/living.c
//	from the Nightmare mudlib
//	inheritable code for living things
//	created by Descartes of Borg september 1992
// Modified by Geldron 030696 to add did_command(x) and fix add_exp2() so
// it actually _will_ log
// Modified by Geldron 031096 to work with arena
// Modified by Geldron 031096 to work with equip/unequip


#include <security.h>
#include <daemons.h>
#include <party.h>
#include <dirs.h>
#include <living.h>

int invis, ok_to_heal, player_age;
static int forced, spiritual, physical;
static int next_lev_exp;
int sight_bonus;
string description;
static string party;
static string *search_path;
static int login_flag;
private string gender;
mapping stats;
static mapping stat_bonus;
mapping languages;
string primary_language;
mapping language_exp;
static int dev_rate_int;

private int xp_to_next_dev;

void did_command(string str);
void set_stats(string str, int x);
void adjust_biorhythms();
void set_invis();
int query_physical();
int query_spiritual();
string query_alignment();
int query_stats(string str);
int query_stat_bonus(string stat);
int query_base_stats(string stat);
int query_poisoning();
int query_potion_healing();
void add_poisoning(int x);
void add_potion_healing(int x);
string query_gender();
void set_gender(string str);
string query_objective();
string query_possessive();
string query_subjective();
int query_stoned();
int query_intox();
int query_stuffed();
int query_quenched();
int query_invis();
void adjust_exp();
int max_buffer_size();
string query_party();
string query_long(string unused);
static void init_path();
int get_dev_rate();
void remove();
int query_exp();
void set_exp(int exp);

static void init_living() {
    add_action("cmd_hook", "", 1);
    init_path();
    init_attack();
}

void override_add_exp(int exp) {
   
}

int buffer_full() {
    return 1;
}

void adjust_exp() {
  
    return;
}

void reset_xp_to_dev()
{
 
 return;
}

void reset_max_exp() {
    
    return;
}

nomask int at_max_exp() {
    
    return 0;
}

int percent_buffer() {
    return 100;
}

int max_buffer_size() {
    return 0;
}

static void init_path() {
    string tmp;

    search_path = ({ DIR_MORTAL_CMDS, DIR_CLASS_CMDS });
    if(ambassadorp(this_object()) ||
      wizardp(this_object()))
	search_path += ({ DIR_AMBASSADOR_CMDS, DIR_SYSTEM_CMDS });
    if(high_mortalp(this_object()) || wizardp(this_object()))
	search_path += ({ DIR_HM_CMDS });
      
    if(wizardp(this_object())) {
	search_path += ({ DIR_CREATOR_CMDS });
	if(file_size(user_path(query_name()) + "bin") == -2)
	    search_path += ({ user_path(query_name())+"bin" });
        if(member_group( (string)this_object()->query_name(), "mentor") ||
           member_group( (string)this_object()->query_name(), "superuser") )
	    search_path += ({ DIR_MENTOR_CMDS });
	if(archp(this_object()))
	    search_path += ({ DIR_ADMIN_CMDS });
    }
}

static void init_stats() { stats = ([]); }

/* Added did_command(cmd); so shadowing would work more efficiently
 * - Geldron 030696
 */
nomask static int cmd_hook(string cmd) {
    string file, verb, abcmd;

    verb = query_verb();
    did_command(cmd);
    if(!cmd) abcmd = 0;
    else if(sizeof(explode(cmd, " ")) > 4)
      abcmd = implode(explode(cmd, " ")[0..3], " ");
    else abcmd = cmd;
    if( verb != "quit" && query_paralyzed()) {
	message("my_action", sprintf("%s", (string)this_player()->query_paralyze_message()),
	  this_player());
	return 1;
    }
    if(!(file = (string)CMD_D->find_cmd(verb, search_path))) {
	if(!((int)SOUL_D->do_cmd(verb, abcmd))) 
	    return (int)CHAT_D->do_chat(verb, cmd);
	else return 1;
    }
    return (int)call_other(file, "cmd_"+verb, cmd);
}

void did_command(string str) {
}

int force_me(string cmd) {
    string tmp;
    int res;

    if(previous_object())
	tmp = geteuid(previous_object());
    else tmp = "";
    /*  Added by Geldron 031096 to work with equip/unequip */
    if(PO && BN(PO) && (BN(PO) == "/cmds/mortal/_equip" || 
	    BN(PO) == "/cmds/mortal/_unequip")) {
	    res = command(cmd);
	    return res;
    }
    if(this_object()->is_player() && origin() != "local" &&
      (tmp != UID_ROOT && tmp != UID_FORCE && tmp != geteuid()) &&
      (!previous_object() || (object)previous_object()->query_caster() != this_object()))
	return 0;
    forced = 1;
    res = command(cmd);
    forced = 0;
    return res;
}

void reduce_stats() {
    
}

int sort_limbs(string one, string two) {
    string a_one, a_two;
    mapping tmp;

    tmp = (mapping)RACE_D->query_limb_info(one, query("race"));
    if(!mapp(tmp))
	a_one = "";
    else 
	a_one = tmp["attach"];
    tmp = (mapping)RACE_D->query_limb_info(two, query("race"));
    if(!mapp(tmp))
	a_two = "";
    else
	a_two = tmp["attach"];
    if(a_one == two) return -1;
    if(a_two == one) return 1;
    return strcmp(one, two);
}

void do_healing(int x) {
}

void set_severed(string limb) {
    
    return;
}

int calculate_healing() {
    
}

void set_party(string str) {
    if(getuid(previous_object()) != UID_ROOT) return;
    party = str;
}

int add_stoned(int x) {
  
}

int query_stoned() {
    
}


void add_poisoning(int x) {
    
}

void add_potion_healing(int x) {
    
}

void set_stats(string str, int x) {
    if(!stats) stats = ([]);
    //TODO: this weight should be a calculation not a 'set thing' -parnell 2018
    if(str == "ps") {
        if(x <= 16)
	        this_object()->set_max_internal_encumbrance(x * 10);
        else if(x > 16)
            this_object()->set_max_internal_encumbrance(x * 20);
        //else if robot
        //this_object()->set_max_internal_encumbrance(x * 25);
        //else if supernatural 
        //this_object()->set_max_internal_encumbrance(x * 20);
        //and ps >=18
        //this_object()->set_max_internal_encumbrance(x * 50);
    }
    stats[str] = x;
}

void set_invis() {
    if(invis) {
	invis = 0;
	write("You step out of the shadows.");
	say(query_mvis());
    }
    else {
	write("You fade into the shadows.");
	say(query_minvis()+"");
	invis = 1;
    }
}

void set_description(string str) { description = str; }

void add_sight_bonus(int x) { sight_bonus += x; }

void add_search_path(string dir) {
    log_file("paths", query_name()+": "+dir+" ("+ctime(time())+")\n");
    if(member_array(dir, search_path) == -1) search_path += ({ dir });
}

void delete_search_path(string dir) {
    if(getuid(previous_object()) != UID_ROOT) return;
    if(member_array(dir, search_path) != -1) search_path -= ({ dir });
}

void add_exp(int x) {
    set_exp(query_exp()+x);
}

int add_intox(int x) {
    
}

int add_stuffed(int x) {
    
}

int add_quenched(int x) {
    
}

void add_stat_bonus(string stat, int amount) {
    
}

string query_long(string unused) {
    object *inv;
    string *tmp;
    string pre, stuff, extra, reg, short;
    int i, x;

    if(this_object()->query_ghost()) return "An ethereal presence.\n";
    reg = "";
    pre = "You look over the "+query_gender()+" "+query("race")+".\n";
    if(::query_long("junk")) pre += ::query_long("junk")+"\n";
    if(description) pre += query_cap_name()+" "+description+"\n";
    if(severed) tmp = keys(severed);
    if(tmp && sizeof(tmp)) {
	reg += query_cap_name()+" is missing a "+tmp[0];
	if(sizeof(tmp) > 1) {
	    if(sizeof(tmp) != 2) reg += ",";
	    for(i=1; i<sizeof(tmp); i++) {
		if(i== sizeof(tmp)-1) reg += " and a ";
		reg += " " +tmp[i];
		if(i != sizeof(tmp)-1) reg +=",";
	    }
	}
	reg += ".\n";
    }
    else reg += query_cap_name() + " has no missing limbs.\n";
    x = ((player_data["general"]["hp"]*100)/player_data["general"]["max_hp"]);
    reg += "%^CYAN%^%^BOLD%^" + capitalize(query_subjective());
    if(x>90) reg += "%^CYAN%^%^BOLD%^ is in top shape.%^RESET%^\n";
    else if(x>75) reg += "%^CYAN%^%^BOLD%^ is in decent shape.%^RESET%^\n";
    else if(x>60) reg += "%^CYAN%^%^BOLD%^ is slightly injured.%^RESET%^\n";
    else if(x>45) reg += "%^CYAN%^%^BOLD%^ is hurting.%^RESET%^\n";
    else if(x>30) reg += "%^CYAN%^%^BOLD%^ is badly injured.%^RESET%^\n";
    else if(x>15) reg += "%^CYAN%^%^BOLD%^ is terribly injured.%^RESET%^\n";
    else reg += "%^CYAN%^%^BOLD%^ is near death.%^RESET%^\n";
    inv = filter_array(all_inventory(this_object()),
      (: (stringp(call_other($1, $2)) && !living($1)) :),
      "query_short");
    if(inv && sizeof(inv)) {
	reg += capitalize(nominative(this_object())) + " is carrying:\n";
	reg += implode(map_array(inv, (: call_other :), "query_short"),
	  "\n")+"\n";
    }
    else reg += capitalize(nominative(this_object())) + " is carrying nothing.\n";
    return pre+reg;
}

int query_stat_bonus(string stat) {
   
}

int query_stats(string stat) {
    if(stats && stats[stat])
        return stats[stat];
    else return 0;
}

int query_base_stats(string stat) {
    if(!stats || !stats[stat]) return 0;
    else return stats[stat];
}


nomask int query_forced() { return forced; }

string *query_search_path() {
    if(previous_object() != this_object() && geteuid(previous_object()) != UID_ROOT) return search_path + ({});
    else return search_path;
}

int query_invis() { return invis; }

int query_exp() { return player_data["general"]["experience"]; }

void set_exp(int exp) 
{
    player_data["general"]["experience"] = exp;
}

string query_alignment() { return player_data["general"]["alignment"]; }

void set_alignment(string x) { player_data["general"]["alignment"] = x; }

int query_intox() {
    if(healing) return healing["intox"];
    else return 0;
}

int query_stuffed() {
    if(healing) return healing["stuffed"];
    else return 0;
}

int query_quenched() {
    if(healing) return healing["quenched"];
    else return 0;
}

int query_poisoning() {
    if(healing) return healing["poisoning"];
    else return 0;
}

int query_potion_healing() {
    if(healing) return healing["potion"];
    else return 0;
}


string query_party() { return party; }

string query_al_title() {
    return "";
}

int query_sight_bonus() { return sight_bonus; }

int query_age() { return player_age; }

string *query_all_stats() { return keys(stats); }

string query_description() { return description; }

void remove() {
    int i;
    object *inv;

    inv = all_inventory(this_object());
    for(i=0; i<sizeof(inv); i++) {
	if(!inv[i]) continue;
	if(inv[i]->drop() && inv[i]) inv[i]->remove();
	/* Some objects call remove() in drop() */
    }
    ::remove();
}

void adjust_biorhythms() {
    
}

int query_spiritual() { return spiritual; }

int query_physical() { return physical; }

string query_primary_lang() {
    if(!primary_language && living(this_object()) &&
      !this_object()->is_player()) return "common";
    return primary_language;
}

void set_primary_lang(string str) { primary_language = str; }


// language system added by Valodin in August 1993

int query_lang_prof(string lang) 
{ 
    if(wizardp(this_object())) return 10;
    if(!languages)
	return 0;
    if(!languages[lang]) 
	return 0;
    return languages[lang];
}

int set_lang_prof(string lang, int i)
{
    if(!languages)
	languages = ([]);
    if(i > 10)
	i = 10;
    languages[lang] = i;
    return languages[lang];
}

string *query_all_languages()
{
    if(!languages)
	return ({});
    return keys(languages);
}

int remove_language(string lang)
{
    if(!languages || !languages[lang])
	return 0;
    map_delete(languages, lang);
    return 1;
}


// each mapping element is an array of two ints
// The first is the number of language points amassed.  When this reaches
// (lang_prof + 1) ^ 4, the language prof advances and the language points
// are decreased.  The second number is the number of excess exp points
// have been spent.  When exp is spent, it is converted to lang_pts based
// on intelligence.  it takes (40 - int) exp pts to make one lang_pt
// -Valodin

void learn_language(string lang, int exp)
{
    int tot_exp, tmp, goal, lang_pts, intel_fac;

    if(lang == "coderish" && !wizardp(this_object()))
	return;
    if(mapp(languages) && languages[lang] >= 10) return;
    if(!language_exp)
	language_exp = ([]);
    if(!language_exp[lang] || !pointerp(language_exp[lang]))
	language_exp[lang] = ({ 0, 0});
    tot_exp = (language_exp[lang][0] += exp);
    intel_fac = (110 - query_stats("intelligence"));
    if(intel_fac > 25) intel_fac = 20;
    if(intel_fac <= 0) intel_fac = 5;
    language_exp[lang][1] = tot_exp / intel_fac;
    language_exp[lang][0] = tot_exp % intel_fac;
    if(language_exp[lang][1] >= (to_int(pow(to_float(languages[lang]),1.1)) +1)) {
	language_exp[lang][1] = 0;
	languages[lang] += (languages[lang] >= 10)?0:1;
    }
    return;
}

int *query_lang_exp(string lang)
{
    if(!language_exp)
	return ({ 0, 0 });
    if(!language_exp[lang])
	return ({ 0, 0});
    return language_exp[lang];
}

void set_gender(string str) { 
    if(str != "male" && str != "female" && str != "neuter") return;
    gender = str;
}

string query_gender() { return (gender ? gender : "neuter"); }

string query_subjective() { return nominative(gender); }

string query_possessive() { return possessive(gender); }

string query_objective() { return objective(gender); }

int get_dev_rate()
{
    return 0;
}
