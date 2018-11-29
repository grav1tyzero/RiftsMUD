//	/std/living/combat.c
//	from the Nightmare mudlib
//	code for combat
//	based upon the LOS combat by Buddha@TMI
//	DarkeLIB 0.1
//	Diewarzau 1994
// Modified by Geldron 031096 to fix HeartBeat prob
// Modified by Geldron 031096 to work with arena

#include <std.h>
#include <party.h>
#include <daemons.h>
// Remove non-standard includes out of std files!
// Security and stability risk.

#pragma optimize
#define KILL_RANGE 5
#define DEFAULT_PARALYZE_MESSAGE "You are stiff as stone."

inherit BODY;
inherit SKILLS;
//savables and props
int wimpy;
private string wimpydir;
private int _attacks_per_melee = 1;
int query_attacks_per_melee() {return _attacks_per_melee;}
void set_attacks_per_melee(int atk) {_attacks_per_melee = atk;}

//ephemeral
static int magic_round;
private static int paralyzed;
private static int hunting;
private static object *attackers;
private static int any_attack;
private static int casting;
private static object *hunters;
private static object *pres_hunters;
private static mapping magic_protection;
private static string paralyze_message;
private static string target_thing;
private static object current;

int ok_to_kill(object targ);
void miss(string how, string type);
void set_paralyzed(int x, string drow);
void remove_paralyzed();
int query_paralyzed();
int clean_up_attackers();
mapping get_damage(object weap);
void run_away();
void send_messages(string *mesgs);
void set_wimpydir(string str);
void set_wimpy(int x);
string query_wimpydir();
int query_wimpy();
int kill_ob(object victim, int which);
int sight_adjustment(object who);
void continue_attack();
void update_current_skill();
void cease_all_attacks();
private string check_up(string limb);
void execute_attack();
string query_paralyze_message();
int check_limbs_for_attack();
object party_leader(object att);
//used by execute attack **********
int protection, hits, pois, i, j, k, num_attacks, x, defendflag;
int parry_bonus, has_shield, skill, y;
//*************

//  This function is used to initialize various variables

void init_attack() {
    init_complex_body();
    hunters = ({});
    attackers = ({});
    pres_hunters = ({});
    wimpydir = "out";
}

// Remove dead or non-existing objects out of the list of attackers
// Return true if there are still attackers around

int clean_up_attackers() {
    object *hunters_tmp, *attackers_tmp, *phunters_tmp;
    object first;
    int i;

    attackers_tmp = ({});
    hunters_tmp = ({});
    phunters_tmp = ({});
    i = sizeof(attackers);
    if(i) first = attackers[0];
    while(i--) {
		if(!attackers[i] || !objectp(attackers[i])) 
			continue;
		if(attackers[i]->shadow_form() ||
		this_object()->shadow_form()) {
			if(environment(attackers[i]) == environment())
				phunters_tmp += ({ attackers[i] });
			else
				hunters_tmp += ({ attackers[i] });
			continue;
		}
		if(attackers[i]->query_ghost()) 
			continue;
		if(environment(attackers[i]) != environment(this_object()))
			hunters_tmp += ({ attackers[i] });
		else attackers_tmp += ({ attackers[i] });
    }
    i = sizeof(hunters);
    while(i--) {
		if(!hunters[i] || !objectp(hunters[i])) 
			continue;
		if(hunters[i]->query_ghost()) 
			continue;
		if(hunters[i]->shadow_form()) {
			if(environment() == environment(hunters[i]))
				phunters_tmp += ({ hunters[i] });
			else
				hunters_tmp += ({ hunters[i] });
			continue;
		}
		if(environment(hunters[i]) == environment(this_object())) {
			if(hunters[i]->query_invis() && !this_object()->query("see invis")) {
				phunters_tmp += ({ hunters[i] });
				continue;
			}
			if(hunters[i]->query_stealth() &&
			skill_contest((int)hunters[i]->query_stealth(),
			(int)this_object()->query_skill("perception"), 1) == 1) {
				phunters_tmp += ({ hunters[i] });
				continue;
			}
			attackers_tmp += ({ hunters[i] });
		}
		else hunters_tmp += ({ hunters[i] });
    }
    i = sizeof(pres_hunters);
    while(i--) {
         if(!pres_hunters[i]) continue;
	if(environment(pres_hunters[i]) != environment()) {
	    hunters_tmp += ({ pres_hunters[i] });
	    continue;
	}
	if(pres_hunters[i]->query_stealth() ||
	  (pres_hunters[i]->query_invis() && !this_object()->query("see invis")) ||
	  pres_hunters[i]->shadow_form() ||
	  this_object()->shadow_form()) {
	    phunters_tmp += ({ pres_hunters[i] });
	    continue;
	}
	attackers_tmp += ({ pres_hunters[i] });
    }
    attackers = attackers_tmp;
    i = member_array(first, attackers);
    if(i > 0) {
	attackers[i] = attackers[0];
	attackers[0] = first;
    }
    hunters = hunters_tmp;
    pres_hunters = phunters_tmp;
    any_attack = sizeof(attackers);
    hunting = sizeof(hunters);
    return any_attack;
}

