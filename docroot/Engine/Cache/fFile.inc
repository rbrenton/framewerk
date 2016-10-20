<?php
/**
 * File Read/Caching Class
 *
 * This class is for reading files in and caching them in SHM cache to increase performance
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @author     R. Brenton Strickler <rbrenton@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2007-2011 the Framewerk Development Group
 * @package    Engine
 * @todo       Add file writing and recaching on writing
 */


class fFile
{
  const CACHE_DURATION = 86400;

  private $fileName;
  private $disableCache;
  
  /**
   * Static cache
   *
   * @var array
   */
  private static $sCache = array();
  
  /**
   * Constructor
   *
   * Check the SHMOP Cache to see if there is a copy in cache, validate we can use it
   * otherwise read it new and cache it
   *
   * @param String $fileName
   * @return bool
   */
  function __construct($fileName, $disableCache = false)
  {
    $this->fileName = $fileName;
    $this->disableCache = $diableCache;
  }
  

  /**
   * Return the file contents
   *
   * @return string
   */
  function read()
  {
    // Turn fileName into realpath(fileName)
    $this->fileName = realpath($this->fileName);

    if(!isset(self::$sCache[$this->filename]))
    {
      if($this->disableCache) {
        self::$sCache[$this->fileName] = file_get_contents($this->fileName);
      } else {
        $keyFile = "framewerk::{$_SERVER['SERVER_NAME']}::fFile::fileContents::{$this->fileName}";
        $keyCached = "framewerk::{$_SERVER['SERVER_NAME']}::fFile::cacheTime::{$this->fileName}";

        $modifiedTime = filemtime($this->fileName);
        $cacheTime = apc_fetch($keyCached);
        if($cacheTime!==false && $modifiedTime<$cacheTime) {
          //cache is current
          self::$sCache[$this->fileName] = apc_fetch($keyFile);
        }
        if($cacheTime===false || $modifiedTime>=$cacheTime || self::$sCache[$this->fileName]=='') {
          //cache is not current
          self::$sCache[$this->fileName] = file_get_contents($this->fileName);
          apc_store($keyFile, self::$sCache[$this->fileName], fFile::CACHE_DURATION);
          apc_store($keyCached, time(), fFile::CACHE_DURATION);
        }
      }
    }

    return self::$sCache[$this->fileName];
  }

}
