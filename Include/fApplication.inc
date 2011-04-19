<?php
/**
 * Interface abstract object
 *
 * This abstract fApplication object is inherited by all Framewerk Interfaces
 * Interfaces are the applications in Framewerk which are responsible for the
 * direct interaction with the user, that is Interfaces are the web applications
 * themselves.
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2004-2011 the Framewerk Development Group
 * @package    Engine
 * @subpackage Abstract_Applications
 */

abstract class fApplication
{
  /**
   * @var fMain
   */
  protected $fMain;

  /**
   * Interface Constructor
   *
   * Sets the all the needed internal variables and engine parameters.
   *
   * @param object fMain object is required
   */
  function __construct()
  {
    // Set our objects and our object shortcuts
    $this->fMain      = fMain::getInstance();
  }

  /**
   * Framewerk Module execution Function
   *
   * This function must be extended by the module or it won't run
   *
   * @return bool true/false if execution of module was successful
   */
  function execute()
  {
    Throw new Exception("The module you have called has not properly extended the fApplication class");
  }

  protected function db($server='master')
  {
    return fDB::getInstance()->connect($server);
  }

}
