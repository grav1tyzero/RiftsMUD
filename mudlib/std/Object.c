// File:      /std/Object.c
// Purpose:   Define functions and varibles to be used by all objects
// Mudlib:    Nightmare
// Credits:
// 92-10-**   Descartes of Borg created this, based on TMI's Object.c
// 93-06-16   Pallando reordered and commented it.
// 93-06-16   Pallando added create(), reset(), prop()
// 93-06-21   Pallando added init()
// 93-06-23   Pallando added seteuid(getuid()) to create()

// std contains the def of MOVE
#include <std.h>
// daemons contains the #define of ALCHEMIST_D
#include <daemons.h>
#include <dirs.h>
#include <clock.h>

// move contains the functions "move", "remove", "clean_up", "set_no_clean"
//   "set_last_location", "query_last_location", "set_weight", "query_weight"
inherit MOVE;

//    Global variables defined in this file
//   =======================================
mapping props;                    // Object data that does get saved.
private static mapping ob_data;   // Object data that does not get saved.
private static string cap_name;   // kept in seperate variable for security
private string true_name;         // kept in seperate variable for security
private string creator;           // File name of the ob that created an object


//    Functions defined in this file
//   ================================
//
// a) Object property functions.
//   init_ob               - initalises the "ob_data" mapping
//   set                   - stores data in the "ob_data" mapping
//   add                   - alters data in the "ob_data" mapping
//   query                 - returns data from the "ob_data" mapping
//   set_property          - alters data in the "props" mapping
//   query_property        - returns data from the "props" mapping
//   remove_all_properties - initalises the "props" mapping
//   remove_property       - removes data from the "props" mapping
//   remove_property_value - alters data in the "props" mapping
//   regexp_query_property - returns data from the "props" mapping
// b) One line set and query functions.
//   set_id,set_short,set_long,set_value,query_id,query_name,query_true_name
//   query_long,query_value,query_destroy,query_creator, set_float_value
// c) More complicated set and query functions.
//   id                    - queries if a str is an id
//   set_name              - also sets cap_name
//   query_short           - also inits ob
//   query_cap_name        - deals with invisibility and ghosts
//   query_light           - also checks if props exists
//   set_destroy           - also resets ob_data
// d) Main functions.
//   create                - initialises object
//   reset                 - handles temperature
//   init                  - future expansion - currently a dummy function
//   move                  - makes sure armor is unequipped
//   get                   - handles "keep" property





// *** Object property functions ***
// I] That use the "ob_data" mapping

void init_ob() { ob_data = ([ "id": ({}) ]); }

void set(string what, mixed arg) {
    if( !stringp(what) ) return;
    if(!ob_data) init_ob();
    if(ob_data[what]) ob_data[what] = arg;
    else ob_data += ([ what: arg ]);
}

static void add(string what, mixed arg) {
    if(!ob_data) ob_data = ([]);
    if(functionp(arg) && geteuid(this_object()) != geteuid(arg[0])) return;
    if(stringp(arg) || intp(arg)) arg = ({ arg });
    if(!ob_data[what]) {
	ob_data += ([what:arg]);
	return;
    }
    if(!pointerp(ob_data[what]) && !mapp(ob_data[what])) return;
    ob_data[what] += arg;
}

varargs mixed query(string what, mixed element) {
    mixed res;

    res = 0;
    if(!ob_data) init_ob();
    if(!stringp(what)) return 0;
    if(!ob_data[what]) return 0;
    if(!element) res = ob_data[what];
    else if( stringp(element) ) res = ob_data[what][element];
    else if( (intp(element)) && pointerp(ob_data[what]) &&
      (element < sizeof(ob_data[what])) ) res = ob_data[what][element];
    if( pointerp(res) && previous_object() != this_object() ) res += ({});
    return res;
}

// II] That use the "props" mapping

void set_property(string prop, mixed value) {
    if(!props) props = ([]);
    props[prop] = value;
}

string *query_property_keys() {
    if(!props) return ({});
    else return keys(props);
}

mapping query_all_properties() {
    return props;
}

mixed query_property(string prop) {
    if(!props)
    {
	props = ([]);
	return 0;
    }
    return props[prop];
}

void remove_all_properties() {
    props = ([]);
}

int remove_property (string prop)
{
    if(props && props[prop])
    {
	props[prop] = 0;
	return 1;
    }
    return 0;
}

int remove_property_value(string prop, mixed val) {
    if(props && props[prop]) {
	props[prop] -= val;
	return 1;
    }
    return 0;
}

string * regexp_query_property (string pattern)
{
    string *vars, *prop;

    vars = keys(props);
    prop = regexp (vars, pattern);
    if (!prop)
	prop = ({});
    return prop;
}





// *** One line set and query functions ***
void set_id(mixed val)
{
    if (stringp(val))
        set("id", ({val}));
    else if (pointerp(val))
        set("id", val);
    else
        return;
}

