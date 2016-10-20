<?php
/**
  * Framewerk Base Class
  *
  * This is the base processing class for the fMain Engine.
  * It is responsible for coordinating the use of all the other classes
  * and actions in Framewerk.
  *
  * @author     Gavin M. Roy <gavinmroy@gmail.com>
  * @author     R. Brenton Strickler <rbrenton@gmail.com>
  * @link       http://framewerk.org
  * @license    http://opensource.org/licenses/bsd-license.php BSD License
  * @copyright  Copyright 2004-2011 the Framewerk Development Group
  */
class fMain extends fSingletonObject
{
  public $configurationXML, $siteXML;

  // Object variables
  private static $instance = null;
  private $processed = false;

  /**
   * Checks to see if the object is instanced in the internal static variable $instance
   * and if not, does so
   *
   * @return fMain instance
   */
  static function getInstance()
  {
    if ( self::$instance == null ) 
    {
      self::$instance = new fMain();
    }
    return self::$instance;
  }

  /**
   * Forcefully unset a singleton instance.
   */
  static function destroy()
  {
    self::$instance = null;
  }

  /**
   * Framewerk Object Constructor
   *
   * DO NOT INSTANCE ANY OBJECTS THAT REFERENCE fMain::getInstance() WITHIN THIS FUNCTION
   * OR A NASTY LOOP WILL OCCUR
   */
  protected function __construct()
  {
    // Set our error handler
    set_error_handler('errorHandler');

    // Read our configuration file

    // Load our registry
    $registry = new fRegistry('fMain');
    $this->configurationXML = &$registry;

    // Set our timezone and locale
    putenv('TZ=' . $registry->timezone);
    setLocale(LC_ALL, strval($registry->locale));

    // Process our ini file settings
    foreach($registry->php AS $setting)
      foreach($setting AS $key=>$value)
        ini_set(strval($key), strval($value));
  }

  /**
   * Initialize Framewerk Object and subobjects
   */
  function process()
  {
    static $processed=false;

    if($processed) return;
    $processed=true;

    // Try and execute the request
    $fSiteProcessor = new fSiteProcessor();
    $fSiteProcessor->process();
  }
}
