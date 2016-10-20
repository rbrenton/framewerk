<?php
// Set PHP error reporting level.
error_reporting(E_ERROR | E_WARNING | E_PARSE | E_CORE_ERROR | E_COMPILE_ERROR | E_USER_ERROR);

// Pull defines from yaml settings file.
$yaml = yaml_parse_file(__DIR__.'/settings.yaml');
function make_defines($array, $prefix=null) {
  static $dirs = array();

  foreach ($array as $key=>$value) {
    $key = strtoupper(ltrim("{$prefix}_{$key}", '_'));
    if (is_array($value)) {
      make_defines($value, $key);
      continue;
    }
    if (!defined($key))
      define($key, $value);

    // Track _DIR entries.
    if (preg_match(';^(.+)_DIR$;', $key, $regs))
      $dirs[] = $regs[1];
  } 

  // Create _ROOT entries relative to DEPLOY_ROOT for all _DIR entries.
  if ($prefix === null && defined('DEPLOY_ROOT')) {
    foreach ($dirs as $prefix) {
      $dirKey = "{$prefix}_DIR";
      if (!defined($dirKey))
        continue;
      $rootKey = "{$prefix}_ROOT";
      if (defined($rootKey))
        continue;
      $dir = constant($dirKey);
      if (!preg_match(';^/;', $dir))
        $dir = constant('DEPLOY_ROOT')."/{$dir}";
      define($rootKey, $dir);
    }
  }
}
make_defines($yaml);


// Set default timezone.
date_default_timezone_set(SYSTEM_TIMEZONE);


// Return the current time as a float
if (!function_exists('microtime_float')) {
function microtime_float() {
  return microtime(true);
}
}

// Calculate time lapse
if (!function_exists('lapse_float')) {
function lapse_float($start=null) {
  $start = (float) $start ?: $_SERVER['REQUEST_TIME_FLOAT'];
  return (microtime(true) - $start);
}
}

// Ensure $_SERVER['REQUEST_TIME_FLOAT'] is set.
if (is_array($_SERVER)) {
  if (!isset($_SERVER['REQUEST_TIME_FLOAT']) || !$_SERVER['REQUEST_TIME_FLOAT'])
    $_SERVER['REQUEST_TIME_FLOAT'] = microtime(true);
}

// Map proper client/browser ip to $_SERVER['USER_ADDR']
if (is_array($_SERVER) && !isset($_SERVER['USER_ADDR'])) {
  foreach (array('HTTP_X_FORWARDED_FOR','REMOTE_ADDR') as $key) {
    if (!isset($_SERVER[$key])) continue;
    if ($_SERVER[$key]=='') continue;
    $_SERVER['USER_ADDR'] = $_SERVER[$key];
    break;
  }
}

// Prevents the New Relic output filter from attempting to insert RUM JavaScript.
if (extension_loaded('newrelic')) {
  newrelic_disable_autorum();

  // add any calls to newrelic_add_custom_tracer() here

  if (is_array($_SERVER) && isset($_SERVER['HTTP_HOST'], $_SERVER['REQUEST_URI'])) {
    // Web requests
    if (!preg_match(';^(.*\.|)[^.]+\.[^.]{2,}$;i', $_SERVER['HTTP_HOST'])) {
      newrelic_ignore_transaction();
    } else {
      newrelic_set_appname($_SERVER['HTTP_HOST']);
      newrelic_capture_params();
      $baseURI = preg_replace(';[?&#].*;', '', $_SERVER['REQUEST_URI']);
      if ($baseURI!='/' && $baseURI!='')
        newrelic_name_transaction($baseURI);
    }
  } else {
    // Command line scripts
    newrelic_background_job(true);
  }
}

if (defined('ROLLBAR_ACCESS_TOKEN') && defined('DEPLOY_ROOT')) {
  @include_once('scripts/rollbar/rollbar.php');
  if (class_exists('Rollbar')) {
    $config = array(
      'access_token' => constant('ROLLBAR_ACCESS_TOKEN'),
    );

    if (defined('ROLLBAR_HANDLER')) {
      $config['handler'] = constant('ROLLBAR_HANDLER');
    }

    if (defined('ROLLBAR_AGENT_LOG_LOCATION')) {
      $logDir = constant('ROLLBAR_AGENT_LOG_LOCATION');
      $logDir = (($logDir[0] != '/') ? constant('DEPLOY_ROOT').'/' : '') . $logDir;
      $config['agent_log_location'] = $logDir;
    }

    Rollbar::init($config);
  }
}
