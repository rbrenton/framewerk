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

    // Set our timezone and locale
    putenv('TZ=' . SYSTEM_TIMEZONE);
    setLocale(LC_ALL, SYSTEM_LOCALE);

    // Process our ini file settings
    if(defined(PHP_SESSION_DIR)) {
      if (!is_dir(PHP_SESSION_DIR)) mkdir(PHP_SESSION_DIR, 0777);
      ini_set('session.save_path', PHP_SESSION_DIR);
    }
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