//  This is the kill command.  If the victim is not yet attacked
//  then it is entered first into the list of victims.  If it
//  is already in the list, then it becomes the first entry

int kill_ob(object victim, int which) {
    object *borg_people;
    
    int i;
    string to_owner, // this_object()->query_owner(); (cpu reduction)
    v_owner,  // victim->query_owner();        (cpu reduction)
    to_name,  // this_object()->query_name();  (cpu reduction)
    v_name;   // victim->query_name();         (cpu reduction)

    //  Add check to see if victim is already in attackers FIRST,
    //      to save cpu.
    //  -Diewarzau 3/31/96

    to_owner = (string)this_object()->query_owner();
    v_owner = (string)victim->query_owner();
    to_name = (string)this_object()->query_name();
    v_owner = (string)victim->query_name();

    if(attackers)
	i = member_array(victim, attackers);
    else i = -1;
    if( i>-1 ) {
	if(attackers[0] == victim) {
	    if(!which)
		return 1;
	}
	attackers[i] = attackers[0];
	attackers[0] = victim;
	return 1;
    }
    if(victim == this_object()) return 0;
    if(victim->is_pet() && v_owner == to_name) return 0;
    if(this_object()->is_pet()) {
	if(to_owner == v_name || to_owner == v_owner) return 0;
    }
    borg_people = (object *)victim->query_attackers();
    if(victim->is_player())
	HUNTING_D->add_hunter(v_name, this_object());
    if(!borg_people) borg_people = ({});
    if(member_array(this_object(), borg_people) == -1)
	if(!ok_to_kill(victim)) {
	    message("my_combat", sprintf(
		"Supernatural forces prevent you from attacking %s.",
		(string)victim->query_cap_name()),
	      this_object());
	    return 0;
	}
	/* changed from 0 to 1 to faciliate PK patch */
    if(this_object()->query_attacked()) return 1;
    if(attackers) i = member_array(victim, attackers);
    else i = -1;
    attackers = ({ victim }) + attackers;
    any_attack = 1;
    if(!which) victim->kill_ob(this_object(), 1);
    return 1;
}

void set_attackers(object *what) {
    attackers = what;
    any_attack = 1;
}

//  Called from heart_beat
//  Here is where we see if we are dead or in combat

void continue_attack() {
    object *death, me;
    int xxx;
    me = this_object();

    if(!environment(me) || me->query_ghost()) return;
    if(me->is_player() && !interactive(me)) return;
    if(query_hp() < 1 && !me->query_ghost() ) {
		if(wizardp(me)) {
			set_hp(1);
			message("my_combat", "You are immortal and cannot die.",me);
		}
		else {
			if(attackers) {
				if(sizeof(attackers) && attackers[0]) {
					attackers[0]->add_kill((string)me->query_name());
					add_death((string)attackers[0]->query_name());
					if(me->is_player()) {
						xxx = sizeof(death = all_inventory(attackers[0]));
						while(xxx--) 
							death[xxx]->notify_kill(me);
					}
					me->die((attackers && attackers[0] ? attackers[0] : 0));
					attackers = ({});
					return;
				}
			}
		}
    }
    if(!any_attack && !hunting) 
		return;
    if(!clean_up_attackers()) 
		return;
    if(environment(this_object())->query_property("no attack")) 
		return;
    if( (player_data["general"]["hp"] * 100) / player_data["general"]["max_hp"] < wimpy ) {
		run_away();
		return;
    }
    if(!query_paralyzed() && check_limbs_for_attack()) 
		execute_attack();
    else if(!check_limbs_for_attack())
		message("my_combat", "You may not attack with no wielding limbs!", this_object());
    else
		message("my_combat", query_paralyze_message(), this_object());
}

