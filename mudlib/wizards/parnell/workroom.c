inherit "/std/vault_locker_room.c";

void create()
{
    ::create();
    set("long", "This inherits from /std/vault_locker_room.c");
    set_exits( ({ "/wizards/parnell/workroom2.c"}), ({"south"}) );
    add_exit("/d/standard/square", "square");
}

void reset()
{
    ::reset();
}