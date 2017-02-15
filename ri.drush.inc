<?php

/**
 * Implements hook_drush_command().
 */
function ri_drush_command() {
  $items['ri'] = array(
    'description' => 'Executes the release instructions.',
    'aliases' => array(),
  );

  $items['ri-preview'] = array(
    'description' => 'Previews scheduled release instructions.',
    'aliases' => array('rip'),
  );

  $items['ri-status'] = array(
    'description' => 'Sets status for the release instruction function.',
    'aliases' => array('ris'),
    'arguments' => array(
      'function' => 'Function name.',
      'flag' => 'Flag (1 or 0).',
    ),
  );

  $items['ri-function'] = array(
    'description' => 'Runs a single release instruction.',
    'aliases' => array('rif'),
    'arguments' => array(
      'function' => 'Function name.',
    ),
  );

  return $items;
}

/**
 * Callback for "ri" command
 */
function drush_ri() {
  $updates = ri_updates(FALSE);

  // Now run the updates.
  foreach ($updates as $module => $functions) {
    foreach ($functions as $function => $version) {
      // Skip if already executed.
      if (ri_executed_get($function)) {
        continue;
      }

      ri_execute_function($function);
    }
  }

  drush_log('Release instructions were executed.', 'ok');
}

/**
 * Callback for "ri-preview" command
 */
function drush_ri_preview() {
  $updates = ri_updates(TRUE);

  foreach ($updates as $module => $functions) {
    foreach ($functions as $function => $version) {
      $message = $function;

      // Message.
      drush_log($message, 'status');
    }
  }
}

/**
 * Callback for "ri-status" command
 */
function drush_ri_status($function, $flag) {
  ri_executed_set($function, $flag);
  drush_log(sprintf('Status for %s() was set to %s.', $function, $flag ? 1 : 0), 'ok');
}

/**
 * Callback for "ri-function" command
 */
function drush_ri_function($function) {
  // Preload the files.
  $updates = ri_updates(FALSE);

  // Function.
  if (FALSE === strpos($function, '*')) {
    ri_execute_function($function);
  }

  // Wildcard.
  else {
    $pattern = '@^' . str_replace('*', '.*', $function) . '$@';

    // Now run the updates.
    foreach ($updates as $module => $functions) {
      foreach ($functions as $function => $version) {
        if (preg_match($pattern, $function)) {
          ri_execute_function($function);
        }
      }
    }
  }
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
  $ri_executed[$function] = $flag ? 1 : 0;
  variable_set('ri_executed', $ri_executed);
}

/**
 * @param $function
 */
function ri_execute_function($function) {
  // Delimiter.
  $delimiter = '##############################';
  echo $delimiter, "\n";
  echo $function, '()', "\n";

  // Execute.
  if (function_exists($function)) {
    if (!($message = $function())) {
      $message = 'Release instructions ' . $function . '() were executed.';
    }

    // Mark as executed.
    ri_executed_set($function);
  }
  else {
    $message = 'Release instruction ' . $function . '() does not exist.';
  }

  // Message.
  drush_log($message, 'status');

  // Delimiter.
  echo $delimiter, "\n\n";
}

/**
 * Finds all the Release Instriction files.
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
 * @param bool $exclude_executed
 *
 * @return array
 */
function ri_updates($exclude_executed = TRUE) {
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
        ksort($module_updates);
      }
    }
  }

  // Exclude.
  if ($exclude_executed) {
    foreach ($updates as $module => $functions) {
      foreach ($functions as $function => $version) {
        if (ri_executed_get($function)) {
          unset($updates[$module][$function]);
        }
      }
    }
  }

  return $updates;
}
