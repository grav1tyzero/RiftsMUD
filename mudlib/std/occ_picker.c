inherit "/std/Object";

private static string *_occs = ({"Coalition Grunt"});

void create() {
    ::create();
    set_id(({"selector", "occ"}));
    set_name("OCC Picker");
    set_long("OCC Picker");
    set_short("OCC Picker");
    set_weight(0.1);
}

void remove() {
    ::remove();
}

int query_is_occ_obj() {return 1;}

void prompt_occs() {
    write("Which OCC do you want?\n");
    for(int x = 0; x < sizeof(_occs); x++) {
        write(sprintf("%d) %s\n", x+1, _occs[x]));
    }
}
void begin_selection() {
    prompt_occs();
    input_to("pick_occ");
}
void assign_base_structural(string selected) {
    int base_sdc=0, base_mdc=0;
    switch(selected) {
        case "1"://grunt
        default:
        base_sdc = roll("2d6+12");
        break;
    }
    write(sprintf("Base SDC: %d\nBaseMDC: %d\n",base_sdc,base_mdc));
    this_player()->set_max_sdc(base_sdc);
    this_player()->set_max_mdc(base_mdc);
    this_player()->set_sdc(base_sdc);
    this_player()->set_mdc(base_mdc);
}
int pick_occ(string selected) {
    int selected_int;
    if(selected == "q") {
        write("canceled\n");
        return 1;
    }
        
    selected_int = atoi(selected);
    if(!selected_int) {
        prompt_occs();
        return 1;
    }
    write(sprintf("You have selected %s\n", _occs[selected_int-1]));
    assign_base_structural(selected);
    switch(selected) {
        case "1" :
            break;
        default :
            prompt_occs();
            break;
    }
    return 1;
}
int drop() {
    write(sprintf("the %s vanishes", query_short()));
    remove();//destroy it.
    return 1;
}