<?php
/**
 * Class Information
 *
 * Uses reflection to pull header comments
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2006-2011 the Framewerk Development Group
 * @package    Engine
 * @subpackage Core
 */

class tClassInfo
{
  private $values;

  /**
   * Constructor
   *
   * Pass in the class name (string) or ReflectionClass to get key/value pairs for
   *
   * @param string|ReflectionClass $class
   */
  function __construct($class)
  {
    if ($class instanceof ReflectionClass)
      $reflection = $class;
    else
      $reflection = new ReflectionClass($class);

    // Get our comments
    $comments = $reflection->getDocComment();

    // Explode the comments into an array via the linefeed
    $lines = explode("\n", $comments);

    // Setup our value array and set the first few entries based upon hard line format values
    $this->values = array();
    $this->values['name'] = trim(ltrim($lines[1], ' * '));
    $this->values['desc'] = trim(ltrim($lines[3], ' * '));

    // Loop through the lines and parse out the key/value pairs
    foreach ( $lines as $line )
    {
      // Get the output of a parsed line
      $x = $this->parseLine($line);
      // If the output is an array, merge the arrays
      if ( is_array($x) ) $this->values = array_merge($this->values, $x);
    }
  }

  /**
   * Break out the key/value pair from a comment line
   *
   * @param String $line
   * @return Array
   */
  function parseLine($line)
  {
    $keyword = '';
    $mode = '';
    $value = '';
    for ( $y = 0; $y < strLen($line); $y++ )
    {
      switch ( $mode )
      {
        case 'keyword':
          if ( $line[$y] != ' ' )
          {
            $keyword .= $line[$y];
          } else {
            $mode = 'value';
            $started = false;
          }
          break;
        case 'value':
          $value .= $line[$y];
          break;
        default:
          if ( $line[$y] == '@' ) $mode = 'keyword';
          break;
      }
    }

    // If the keyword is version, strip out the svn Rev: format info
    if ( $keyword == 'version' )
    {
      $temp = explode(':', $value);
      $value = trim(rtrim($temp[1], '$'));
    }

    // If we have a keyword, return the array
    if ( strlen($keyword) ) return array($keyword => trim($value));
  }

  // Get a variable from the class info value array
  function __get($variable)
  {
    return $this->values[$variable];
  }

}