object party_leader(object att) {
    string party;
    object ret;

    if(!att->is_player()) 
		return 0;
    if(att->query_ghost()) 
		return 0;
    party = (string)PARTY_OB->party_member(att);
    if(!party) 
		return 0;
    ret = (object)PARTY_OB->query_leader(party);
    if(ret && environment(ret) == environment(att) && ret != att) 
		return ret;
    return 0;
}

void execute_attack() {
    object *weapons, *a_weapons, ldr, me;
    mapping damage;
    string *tmp;
    string *criticals = ({});
    mixed w_hit;
	int trace = 0;
    int spec_com;
    protection = hits = pois = i = j = k = num_attacks = x = defendflag = 0;
    parry_bonus = has_shield = skill = y = 0;


    me = this_object();
	trace = (wizardp(me) && (string)me->getenv("TRACE") == "on");
    spec_com=0;
    if(member_array(me, (object *)attackers[0]->query_attackers()) < 0)
		attackers[0]->kill_ob(me, 1);
    ldr = party_leader(attackers[0]);
	if(!num_attacks)
		num_attacks = 1;//set default to 1.

	for(i=0; i<num_attacks; i++) 
	{
	    defendflag = 0;
	    target_thing = (string)attackers[0]->return_target_limb();
	    if(!sizeof(weapons))
			current = 0;
	    else
			current = weapons[i%sizeof(weapons)];
	    a_weapons = distinct_array(attackers[0]->query_wielded());
	    //TODO: do WP checks here

          
        message("info", "To hit skill: "+skill, me);
		
	    if(0 /*dodge roll*/ && !attackers[0]->query_paralyzed()) {
			miss("dodge",(current)?(string)current->query_type() : 0);
			defendflag = 1;
			continue;
	    } else if(check_limbs_for_attack() && 0 /*parry roll*/) {
			miss("parry", (current)?(string)current->query_type() : 0);
			defendflag = 1;
			continue;
	    } else
			damage = get_damage(current);
	    if(!damage || !keys(damage)) 
			continue;

	    if(current) {
			w_hit = current->query_hit();
			if(functionp(w_hit)) 
				w_hit = evaluate(w_hit, attackers[0]);
			else if(stringp(w_hit)) 
				message("my_combat",w_hit,me);
	    }
	    tmp = keys(damage);
	    if(w_hit && mapp(w_hit))
		tmp = distinct_array(tmp + keys(w_hit));
	    x = 0;
	    for(k=0;k<sizeof(tmp);k++) {
			if(mapp(w_hit) && !undefinedp(w_hit[tmp[k]])) {
				if(undefinedp(damage[tmp[k]])) damage[tmp[k]] = w_hit[tmp[k]];
				else damage[tmp[k]] += w_hit[tmp[k]];
			}
	    }
	    if(!criticals || !sizeof(criticals) || x > 0) {
			send_messages(
				(string *)DAMAGE_D->attack_message(
					sprintf("%d %s %s:%s", 
						x, 
						(current)?(string)current->query_verb() :"hit", 
						(string)me->query_name(), 
						(string)attackers[0]->query_name()
					)//sprintf
				)//attack_message
			);//send_messages
			x = (int)attackers[0]->do_damage(target_thing, x); 
		}
	}
	if(x > 0) attackers[0]->check_on_limb(target_thing);
}

void stop_hunting() {
    hunting = 0;
    hunters = ({});
}
void cease_all_attacks() {
    attackers = ({});
    any_attack = 0;
}

object query_current_attacker() {
    if(!sizeof(attackers)) return 0;
    else return attackers[0];
}

void run_away() {
    string *str;

    str = (string *)environment(this_object())->query_exits();
    if(!str || !sizeof(str)) {
	message("my_combat", "There is nowhere to run!!!", this_object());
	return;
    }
    message("my_combat", "You run away.", this_object());
    if(member_array(wimpydir, str) != -1) this_object()->force_me(wimpydir);
    else this_object()->force_me(str[random(sizeof(str))]);
}

void set_wimpydir(string str) {
    if(!stringp(str)) return;
    wimpydir = str;
}

void set_wimpy(int x) { wimpy = x; }

string query_wimpydir() { return wimpydir; }

int query_wimpy() { return wimpy; }

mapping get_damage(object weap) {
    return ([]);
}

void set_casting(int x) { casting = x; }

int query_casting() { return casting; }


