<?php

/**
 * @return string
 */
function test001_ri_7001() {
  module_enable(array('ri'));
  drush_log('Enabled module *ri*', 'status');

  return 'Test RI Number One was Completed';
}

/**
 * @return string
 */
function test001_ri_7002() {
  features_revert_module('feature_name');
  drush_log('Reverted feature *feature_name*', 'status');

  return 'Test RI Number Two was Completed';
}