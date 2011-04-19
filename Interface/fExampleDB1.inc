<?php

class fExampleDB1 extends fApplication
{
  function __construct()
  {
    parent::__construct();
  }

  public function execute()
  {
    /* Set Content-Type to JavaScript */
    header("Content-Type: text/javascript");

    /* Turn off E_NOTICE errors due to unset index errors */
    $err = error_reporting();
    if($err & E_NOTICE)
      error_reporting($err ^ E_NOTICE);

    /* Determine script URI for easy reference in callbacks later */
    $scriptBaseURI = 'http'.($_SERVER['HTTPS']=='on'?'s':'')."://{$_SERVER['HTTP_HOST']}{$_SERVER['REDIRECT_URI']}";

    try {

      /* Make sure that we have a valid JSONP request */
      if(isset($_GET['callback']) && !preg_match(';^[A-z0-9]+$;', $_GET['callback']))
        throw new Exception('bad callback ', 400);

      /* Initialize data response */
      $data = null;

      // Main/Slave Database
      $select = pg_query($this->db(), "SELECT NOW():");
      $row = pg_fetch_assoc($select);
      $data = $row['now'];

      // Master Database
      //$insert = pg_query($this->db('master'), "INSERT INTO foo (bar) VALUES ('derp');");

      if(isset($_GET['callback'])) {
        $callBack = $_GET['callback'];
        $responseData = &$data;
        $responseDetails = null;
        $responseStatus = 200;
      } else {
        $responseData = &$data;
      }
    } catch (Exception $e) {
      if(isset($_GET['callback'])) {
        $callBack = null;
        $responseData = null;
        $responseDetails = $e->getMessage();
        $responseStatus = $e->getCode() ? $e->getCode() : 400;
      } else {
        $responseData = '/* error: '.$e->getMessage().' */';
      }
    }

    if(isset($_GET['callback'])) {
      if($callBack!==null) echo "{$_GET['callback']}(";
      echo json_encode(array('responseData'=> $responseData,
                             'responseDetails'=> $responseDetails,
                             'responseStatus'=> $responseStatus));
      if($callBack!==null) echo ");";
      echo "\n";
    } else {
      echo $responseData;
    }
  }

}

