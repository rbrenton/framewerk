<?php
/**
 * Framewerk Non-Caching Controlling Program
 *
 * This program is responsible for the high level operation of the fMain object. All requests are funneled through this script.
 *
 * @author     R. Brenton Strickler <rbrenton@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2004-2011 the Framewerk Development Group
 * @since      2004
 */

/**
 * Define PHP class filesystem map array.
 */
$_classFiles = null;

/**
 * Build an array of all files
 *
 * @param string Directory
 */
function mapClassFiles($directory) {
  global $_classFiles;
  static $ignore = array('Engine/System');

  if(in_array($directory, $ignore))
    return;

  $dir = opendir($directory);
  while(($entry = readdir($dir))) {
    if($entry[0]=='.') continue; //ignore ., .., .svn, .git, etc
    $file = "{$directory}/{$entry}";
    if(is_dir($file))
      mapClassFiles($file);
    else if(is_file($file) && preg_match(';(^|/)([^/]+)[.]inc$;', $file, $regs))
      $_classFiles[$regs[2]] = $file;
  }
}

/**
 * Using PHP5's Object Autoloader we will load the files when they need to be instanced
 *
 * @param Object $classname
 */
function __autoload($classname) {
  global $_classFiles;

  if($_classFiles===null) {
    $_classFiles = array();
    mapClassFiles(realpath('.'));
  }

  // Check that the file is mapped
  if(isset($_classFiles[$classname]))
    // Since the user should never be requiring a file, no need for require_once
    require($_classFiles[$classname]);
}

require_once('Engine/System/debug.inc');

// Create our instance of the debug and Framewerk object
$fMain = fMain::getInstance();
$fMain->process();
