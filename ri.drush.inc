<?php

/**
 * Implements hook_drush_command().
 */
function ri_drush_command() {
  $items['ri'] = array(
    'description' => 'Executes the release instructions.',
    'aliases' => array(),
  );

  return $items;
}

/**
 * Callback for "ri" command
 */
function drush_ri() {
  $updates = array();

  if ($ri_files = ri_files()) {
    foreach ($ri_files as $ri_file) {
      // Load file.
      module_load_include('inc', $ri_file['module'], 'ri/' . $ri_file['name']);

      // Get function names prefix.
      $separator = '_';
      $function_name = trim(preg_replace('@[^a-z0-9_]+@', $separator, drupal_strtolower($ri_file['name'])), $separator);

      // Prepare regular expression to match all possible defined hook_update_N().
      $regexp = '/^' . $function_name . '_(?P<version>\d+)$/';
      $functions = get_defined_functions();
      // Narrow this down to functions ending with an integer, since all
      // hook_update_N() functions end this way, and there are other
      // possible functions which match '_update_'. We use preg_grep() here
      // instead of foreaching through all defined functions, since the loop
      // through all PHP functions can take significant page execution time
      // and this function is called on every administrative page via
      // system_requirements().
      foreach (preg_grep('/_\d+$/', $functions['user']) as $function) {
        // If this function is a module update function, add it to the list of
        // module updates.
        if (preg_match($regexp, $function, $matches)) {
          $updates[$ri_file['module']][$function] = $matches['version'];
        }
      }
      // Ensure that updates are applied in numerical order.
      foreach ($updates as &$module_updates) {
        asort($module_updates, SORT_NUMERIC);
      }
    }
  }

  // Now run the updates.
  foreach ($updates as $module => $functions) {
    foreach ($functions as $function => $version) {
      // Skip if already executed.
      if (ri_executed_get($function)) {
        continue;
      }

      // Execute.
      if (!($message = $function())) {
        $message = 'Release instructions ' . $function . '() were executed.';
      }

      // Mark as executed.
      ri_executed_set($function);

      // Message.
      drush_log($message, 'status');
    }
  }

  drush_log('Release instructions were executed.', 'ok');
}

/**
 * @param null $function
 *
 * @return bool|null
 */
function ri_executed_get($function = NULL) {
  $ri_executed = variable_get('ri_executed', array());
  return is_null($function) ? $ri_executed : !empty($ri_executed[$function]);
}

/**
 * @param $function
 * @param bool $flag
 */
function ri_executed_set($function, $flag = TRUE) {
  $ri_executed = ri_executed_get();
  $ri_executed[$function] = $flag;
  variable_set('ri_executed', $ri_executed);
}

/**
 * Finds all the Release Instriction files.
 *
 * @return array
 */
function ri_files() {
  $ri_files = array();

  $hook = 'ri';
  foreach (module_implements($hook) as $module) {
    $directory = DRUPAL_ROOT . '/' . drupal_get_path('module', $module) . '/ri';
    if (is_dir($directory)) {
      if ($files = file_scan_directory($directory, '/.*\.ri\.inc$/')) {
        foreach ($files as $file) {
          $ri_files[] = array(
            'module' => $module,
            'name' => $file->name,
          );
        }
      }
    }
  }

  return $ri_files;
}

/**
 * Implements hook_ri().
 */
function ri_ri() {
  return TRUE;
}
