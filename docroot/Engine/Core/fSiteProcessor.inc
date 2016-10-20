<?php
/**
 * Site Processor Class.
 *
 * This class processes the site.xml document and performs the appropriate actions based
 * upon selected keys and the various attributes and nodes per key.
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @author     R. Brenton Strickler <rbrenton@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2006-2011 the Framewerk Development Group
 */

class fSiteProcessor
{
  protected $fMain;
  private $uri, $uriValues, $uriDepth, $siteXML, $class;

  /**
   * Construct the fSiteProcessor Object
   *
   * @param SimpleXMLObject $sitexml
   */
  function __construct()
  {
    $this->fMain = fMain::getInstance();

    // Load the site.xml document
    if(is_file('Config/site.xml'))
    {
      $file = new fFile('Config/site.xml');
      $data = $file->read();
      $this->siteXML = @simplexml_load_string( $data );
    }
    if(!$this->siteXML)
      throw new fHTTPException(500, 'There was an error parsing site.xml');

    $this->uriDepth = 0;
  }

  /**
   * Comments would be nice here, eh? What does this function actually do?
   */  
  function processKey($node, $depth = 0)
  {
    // If we're this deep, we're deep in it
    if($depth > 32) return false;

    $process = false;
    $processed = false;

    // Get the URI we want to use
    $uri = '';
    for ( $i = $depth; $i < count($this->uri); $i++)
      $uri .= '/' . $this->uri[$i];
    
    if($uri == '') 
      $uri = '/';

    // Run xpath to find matching URI nodes
    $xml = $node->xpath('key[starts-with("' . $uri . '", @uri)]');

    // Sort the keys
    usort($xml, array('fSiteProcessor', '_arrangeKeyNodes'));
    
    foreach ( $xml as $key )
    {
      if(isset($key['uri']))
      {
        $count = count(explode('/', trim($key['uri'], '/')));
        $uri = '';

        for ( $y = $depth; $y < ( $count + $depth); $y++)
          $uri .= '/' . $this->uri[$y];
        
        // Process and see if our URI's match
        if($uri == $key['uri'])
        {
          $process = true;

          // If we have a hostname see if we have a match
          if(isset($key['host']))
            if(strtolower($key['host']) != strtolower($_SERVER['SERVER_NAME']))
              $process = false;

          // Force SSL if required
          if(intval($key['force-ssl']) == 1) {
            if($_SERVER['SERVER_PORT'] != 443) {
              header("Location: https://{$_SERVER['HTTP_HOST']}{$_SERVER['REQUEST_URI']})");
              exit();
            } else if($_SERVER['HTTPS']!='on') {
              throw new fHTTPException(500, "SSL required but not properly configured.");
            }
          }

          // Process any sub-keys if we have them
          $response = false;
          if(isset($key->key) && ( $process == true )) {
            $response = $this->processKey($key, ($depth + $count));
            if(( $process == false ) && ( $response == true ) ) $process = true;
          }


          // If we have permission and match conditions process the request
          if(( $process == true ) && ( $response == false )) {
            $this->setURIValues(($depth + $count));

            if(isset($key->class))
              $this->class = strval($key->class);

            // Process any actions if we have them
            if(( isset($key->action) ) && ( $response != true ))
            {
              $response = $this->processAction($key, ($depth + $count));
              if(( $process == false ) && ( $response == true ) ) $process = true;
            }

            // If we didn't process a sub key or action successfully, try and see if we can execute a function on this class
            if($response != true)
            {
              if(strlen($this->class))
              {
                $className =  $this->class;
                if(!class_exists($className))
                  throw new fHTTPException(500, "Missing interface class {$className}.");
                // Check to see if we have a function and execute it if so, otherwise try and execute the default function
                if(isset($action->function))
                     $functionName = strval($action->function);
                else $functionName = 'execute';
                if(!method_exists($className, $functionName))
                  throw new fHTTPException(500, "Missing interface method {$functionName} for class {$className}.");
                $obj = new $className();
                $obj->$functionName($this->uriValues);
                $processed = true;
              }
            }
          }
        }
      }
      // Break if our xpaths dont match anymore or if we've processed a result
      if(( $processed == true ) || ( $key['uri'] != $uri ))
        break;
    }

    return $process;
  }

