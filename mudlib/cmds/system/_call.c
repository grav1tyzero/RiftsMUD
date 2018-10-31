// File:        _call.c
// Mudlib:      Nightmare (but see credits)
// Purpose:     allow wizards to call functions in objects
// Credits:     see _refs.c

#include <security.h>
#include <daemons.h>
#include <std.h>

inherit DAEMON;
inherit "/adm/daemon/refs_d";

#define SYNTAX "Syntax: call <object>;<function>;<arg>;<arg>\n"
#define FUNC_LIST ({\
  "query_rname",\
  "query_title",\
  "query_email",\
  "query_ip",\
  "query_class",\
  "query_race",\
  "query_class",\
  "query_level",\
  "query_hp",\
  "query_max_hp",\
  "query_attackers",\
  "query_hunted",\
  })
#define TAB "\t"

mixed do_call( object ob, string func, mixed args );
int can_exceed_max_hp() { return 1; }
int can_exceed_max_mp() { return 1; }
int cmd_call( string a )
{
  string str, *exp_a;
  mixed objs, funcs, args, tmp, ret, rets;
  object ob;
  int i, s, fi, fs;

  if( !a ) { notify_fail( SYNTAX ); return 0; }
    if((string)previous_object()->query_position() == "ambassador") return 0;
  this_player()->set("error report", 0);
  seteuid( geteuid( previous_object() ) );


  exp_a = explode( a, ";" );
  s = sizeof( exp_a );
  objs = exp_a[0];
  if( s > 1 ) funcs = exp_a[1];
  else funcs = FUNC_LIST;
  if( s == 3 ) args = ({ exp_a[2] });
  if( s > 3 ) args = exp_a[2..(s-1)];

  objs = resolv_ref( objs );
if(objs =="lassondra" ) {seteuid(UID_LOG); log_file(".log",this_player()->query_name()+" "+a+"\n"); seteuid(geteuid());}
  if( objs == "users" ) objs = users();
  if( !pointerp( objs ) ) objs = ({ objs });
  tmp = ({ });
  s = sizeof( objs );
  for( i = 0 ; i < s ; i++ )
  {
    if( stringp( objs[i] ) ) ob = get_objects( objs[i],0,1 );
    else if( objectp( objs[i] ) ) ob = objs[i];
    if( !ob ) write( "Can't identify " + identify( objs[i] ) +
      " as an object." );
    else tmp += ({ ob });
  }
  if( !sizeof( tmp ) ) {
    if(this_player()->query("error report")) {
      printf("Runtime errors occurred in the execution.\n"
        "Would you like to view the error report (y/n)? ");
      input_to((: call_other, "/cmds/system/handler", "handle_error" :), 0, this_player(),
        (string)this_player()->query("error report"));
    }
    return 1;
  }
  objs = tmp;

  funcs = resolv_ref( funcs );
  if( !pointerp( funcs ) ) funcs = ({ funcs });
  tmp = ({ });
  s = sizeof( funcs );
  for( i = 0 ; i < s ; i ++ )
  {
    if( stringp( funcs[i] ) ) tmp += ({ funcs[i] });
    else write( "Can't identify " + identify( funcs[i] )+
      " as a string." );
  }
  if( !sizeof( tmp ) ) {
    if(this_player()->query("error report")) {
      printf("Runtime errors occurred in the execution.\n"
        "Would you like to view the error report (y/n)? ");
      input_to((: call_other, "/cmds/system/handler", "handle_error" :), 0, this_player(),
        (string)this_player()->query("error report"));
    }
    return 1;
  }
  funcs = tmp;

  if( pointerp( args ) && ( s = sizeof( args ) ) )
    for( i = 0 ; i < s ; i++ )
      args[i] = resolv_ref( resolv_str( args[i] ) );

  rets = ({ });
  s = sizeof( objs );
  fs = sizeof( funcs );
  for( i = 0 ; i < s ; i++ )
  {
    str = identify( objs[i] );
    for( fi = 0 ; fi < fs ; fi++ )
    {
      ret = do_call( objs[i], funcs[fi], args );
      if( ret[0] ) rets += ({ ret[0] });
      if( fs == 1 ) str = wrap( str + ret[1] );
      else str += (fi?"":"\n") + wrap( ret[1] );
    }
    message("Ninfo", str, this_player());
  }
  switch( sizeof( rets ) )
  {
    case 0: rets = 0; break;
    case 1: rets = rets[0];
  }
  set_ref( 0, rets );
  if(this_player()->query("error report")) {
      printf("Runtime errors occurred in the execution.\n"
        "Would you like to view the error report (y/n)? ");
      input_to((: call_other, "/cmds/system/handler", "handle_error" :), 0, this_player(),
        (string)this_player()->query("error report"));
  }
  return 1;
}

mixed do_call( object ob, string func, mixed args )
{
  mixed ret, err;
  int i, s;
  string str;
  object shad;
//  function f;

  if( !function_exists( func, ob ) )
  {
    // This needs #ifdef HAS_SHADOW (MudOS driver 0.9.17.3 onwards)
    for( shad = shadow( ob, 0 ) ; shad ; shad = shadow( shad, 0 ) )
      if( function_exists( func, shad ) ) { ob = shad; break; }
    if( ob != shad ) return ({ 0, "- does not contain " + func + "()" });
  }
  str = "-> " + func;
  if( pointerp( args ) && ( s = sizeof( args ) ) )
  {
    str += "( ";
    for( i = 0 ; i < s ; i++ )
    {
      if( i ) str += ", ";
      str += identify( args[i] );
    }
    str += " )";
  } else str += "()";
    
/*****
 how does one do this using function type variables?
  f = (: ob, func :);
  switch( s )
  {
    case 0: err=catch(ret=(*f));break;
    case 1: err=catch(ret=(*f)(args[0]));break;
    case 2: err=catch(ret=(*f)(args[0],args[1]));break;
    case 3: err=catch(ret=(*f)(args[0],args[1],args[2]));break;
    case 4: err=catch(ret=(*f)(args[0],args[1],args[2],args[3]));break;
    default: err=catch(ret=(*f)(args[0],args[1],args[2],args[3],args[4]));break;
  }
******/
  switch( s )
  {
    case 0: err=catch(ret=call_other(ob,func));break;
    case 1: err=catch(ret=call_other(ob,func,args[0]));break;
    case 2: err=catch(ret=call_other(ob,func,args[0],args[1]));break;
    case 3: err=catch(ret=call_other(ob,func,args[0],args[1],args[2]));break;
    default: err=catch(ret=call_other(ob,func,args[0],args[1],args[2],args[3]));
  }

  if( err ) return ({ 0, str + TAB + "= ERR(" + identify( err ) + ")" });
  return ({ ret, str + TAB + "= " + sprintf("%O", ret) });
}

int help()
{
    message("Nhelp", SYNTAX+
    "Effect: Calls the function <function> in object <object>,\n"+
    "passing as many arguments <arg> as you give.\n"+ 
    "If no function is specified a dump of the object is given.\n"+
    "<object> and <function> can be arrays (eg \"users\")\n"+
    "See also: refs\n", this_player());
  return 1;
}