void send_messages(string *mesgs) {
    if(sizeof(mesgs) != 3) mesgs = ({ "BUG in message sender",
	  "BUG in message sender", "BUG in message sender" });
    if(strlen(mesgs[0]))
		message("my_combat",mesgs[0],this_object());
    if(strlen(mesgs[1]))
		message("other_combat",mesgs[1],environment(this_object()),
	  ({ this_object(), attackers[0] }));
    if(strlen(mesgs[2]))
		message("my_combat",mesgs[2],attackers[0]);
    return;
}

object *query_hunted() { return hunters + pres_hunters; }

object *query_attackers() {
    if(!attackers) return ({});
    if(!sizeof(attackers)) return ({});
    return attackers;
}

int sight_adjustment(object who) {
      int elight, ret;

    ret = 0;
    if(!pointerp(attackers) || !sizeof(attackers)) return 0;
    elight = effective_light(who);
    if(elight >6 || elight < 1)
	ret = random(20);
    if((int)this_object()->query_property("light") > 0)
	ret -= random(10);
    else if((int)this_object()->query_property("light") < 0)
	ret += random(10);
    if((int)this_object()->query_invis() && !(int)who->query("see invis")) {
	ret += 60 - ((int)who->query_skill("perception") / 3);
    }
        return ret;
}

void miss(string how, string type) {
    string you, me;

    you = (string)attackers[0]->query_cap_name();
    me = query_cap_name();
    switch(lower_case(how)) {
    case "block":
	send_messages(({ "Your attack is blocked by "+you+"'s shield.",
	    you+" blocks "+me+"'s attack deftly.",
	    "You skillfully block "+me+"'s attack with your shield." }));
	break;
    case "parry":
	send_messages(({ you+" skillfully parries your feeble attack.",
	    you+" bats "+me+"'s attack aside with a skillful parry.",
	    "You deftly parry "+me+"'s attack." }));
	break;
    case "dodge":
	send_messages(({ you+" dodges your attack.",
	    you+" sidesteps "+me+"'s attack.",
	    "You dodge "+me+"'s attack." }));
	break;
    default:
	send_messages(({ "You miss "+you+".",me+" misses "+you+".",
	    me+" misses you." }));
    }
    return;	
}

void set_paralyzed(int x, string drow) {
    int res;

    if((int)this_object()->query_paralyzed() > 0) return;
    if(wizardp(TO)) {
	message("my_action", "Your Immortality just saved you from being "
	  "paralyzed.", this_object());
	return;
    }
    if(this_object()->query("no stun")) {
      message("info", "%^CYAN%^%^BOLD%^You resist being stunned!",
	this_object());
      return;
    }
    res = (int)this_object()->query_skill("resist stun");
    if(res) {
      res = random(res) + res/3;
      if(res >= 10)
	message("info", "%^MAGENTA%^%^BOLD%^You resist "+res+
		"% of the stun!", this_object());
      x -= x * res / 100;
    }
    if(x <= 0) return;
    if(!drow)
	paralyze_message = DEFAULT_PARALYZE_MESSAGE;
    else
	paralyze_message = drow;
    paralyzed = 1;
    delayed_call("remove_paralyzed", x);
}

string query_paralyze_message() { return paralyze_message; }

int query_paralyzed() { return paralyzed; }

void remove_paralyzed() { 
    if(!this_object()) return;
    remove_call_out("remove_paralyzed");
    paralyzed = 0;
    message("my_combat", "%^YELLOW%^YOU CAN MOVE AGAIN!!!%^RESET%^", this_object());
}


void set_magic_round(int x) { magic_round = x; }

int query_magic_round() { return magic_round; }

int ok_to_kill(object targ) {
    if(targ->is_invincible()) 
		return 0;
    if(wizardp(this_object())) 
		return 1;
    if(!this_object()->is_player() || !targ->is_player()) 
		return 1;
    return 1;
}

int check_limbs_for_attack() {
    string *which_limbs;
    string refi;
    int i, num;

    if(this_object()->query_ghost()) return 0;
    which_limbs = (string *)this_object()->query_wielding_limbs();
    if(!sizeof(which_limbs)) return 0;
    i = sizeof(which_limbs);
    num = 0;
    while(i--) {
	if(this_object()->query_crippled(which_limbs[i])) continue;
	refi = (string)this_object()->query_reference(which_limbs[i]);
	if(refi && this_object()->query_crippled(refi)) continue;
	num++;
    }
    return sizeof(which_limbs);
}
