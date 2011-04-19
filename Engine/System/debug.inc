<?php
/**
 * Framewerk Error Handling
 *
 * Contains functions specifying how to handle PHP error messages and exceptions
 *
 * @author     Joe Hansche <madCoder@gamesurge.net>
 * @author     Rick Martinez <rick.martinez@gmail.com>
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2004-2011 the Framewerk Development Group
 * @package    Engine
 * @subpackage Library
 */

/**
 * Error Handler
 *
 * @param int $code
 * @param string $message
 * @param string $file
 * @param int $line
 */
function errorHandler($code, $message, $file, $line)
{


}

/**
 * Exception Handler
 *
 * @param Exception $exception
 */
function exceptionHandler($exception)
{
  try
  {
    $error = new fHTTPMessage($exception);
    $error->show();
  } catch ( Exception $e ) {
    echo "<pre><strong>\nA fatal Framewerk error occured that prevented it from handling an exception.</strong>\n\n<hr />\n";
    echo "Message: {$e->getMessage()}\n\n";
    echo "File: {$e->getFile()}\n";
    echo "Line: {$e->getLine()}\n\n";
    echo "{$e->getTraceAsString()}\n</pre>";
  }
  die();
}

// Set our error reporting levels and handlers
error_reporting(E_ALL ^ ( E_USER_ERROR | E_USER_NOTICE | E_USER_WARNING | E_NOTICE | E_WARNING ) );
set_error_handler('errorHandler', E_USER_ERROR | E_USER_NOTICE | E_USER_WARNING  | E_NOTICE | E_WARNING );
set_exception_handler('exceptionHandler');
