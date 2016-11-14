# Automated Release Instructions for Drupal 7


1. Release instructions are applied via
*drush ri*
and the executed RI are being stored in the variable *ri_executed*

2. Not to check all the modules at once there's a hook that lets system know that the module contains the release instructions: hook_ri()
```
// we don't want to do here. yet..
function MODULE_ri() {
  return TRUE;
}
```

3. Release instructions should be stored per ticket in the files located in MODULEPATH/ri/TICKET.ri.inc, e.g. path sites/all/modules/custom/queueapi/ri/test001.ri.inc.
There is NO restriction on the file name but for readability sake you may want to stick to ticket name.

4. File with the release instructions should contain the functions named according to the convention, if the file is TICKET.ri.inc then we try to execute functions ticket_ri_N(), for the example above they should be test001_ri_1() or test001_ri_7000(), - they will be executed in the numeric order so there's a way to add additions.