  function processAction($node, $depth = 0)
  {
    $this->uriDepth = $depth;

    // Loop through the actions that were passed in
    foreach ( $node->action AS $action)
    {
      $process = false;
      $count = 0;
      if(isset($action['uri']))
      {
        $count = count(explode('/', ltrim($action['uri'], '/')));
        $uri = "";
        for ( $y = $depth; $y < ( $count + $depth); $y++)
          $uri .= '/' . $this->uri[$y];

        // Process and see if our URI's match
        if($uri == $action['uri'])
          $process = true;

      } else
      $process = true;

      // Set URI Values
      $this->setURIValues(($depth + $count));

      // If we still can process, check our permissions and conditions
      if($process == true)
      {
        // If we have a hostname see if we have a match
        if(isset($key['host']))
          if(strtolower($key['host']) != strtolower($_SERVER['SERVER_NAME']))
            $process = false;
      }

      // If we can process, do so
      if($process == true)
      {
        // Set the title if there is one
        if(isset($action->title))
          $this->fOutput->setPageName($action->title);

        // Set the pageCategory if there is a group
        if(isset($action->group))
          $this->fOutput->setPageCategory($action->group);

        // Override our theme if we need to
        if(isset($action->theme))
          fTheme::getInstance()->override(strval($action->theme));

        // If class is set instantiate it
        if(isset($action->class))
          $this->class = strval($action->class);

        // If we have a class set run the function if it's there
        $processed = false;
        if(isset($this->class))
        {
          $className = $this->class;
          $obj = new $className();
          $className = $this->class;
          if(isset($action->function))
          {
            $functionName = strval($action->function);
            $processed = $obj->$functionName($this->uriValues);
          }
        }

        // If we have sub-actions try and perform them
        if(isset($action->action) && ( $processed != true ))
          $processed = $this->processAction($action, ($depth + $count));
      }

      // Break if our xpaths dont match anymore or if we've processed a result
      if(( $processed == true ) || ( $action['uri'] != $uri ))
      break;
    }
    return $processed;
  }

  function process($uri)
  {
    $uri = preg_replace('/([?#].*)/', '', $_SERVER['REQUEST_URI']);

    $temp = explode('/', trim($uri, '/'));
    $this->uri = array();
    foreach ( $temp AS $value)
    {
      if(strlen(Trim($value)))
      {
        $this->uri[] = $value;
      }
    }

    if(!$this->processKey($this->siteXML))
      throw new fHTTPException(404, 'File Not Found');
  }

  /**
   * Rebuild our fSession Keyword and URI Values based upon our execution attempts
   */
  function setURIValues($depth)
  {
    // Set our uriValues
    $uriValues = array();
    for ( $y = $depth; $y < count($this->uri); $y++)
      if(strlen($this->uri[$y]))
        $uriValues[] = $this->uri[$y];
    $this->uriValues = $uriValues;

  }

  /**
   * Arranges key nodes by string length
   *
   * @param string $a
   * @param string $b
   */
  private static function _arrangeKeyNodes($a, $b)
  {
    if(!isset($a['uri']))
      return 1;
    if($a['uri'] == $b['uri'])
      return 0;
    if(strlen($a['uri']) > strlen($b['uri']))
      return -1;
    else
      return 1;
  }

  /**
   * Takes site.xml key options and works backwards to generate the uri's leading to the matched key
   * Options left blank will be skipped
   *
   * @param string $class
   * @param string $function
   * @param string $title
   * @param string $group
   */
  public function siteURI($class, $function, $title, $group)
  {
      // Make this use static accessors so that we can call fSiteProcessor::siteURI()
    if(!isset($class) && !isset($function) && !isset($title) && !isset($group))
      return strval(fMain::getInstance()->configurationXML->site->baseURI) . fMain::getInstance()->keyword;

    $xml = fMain::getInstance()->site;
    $xpath = '//key[@uri]';
    if(isset($class))
      $xpath .= "[class='" . $class . "']";
    if(isset($function))
      $xpath .= "[function='" . $function . "']";
    if(isset($title))
      $xpath .= "[title='" . $title . "']";
    if(isset($group))
      $xpath .= "[@group='" . $group . "']";
    $results = $xml->xpath($xpath);
    if(!sizeof($results))
      return false;
    $uri     = array();
    $uri[]   = strval($results[0]['uri']);
    if(sizeof($xml->xpath($newPath = $xpath . '/..')))
      $uri = self::siteURIParent($xml, $newPath, $uri);
    return join('', array_reverse($uri));
  }

  /**
   * Private recursive function to generate an array of uri's leading to the given xpath
   *
   * @param object $xml
   * @param string $xpath
   * @param string $uri
   */
  private static function siteURIParent($xml, $xpath, $uri)
  {
    $parent = $xml->xpath($xpath);
    $uri[]  = strval($parent[0]['uri']);
    if(sizeof($xml->xpath($newPath = $xpath . '/..')))
      $uri = self::siteURIParent($xml, $newPath, $uri);
    return $uri;
  }

  static function returnClassURI($className)
  {
    $template = <<<TEMPLATE
<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
 <xsl:output encoding="ISO-8859-15" omit-xml-declaration="yes" method="xml" version="1.0" indent="yes" />
 <xsl:template match="/site">
   <xsl:for-each select="//key/class[.='tSoapServer']">
    <xsl:for-each select="ancestor-or-self::key">
     <xsl:value-of select="@uri" />
    </xsl:for-each>
   </xsl:for-each>
 </xsl:template>
</xsl:stylesheet>
TEMPLATE;
    $xsl = DOMDocument::loadXML($template);
    $proc = new XSLTProcessor();
    $proc->importStyleSheet($xsl);
    $dom = new DOMDocument('1.0', 'UTF-8');
    $temp = $dom->importNode(dom_import_simplexml(fMain::getInstance()->site), true);
    $dom->appendChild($temp);
    $result = $proc->transformToXml($dom);
    return trim($result);
  }
}
