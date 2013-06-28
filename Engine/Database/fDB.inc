<?php // $Id$
/**
 * DOMDocument helper class
 *
 * Defines functions for generating DOM Documents
 *
 * @author     R. Brenton Strickler <rbrenton@gmail.com>
 * @link       http://www.framewerk.org/
 * @license    BSD
 * @copyright  Copyright 2011 Framewerk Development Group
 * @package    Toolkit
 * @subpackage PDO
 */

class fDB extends fSingletonObject
{
  private $registry;
  private $resources=array();

  private static $instance;

  function __construct( )
  {
    parent::__construct();
    $this->registry = new fRegistry($this);
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

  public function connect($server)
  {
    if($this->resources[$server])
      return $this->resources[$server];

    if(!$this->registry->{$server})
      throw new Exception("Server {$server} not found in registry.");

    $entry = &$this->registry->{$server};

    if(!($this->resources[$server] = @pg_connect((string) $entry->dsn)))
      throw new Exception('Database connection error.');
      //throw new Exception(pg_last_error());//tmi

    return $this->resources[$server];
  }

  public function resource($server)
  {
    return $this->resources[$server];
  }

}
