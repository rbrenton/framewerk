#!/usr/bin/php
<?php
$now = time();

// Must run as root.
$uid = posix_getuid();
if ($uid != 0)
  throw new Exception('Invalid user id.');

define('PROCESS_LOCKING_FILE', __FILE__.".pid");
define('DEBUG', false);
define('DEBUG_SQL', false);
require_once(__DIR__.'/Engine/Utils/CLI.inc');

$cwd = preg_replace(';/[^/]*$;', '/', __FILE__);

// Set environment variables.
$env = array(
  // Set Jenkins endpoint.
  'JENKINS_HOME'=>'http://EXAMPLE_USER:EXAMPLE_PASS@EXAMPLE_HOST:8080/',
);

// Silently open database connection.
ob_start();
$master = mysqlConnect(MYSQL_MASTER);
if ($master) ob_end_clean();
else ob_end_flush();

// Fetch cron jobs.
$ami = exec('curl http://169.254.169.254/latest/meta-data/ami-id 2> /dev/null');
$amiSQL = mysql_real_escape_string($ami);
$hostname = php_uname("n");
$hostnameSQL = mysql_real_escape_string($hostname);
$sql = <<<SQL
SELECT
  `id`,
  `user`,
  (select pid from cron_status where cron_id=c.id and '{$hostnameSQL}'=hostname limit 1) AS pid,
  `m`,
  `h`,
  `dom`,
  `mon`,
  `dow`,
  `jenkins_job`,
  `workdir`,
  `command`,
  `do_concurrent`,
  `do_sequential`
FROM cron c
WHERE
  (hostname IS NULL OR '{$hostnameSQL}' LIKE hostname) AND
  (ami IS NULL OR '{$amiSQL}' LIKE ami) AND
  suspended=0;

SQL;
$select = mysql_query_debug($sql, __LINE__);
if (!$select) {
  throw new Exception(mysql_error($master));
}

// Find java binary.
$java = exec('which java 2> /dev/null');

function cronMatch($expr, $cur) {
  // Handles:
  //  *
  //  30
  //  */15
  //  0,15,30,45
  //  1-6

  $parts = explode(',', $expr);
  foreach ($parts as $part) {
    if ($part == '*') return true;
    if ($part === $cur) return true;
    if ($part == $cur && is_numeric($part) && is_numeric($cur)) return true;
    if (preg_match(';^[*]/([0-9]+)$;', $part, $regs)) {
      $div = $regs[1];
      if ($cur % $div == 0) return true;
    }
    if (preg_match(';^([0-9]+)-([0-9]+)$;', $part, $regs)) {
      $low = $regs[1];
      $high = $regs[2];
      if ($cur >= $low && $cur <= $high) return true;
    }
  }
  return false;
}

function jobEndCheck($cronId, $hostname, $pid, $runAs=null) {
  $cronId = (int) $cronId;
  $hostnameSQL = mysql_real_escape_string($hostname);
  $pid = (int) $pid;
  if ($pid<=0) return null;

  // Check if process is still running
  $user = getpiduser($pid);
  if ($user != '' && $user == $runAs)
    return false;

  debug("Storing end time for cron id {$cronId}, pid {$pid}.");
  $master = mysqlConnect(MYSQL_MASTER);
  $result = mysql_query_debug("UPDATE cron_status SET pid=NULL, time_end=NOW() WHERE cron_id={$cronId} AND hostname='{$hostnameSQL}' AND pid='{$pid}';", __LINE__, $master);

  return true;
}

function jobStart($cronId, $hostname, $pid) {
  $cronId = (int) $cronId;
  $hostnameSQL = mysql_real_escape_string($hostname);
  $pid = (int) $pid;
  if ($pid<=0) return null;

  debug("Storing start time for cron id {$cronId}, pid {$pid}.");

  // Update start time and pid in database.
  $master = mysqlConnect(MYSQL_MASTER);

  $select = mysql_query_debug("SELECT id FROM cron_status WHERE cron_id='{$cronId}' AND hostname='{$hostnameSQL}';", __LINE__, $master);
  $row = mysql_fetch_assoc($select);
  $statusId = (int) $row['id'];

  if (!$statusId) {
    $upsert = mysql_query_debug("INSERT INTO cron_status (cron_id, hostname, pid) VALUES ('{$cronId}', '{$hostnameSQL}', '{$pid}') ON DUPLICATE KEY UPDATE time_start=NOW(), time_end=NULL, pid='{$pid}';", __LINE__, $master);
    $statusId = $result ? mysql_insert_id() : null;
  } else {
    $update = mysql_query_debug("UPDATE cron_status SET time_start=NOW(), time_end=NULL, pid='{$pid}' WHERE id='{$statusId}';", __LINE__, $master);
  }

  return $statusId;
}


