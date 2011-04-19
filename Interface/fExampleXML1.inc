<?php

class fExampleXML1 extends fApplication
{
  private $baseURI;
  private $uriStack;

  function __construct()
  {
    $this->baseURI = '/example1/';
    $this->uriStack = array();
    if(preg_match(";^{$this->baseURI}(.*);", $_SERVER['REDIRECT_URL'], $regs)) {
      $this->uriStack = split('/',$regs[1]);
    }

    parent::__construct();
  }

  function execute()
  {

    try {
      set_time_limit(300);
      $function = array_shift($this->uriStack);
      switch($function) {
      case 'test':// /example1/test
        $this->doTest();
        break;
      default:
        throw new Exception('Invalid request URL.');
      }

    } catch (Exception $e) {
      $this->error($e->getMessage());
    }

    $this->error('Unexpected execution path.');
  }

  private function respond(&$data)
  {
    $this->respondXML($data);
  }

  private function error($description)
  {
    $this->respondXML($description);
  }

  private function respondXML(&$data)
  {
    @header("Content-Type: text/xml");

    //$data = xmlentities($data);

    $xml = <<<XML
    <?xml version="1.0"?>
    <response>
      <data>{$data}</data>
    </response>
XML;

    echo $xml;
    die();
  }

  private function doTest()
  {
    $this->respondXML("Hello World!");
  }


}
