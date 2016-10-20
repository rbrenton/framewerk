#!/usr/bin/php
<?php
require_once('/var/www/www.example.org/Engine/Utils/CLI.inc');

if($argc < 2 || $argc > 3) {
  echo "Usage: {$argv[0]} <partner_id> [date]\n";
  die();
}
$partnerId = (int) $argv[1];
$date = strftime('%Y-%m-%d', strtotime($argv[2] ?: 'now'));
echo "User agents for $partnerId on $date\n";

mysqlConnect(MYSQL_SLAVE);
$sql = "SELECT lsm.user_agent AS user_agent FROM log_sessions ls LEFT JOIN log_session_meta lsm ON (lsm.log_session_id=ls.id) WHERE ls.time_firstaction >= '{$date}' AND ls.time_firstaction < TIMESTAMPADD(DAY, 1, '{$date}') AND ls.partner_id={$partnerId};";
$select = mysql_query($sql);

$uas = array();
while($row = mysql_fetch_assoc($select)) {
  $uas[$row['user_agent']]++;
}

$browsers = array();
$parser = new \UAS\Parser();
$parser->SetCacheDir(sys_get_temp_dir() . "/uascache/");
foreach($uas as $ua=>$count) {
  $browser = 'unknown';
  if($ret = $parser->parse($ua)) {
    $browser = "{$ret['typ']} - {$ret['ua_family']}";
  }
  $browsers[$browser] += $count;
}

ksort($browsers);
$total = $max1 = $max2 = 0;
foreach($browsers as $browser=>$count) {
  $max1 = max($max1, strlen($browser));
  $max2 = max($max2, strlen($count));
  $total += $count;
}
$max2 = max($max2, strlen($total));

foreach($browsers as $browser=>$count) {
  $pct = number_format($count / $total * 100.0, 2, '.', '');
  echo str_pad($browser, $max1, ' ', STR_PAD_RIGHT);
  echo "  ";
  echo str_pad($count, $max2, ' ', STR_PAD_LEFT);
  echo " ";
  echo str_pad($pct, 6, ' ', STR_PAD_LEFT);
  echo "%\n";
}
{
  echo str_pad('', $max1, ' ', STR_PAD_RIGHT);
  echo "  ";
  echo str_pad($total, $max2, ' ', STR_PAD_LEFT);
  echo " ";
  echo str_pad('', 6, ' ', STR_PAD_LEFT);
  echo " \n";
}
