<?php
/**
 * fSingletonObject
 *
 * A default object implementation
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://www.framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2004-2011 the Framewerk Development Group
 */

abstract class fSingletonObject
{
  /**
   * @var fMain
   */
  protected $fMain;

  private $entity = array();
  protected $validEntities = array();

  /**
   * @param Object fMain parent
   */
  protected function __construct()
  {
    $this->fMain    = fMain::getInstance();
  }

  /**
   * @abstract
   * @static
   * @return mixed
   */
  abstract public static function getInstance();

  /**
   * @todo We added this when we were having destruction sequence errors that I think have otherwise been resolved
   * I'd like to see if we can remove this entirely
   * abstract static function destroy();
   */

  /**
   * PHP Object property overload
   *
   * @param String Keyname ($Object->Keyname)
   * @param String Value
   */
  function __set($key, $value)
  {
    if ( array_search($key, $this->validEntities) === false )
    throw new Exception('[' . get_class($this) . '] Attempt to set an invalid ' . get_class($this) . ' entity: ' . $key);

    $this->entity[$key] = $value;
  }

  /**
   * PHP Object property overload
   *
   * @param String Keyname ($Object->Keyname)
   * @return String Value
   */
  function __get($key)
  {
    if ( array_search($key, $this->validEntities) === false )
      throw new Exception('[' . get_class($this) . '] Attempt to get an invalid ' . get_class($this) . ' entity: ' . $key);

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
  protected function validEntity($key, $default = '')
  {
    if ( array_search($key, $this->validEntities) )
    throw new Exception('Attempting to add a duplicate valid entity: ' . $key);

    $this->entity[$key] = $default;
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
      return false;
    return (isset($this->entity[$key]) && $this->entity[$key] !== null);
  }

  static function revision()
  {
    /**
     * @todo figure out if we're able to read ourselves to get our revision
     */
  }
}
