<?php
/**
 * Database helper class
 *
 * Manages Memcached resources
 *
 * @author     R. Brenton Strickler <rbrenton@gmail.com>
 * @link       http://www.framewerk.org/
 * @license    BSD
 * @copyright  Copyright 2013 Framewerk Development Group
 * @package    Toolkit
 */

class fMD extends fSingletonObject
{
  private $md = null;

  private static $instance;

  function __construct()
  {
    parent::__construct();
  }

  function __destruct()
  {
  }

  /**
   * Get or create fMD instance
   * 
   * @return fMD
   */
  public static function getInstance()
  {
    if ( self::$instance === NULL )
    {
      self::$instance = new fMD();
    }
    return self::$instance;
  }

  public static function connect()
  {
    return self::getInstance()->pool();
  }

  public function pool()
  {
    if($this->md!==null) return $this->md;

    // Initialize memcached.
    $this->md = new Memcached();
    $this->md->setOption(Memcached::OPT_CLIENT_MODE, Memcached::DYNAMIC_CLIENT_MODE);

    // Sanity check for persistent connection pool.
    //FIXME: compare to settings and reset if mismatch
    if(count($this->md->getServerList())>0)
      return $this->md;

    // Set connect timeout.
    $this->md->setOption(Memcached::OPT_CONNECT_TIMEOUT, (int) ELASTICACHE_TIMEOUT);

    // Add servers.
    $port = (int) ELASTICACHE_PORT ?: 11211;
    $this->md->addServer((string) ELASTICACHE_HOST, $port);

    return $this->md;
  }

  public function getdb($conn, $singleRow, $singleColumn, $key, $sql, $timeout=1800)
  {
    $md = $this->pool(); 
    $data = $md->get($key);
    if(!$data) {
      $select = mysql_query($sql, $conn);
      if(!$select) {
        return null;
      }
      $count = mysql_num_rows($select);
      if($count == 0) {
        $value = null;
      } else if($singleRow) {
        $row = mysql_fetch_assoc($select);
        $cols = array_keys($row);
        $value = $singleColumn ? $row[$cols[0]] : $row;
      } else {
        $value = array();
        while($row = mysql_fetch_assoc($select)) {
          $cols = array_keys($row);
          $value[] = $singleColumn ? $row[$cols[0]] : $row;
        }
      }
      $data = array('value'=>$value);
      $md->set($key, $data);
    }
    return $data['value'];
  }

  public function setdb($conn, $key, $value, $sql, $timeout=1800)
  {
    $md = $this->pool();
    $md->set($key, array('value'=>$value), $timeout);
    $update = mysql_query($sql, $conn);
    return $update;
  }

}
