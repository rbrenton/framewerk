<?php
/**
 * CLI Common Library
 *
 * This script will self-destruct in 5 seconds.
 *
 * @author rbrenton@gmail.com
 *
 *
 * Example Usage:

// Optional Defines
define('PARTNER_ID', 100);
define('PROCESS_LOCKING_FILE', __FILE__.'.'.PARTNER_ID.'.pid');

// Suggested Defines
define('DEBUG', true);
define('DEBUG_SQL', false);

// Include CLI config and functions.
require_once('/var/www/www.example.org/Engine/Utils/CLI.inc');

 *
 *
 */

// Include relative to this file.
require_once(__DIR__."/../../Config/settings.inc");

if(!isset($_SERVER))
  $_SERVER = array();

if($_SERVER['DOCUMENT_ROOT'] == '')
  $_SERVER['DOCUMENT_ROOT'] = FRAMEWERK_SYSTEM_DOCROOT;

if(!function_exists('debug')) {
function debug($message, $isSQL=false) {
  if(!defined('DEBUG') || !DEBUG) return;
  if((!defined('DEBUG_SQL') || !DEBUG_SQL) && $isSQL) return;

  echo trim($message)."\n";
  flush();
}
}

function mysqlConnect($host=null) {
  static $conns = array();

  // Default to master db
  if($host === null)
    $host = MYSQL_MASTER;

  // If already connected, check it and then return that connection
  if(isset($conns[$host])) {
    if(@mysql_ping($conns[$host])) {
      return $conns[$host];
    }
    debug("MySQL server went away: {$host}");
  }

  debug("Connecting to MySQL server: {$host}");
  // Make the connection
  if(!($conn = @mysql_connect($host, MYSQL_USERNAME, MYSQL_PASSWORD)))
    throw new Exception(mysql_error()."\n");

  // Only allow it to select primary db for now
  debug('Selecting database: '.MYSQL_DATABASE);
  if(!mysql_select_db(MYSQL_DATABASE, $conn))
    throw new Exception(mysql_error()."\n");

  // Save the connection
  $conns[$host] = $conn;

  return $conn;
}

if(!function_exists('mysql_query_debug')) {
function mysql_query_debug($sql, $line, $conn=null) {
  // Output SQL first if DEBUG_SQL is true
  if(defined('DEBUG_SQL') && DEBUG_SQL)
    debug("[{$line}] SQL: $sql");

  // Perform query and return if no error
  if($conn===null)
    $query = mysql_query($sql);
  else
    $query = mysql_query($sql, $conn);
  if($query)
    return $query;

  // Output SQL if it wasn't already printed
  if(!(defined('DEBUG_SQL') && DEBUG_SQL))
    debug("[{$line}] SQL: $sql");

  // Output MySQL error
  debug("[{$line}] Error: ".mysql_error());

  return $query;
}
}

function mysqlIConnect($host=null, $id=null) {
  static $conns = array();

  // Default to master db
  if($host === null)
    $host = MYSQL_MASTER;

  // If already connected, check it and then return that connection
  if(isset($conns["$host:$id"])) {
    if(@mysqli_query($conns["$host:$id"], "SELECT 1;"))
      return $conns["$host:$id"];
  }

  debug('Connecting to MySQL server: '.$host);
  // Make the connection
  if(!($conn = mysqli_connect($host, MYSQL_USERNAME, MYSQL_PASSWORD, MYSQL_DATABASE)))
    throw new Exception(mysqli_connect_error()."\n");

  // Save the connection
  $conns["$host:$id"] = $conn;

  return $conn;
}

if(!function_exists('mysqli_query_debug')) {
function mysqli_query_debug($conn, $sql, $line) {
  // Output SQL first if DEBUG_SQL is true
  if(defined('DEBUG_SQL') && DEBUG_SQL)
    debug("[{$line}] SQL: $sql");

  // Perform query and return if no error
  $query = mysqli_query($conn, $sql);
  if($query)
    return $query;

  // Output SQL if it wasn't already printed
  if(!(defined('DEBUG_SQL') && DEBUG_SQL))
    debug("[{$line}] SQL: $sql");

  // Output MySQL error
  debug("[{$line}] Error: ".mysqli_error($conn));

  return $query;
}
}

