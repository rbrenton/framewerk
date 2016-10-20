<?php
/**
 * Database helper class
 *
 * Manages MySQL resources
 *
 * @author     R. Brenton Strickler <rbrenton@gmail.com>
 * @link       http://www.framewerk.org/
 * @license    BSD
 * @copyright  Copyright 2011 Framewerk Development Group
 * @package    Toolkit
 */

class fDB extends fSingletonObject
{
  private $resources = array();

  private static $instance;

  function __construct()
  {
    parent::__construct();
  }

  function __destruct()
  {
    foreach($this->resources[$server] as $conn) {
      mysql_close($conn);
    }
  }

  /**
   * Get or create fDB instance
   * 
   * @return fDB
   */
  public static function getInstance( )
  {
    if ( self::$instance === NULL )
    {
      self::$instance = new fDB();
    }
    return self::$instance;
  }

  public static function conn($server)
  {
    return self::getInstance()->connect($server);
  }

  public function connect($server)
  {
    if($this->resources[$server])
      return $this->resources[$server];

    $server = strtoupper("MYSQL_{$server}");
    if(!defined($server))
      throw new Exception("Server {$server} not found in settings.");

    if(!($this->resources[$server] = @mysql_pconnect(constant($server), MYSQL_USER, MYSQL_PASS)))
      throw new Exception('Database connection error.');

    if(!mysql_select_db(MYSQL_DB, $this->resources[$server]))
      throw new Exception('Database selection error. '.MYSQL_DB);

    return $this->resources[$server];
  }

  public function resource($server)
  {
    return $this->resources[$server];
  }

}