// Iterate cron jobs.
$children = array();//track child pids
while ($row = mysql_fetch_assoc($select)) {
  $cronId = $row['id'];
  $pid = $row['pid'];
  debug("Checking cron id {$cronId}.");

  // End previous finished process.
  if ($row['pid']>0 && jobEndCheck($cronId, $hostname, $row['pid'], $row['user'])) {
    $pid = null;
  }

  // Check m (minute)
  //debug("Checking m.");
  if (!cronMatch($row['m'], strftime('%M', $now)))//00 to 59
    continue;

  // Check h (hour)
  //debug("Checking h.");
  if (!cronMatch($row['h'], strftime('%H', $now)))//00 to 23
    continue;

  // Check dom (day of month)
  //debug("Checking dom.");
  if (!cronMatch($row['dom'], strftime('%d', $now)))//01 to 31
    continue;

  // Check mon (month)
  //debug("Checking mon.");
  if (!cronMatch($row['mon'], strftime('%m', $now)))//01 to 12
    continue;

  // Check dow (day of week)
  //debug("Checking dow.");
  if (!cronMatch($row['dow'], strftime('%w', $now))//1 to 7(sun)
  && !cronMatch($row['dow'], strftime('%u', $now))//0(sun) to 6
  && !cronMatch($row['dow'], strftime('%a', $now)))//Sun to Sat
    continue;

  debug("Passed time checks.");

  // Check pid and do_concurrent.
  debug("Checking pid and do_concurrent.");
  if ($pid && !$row['do_concurrent'])
    if ($row['user'] == getpiduser($pid))
      continue;

  // Begin child process.
  debug("Forking child.");
  $child = pcntl_fork();
  if ($child > 0) {
    // Update start time and pid in database.
    jobStart($cronId, $hostname, $child);
    $children[$cronId] = $child;
  } else {
    try {
      // Set working directory.
      $workdir = rtrim($row['workdir'],'/').'/';
      if ($workdir[0]!='/') $workdir = "{$cwd}{$workdir}";

      // Set command.
      $command = $workdir.$row['command'];

      // Set up job.
      if ($java && $row['jenkins_job']) {
        $args = array('-Xmx1G', '-jar', exec('ls /usr/share/jenkins/external-job-monitor/java/jenkins-core-*.jar'), $row['jenkins_job'], $command);
        $command = $java;
      } else {
        $args = array();
      }

      // Switch to working directory.
      if (!is_dir($workdir))
        throw new Exception("No such directory: {$workdir}");
      if (!chdir($workdir))
        throw new Exception("Unable to chdir: {$workdir}");

      // Set process uid.
      if ($row['user'] !== null && $row['user'] !== 'root') {
        $uid = exec('id -u '.escapeshellarg($row['user']).' 2> /dev/null');
        if (!posix_setuid($uid))
          throw new Exception("Unable to set uid: {$uid}");
      }

      // Execute command.
      if (!file_exists($command)) {
        throw new Exception("File doesn't exist: {$command}");
      }
      pcntl_exec($command, $args, $env);

      // Code after pcntl_exec never executes unless there is an error. this makes setting time_end difficult.

    } catch (Exception $e) {
      echo "CronId: {$cronId} PID:{$pid} Error: ".$e->getMessage()."\n";
    }

    // Update end time and pid in database.
    jobEndCheck($cronId, $hostname, posix_getpid());
    unset($children[$cronId]); // doesn't actually help since we're forked

    // End child process.
    exit(0);
  }
}

// Wait up to 60 seconds for children to finish.
$i=0;
while (($c=count($children))>0) {
  echo "Waiting on $c children...\n";
  sleep(1);
  if ($i++>=60) break;
  $tmp = $children;
  foreach ($tmp as $cronId=>$pid) {
    echo "Process $pid";
    if (jobEndCheck($cronId, $hostname, $pid)) {
      echo " ended\n";
      unset($children[$cronId]);
    } else {
      echo " still running\n";
    }
  }
}