function _filenameProcessLock($file=null, $key=null) {
  // Check for blank filename.
  if($file == '') {
    if(defined('PROCESS_LOCKING_FILE') && PROCESS_LOCKING_FILE != '') {
      $file = PROCESS_LOCKING_FILE;
    } else {
      $file = 'cli_process';
      if($key == '' && defined('PARTNER_ID') && PARTNER_ID > 0) {
        $file .= '.'.PARTNER_ID;
      }
    }
  }

  if($key != '') {
    $file .= '.'.$key;
  }

  // Ensure .pid extension
  if(!preg_match(';[.]pid$;', $file))
    $file .= '.pid';

  return $file;
}

function startProcessLock($processLockingFile=null, $processLockingKey=null) {
  debug('Attaining process lock.');
  $processLockingFile = _filenameProcessLock($processLockingFile, $processLockingKey);

  debug("Checking PID file: {$processLockingFile}");
  $e = error_reporting();
  error_reporting(E_ERROR | E_WARNING | E_PARSE);
  if(!($f = @fopen($processLockingFile, 'x'))) { //open fails if file exists
    // Grab PID from file.
    if(!($pid = trim(file_get_contents($processLockingFile))))
      die("Cannot open for reading, or contains zero value: {$processLockingFile}\n");
    $pidUser = getpiduser($pid);
    if($pidUser=='')
      unlink($processLockingFile);
    else {
      // PID file already exists, and program with PID is running.
      debug("PID running: {$pid}");
      die();
    }
    if(!($f = @fopen($processLockingFile, 'x')))
      die("Cannot open file for writing: {$processLockingFile}\n");
  }
  error_reporting($e);
  fwrite($f, getmypid());
  fclose($f);
}

function endProcessLock($processLockingFile=null, $processLockingKey=null) {
  debug('Releasing process lock.');
  $processLockingFile = _filenameProcessLock($processLockingFile, $processLockingKey);

  // Remove PID file, if it's ours.
  if(getmypid() == trim(file_get_contents($processLockingFile)))
    unlink($processLockingFile);
}

function getpiduser($pid) {
  $e = error_reporting();
  error_reporting(E_ERROR | E_WARNING | E_PARSE);
  $pid = (int) $pid;
  $ps=shell_exec("ps up {$pid}");
  $ps=explode("\n", $ps);
  if(count($ps)<2) {
    error_reporting($e);
    return false;//couldn't find pid
  }
  foreach($ps as $key=>$val)
    $ps[$key]=explode(' ', preg_replace(';[[:space:]]+;', ' ', trim($ps[$key])));
  foreach($ps[0] as $key=>$val){
    $pidinfo[$val] = $ps[1][$key];
    unset($ps[1][$key]);
  }
  if(is_array($ps[1]))
    $pidinfo[$val].=' '.implode(' ', $ps[1]);
  error_reporting($e);
  if(preg_match('; <defunct>$;', $pidinfo['COMMAND']))
    $pidinfo['DEFUNCT'] = true;
  return $pidinfo['DEFUNCT'] ? '' : $pidinfo['USER'];
}

if(!function_exists('array2csv')) {
function array2csv($array) {
  $row='';
  $i=0;
  foreach($array as $key=>$value)
  {
    if($i++>0) $row .= ',';
    if(preg_match('/[,"]/m', $value))
    {
      $value = preg_replace('/["]/m', '""', $value);
      $value = '"'.$value.'"';
    }
    $row .= $value;

  }
  return "$row\n";
}
}

function mysql_query2csv($sql, $conn=null) {
  $select = mysql_query_debug($sql, __LINE__, $conn);
  if(!$select) return;

  $header=false;
  while($row = mysql_fetch_assoc($select)) {
    if(!$header) {
      echo array2csv(array_keys($row));
      $header = true;
    }     
    echo array2csv($row);
  }     
}
