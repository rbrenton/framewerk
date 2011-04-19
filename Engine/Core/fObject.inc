<?php
/**
 * fObject
 *
 * A default object implementation
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @author     R. Brenton Strickler <rbrenton@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2004-2011 the Framewerk Development Group
 * @package    Engine
 * @subpackage Core
 * @abstract
 */

abstract class fObject
{
  protected $fMain;
  private $entity = array();
  public  $validEntities = array();

  /**
   * Constructor, get parent reference
   */
  function __construct()
  {
    $this->fMain = fMain::getInstance();
  }

  function getDataValues()
  {
    return $this->entity;
  }

  /**
   * PHP Object property overload
   *
   * @param String Keyname ($Object->Keyname)
   * @param String Value
   */
  function __set($key, $value)
  {
    if ( array_search($key, $this->validEntities) === false )
      throw new Exception('Attempt to set an invalid ' . get_class($this) . ' entity.', 0);
    $this->entity[$key] = $value;

    // Hide passwords
    if ( $key == "password" ) $value = "*** masked ***";
  }

  /**
   * PHP Object property overload
   *
   * @param String Keyname ($Object->Keyname)
   * @return String Value
   */
  function __get($key)
  {
    if ( !isset($this->entity[$key]) && array_search($key, $this->validEntities) === false )
    throw new Exception(get_class($this) . ": Attempt to reference an invalid " . get_class($this) . " entity \"$key\"", 0);

    return $this->entity[$key];
  }

  /**
   * PHP Object method overload.  This is not supported.  Return a warning if someone tries to use it.
   *
   * @param String function name
   * @param Array arguments
   */
  function __call($func, $args)
  {
    $args;
    throw new Exception("Undefined function called:  " . get_class($this) . "::" . $func . "()");
  }

  /**
   * Add a key to the internal array that specifies it's a valid entity
   *
   * @param String Key
   */
  protected function validEntity($key)
  {
    if ( array_search($key, $this->validEntities) !== false ) {
      trigger_error(get_class($this) . ": Attempting to add a duplicate valid entity '$key'.", E_USER_WARNING);
      return;
    }
    $this->entity[$key] = null;
    $this->validEntities[] = $key;
  }


  /**
   * Validate that the key being asked for is a valid entity set with ValidEntity()
   *
   * @param String Key
   * @return bool is Valid
   */
  protected function isEntitySet($key)
  {
    if ( array_search($key, $this->validEntities) === false )
    {
      return false;
    }
    return (isset($this->entity[$key]) && $this->entity[$key] !== null);
  }

}