void   set_short(string str)     { set("short", str); }
void   set_long(string str)      { set("long", str); }
void   set_value(int x)          { set_property("value", x); }
void   set_float_value(float x)  { set_property("float value", x); }
string *query_id()               { return query("id"); }
nomask string query_name() 	 { return true_name; }
string query_true_name()         { return true_name; }
string query_long(string str) {
    string desc;

    if(functionp(ob_data["long"]))
	desc = (string)(*ob_data["long"])(str);
    else desc = ob_data["long"];
    if(!this_player()) return desc;

    return desc;
}

int    query_value()             {
    if(query_property("value")) return query_property("value");
    else if(query_property("float value"))
	return to_int(query_property("float value"));
    else return 0;
}

float query_float_value() {
    if(query_property("float value")) return query_property("float value");
    else if(query_property("value"))
	return to_float(query_property("value"));
    else return 0.0;
}

int    query_destroy()           { return ob_data["destroy"]; }
string query_creator()           { return creator; }





// *** These functions added to support savable items ***

int save_me(string file) {
if(!file) return 0;
seteuid(getuid());
if(save_object(DIR_OBJECTS+"/"+file)) return 1;
return 0;
}

int restore_me(string file) {
    int ret;

    seteuid(getuid());
    if(!file || !file_exists(DIR_OBJECTS+"/"+file+".o")) return 0;
    seteuid(getuid(this_object()));
    ret = restore_object(DIR_OBJECTS+"/"+file, 1);
    rm(DIR_OBJECTS+"/"+file+".o");
    if(query_property("new desc")) {set("short", query_property("new desc"));}
    return ret;
}

// *** More complicated set and query functions ***

int id(string str) {
    if(!ob_data) init_ob();
    if( !stringp(str) ) return 0;
    if( lower_case(str) == true_name) return 1;
    if(member_array(str, ob_data["id"]) != -1) return 1;
    return 0;
}

int plural_id(string str) {
    string *plural_id;
    int i;

    if(!ob_data) init_ob();
    if(!pointerp(ob_data["id"])) return 0;
    i = sizeof(ob_data["id"]);
    if(!i) return 0;
    plural_id = allocate(i);
    while(i--)
	plural_id[i] = pluralize(ob_data["id"][i]);
    if( !stringp(str) ) return 0;
    if( lower_case(str) == pluralize(true_name)) return 1;
    if(member_array(str, plural_id) != -1) return 1;
    return 0;
}

void set_name(string str) {
    if( !stringp(str) ) return;
    cap_name = capitalize(str);
    true_name = str;
    if(creator) return;
    if(previous_object()) creator = file_name(previous_object());
    else creator = "Unknown";
}

string query_short() {
    string desc;

    if(!ob_data) init_ob();
    if(wizardp(this_object()) && this_object()->query_invis()) return 0;
    if(functionp(ob_data["short"]))
	desc = (string)(*ob_data["short"])();
    else desc = ob_data["short"];

    return desc;
}

string query_cap_name()
{
    if (!true_name)
        return 0;
    if (!cap_name)
        return 0;
    if (this_object()->query_invis())
        return "Someone";
    if ((string)TO->query_ghost())
    {
        if ((string)TO->query_name())
            return "The ghost of " +
                   CAP((string)TO->query_name());
        else
            return "A ghost";
    }
    return cap_name;
}

int query_light() {
    if(!props) return 0;
    if(props["light"]) return props["light"];
    return 0;
}

void set_destroy() {
    if(!ob_data) ob_data = ([]);
    ob_data["destroy"] = 1;
}

int __Read(string str) {
    if(!str) return notify_fail("Read what?\n");
    if(!ob_data || !ob_data["read"])
	return notify_fail(living(this_object()) ? "Read a living thing?\n" :
	  "There is nothing on it to read.\n");
    if(functionp(ob_data["read"]))
	return (int)(*ob_data["read"])(str);
    else if(stringp(ob_data["read"])) {
	if(present(str, this_player()) != this_object()) return notify_fail(DEFAULT_MSG);
	message("info", ob_data["read"], this_player());
	message("other_action", (string)this_player()->query_cap_name()+
	  " reads the "+query_name()+".", environment(this_player()),
	  ({ this_player() }));
	return 1;
    }
    else return notify_fail("It looks all jumbled.\n");
}

void create() {
    seteuid( getuid() ); // ensures all objects have an euid.

    // Once all objects call ::create(), many of the checks that
    // ob_data and props exist can be removed, saving on cpu.
    ob_data = ([ "id": ({}) ]);
    props = ([]);
}

void reset()
{
    // do temperature
}

void init() {
    add_action("__Read", "read");
}

int move(mixed dest) {
    this_object()->unequip();
    return ::move(dest);
}

int get() {
    string who;

    who = (string)this_object()->query_property("keep");
    if(!who) return 1;
    if(who != (string)this_player()->query_name()) {
	write("A magical force prevents you from taking it!\n");
	say("A magical force prevents "+(string)this_player()->query_cap_name()+
	  " from taking the "+query_name()+".\n", this_player());
	return 0;
    }
    return 1;
}

/* EOF */
