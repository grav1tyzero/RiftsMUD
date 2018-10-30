/*
/adm/simul_efun/roll.c
For RiftsMUD
by Parnell 2018

interprets a string dice roll to a value.
*/
private static int die(int number, int sided) {
  int retval;
  if(number < 1)
    return 0;
  if(sided < 1)
    return 0;
  for(int x=0;x<number;x++) {
    retval += (random(sided) + 1);
  }
  return retval;
}
int roll(string roll_val) {
  int match,number=1, die=6, multiplier=1, addend=0;

  match = sscanf(roll_val,"%dd%dx%d+%d",number,die,multiplier,addend);
  if(match < 2)
    throw("Invalid args expected [numdice]d[sides]x[multiplier]+[addend] got:" + roll_val);
  if(match==2)
    match = sscanf(roll_val,"%dd%d+%d",number,die,addend);
  return die(number,die) * multiplier + addend;
  
}