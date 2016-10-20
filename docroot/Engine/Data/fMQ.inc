<?php
/**
 * Database helper class
 *
 * Manages AMQP resources
 *
 * @author     R. Brenton Strickler <rbrenton@gmail.com>
 * @link       http://www.framewerk.org/
 * @license    BSD
 * @copyright  Copyright 2013 Framewerk Development Group
 * @package    Toolkit
 */

class fMQ extends fSingletonObject
{
  private $mq = null;
  private $ch = null;
  private $exchanges = array();

  private static $instance;

  function __construct()
  {
    parent::__construct();
  }

  function __destruct()
  {
    if($this->mq instanceof AMQPConnection) {
      $this->mq->disconnect();
    }
  }

  /**
   * Get or create fMQ instance
   * 
   * @return fMQ
   */
  public static function getInstance( )
  {
    if ( self::$instance === NULL )
    {
      self::$instance = new fMQ();
    }
    return self::$instance;
  }

  public static function conn($exchange)
  {
    return self::getInstance()->connect($exchange);
  }

  public function connect($exchange)
  {
    if($this->mq instanceof AMQPConnection) {
      if($this->exchanges[$exchange])
        return $this->exchanges[$exchange];
    }

    if(!($this->mq = new AMQPConnection(array('host'=>RABBITMQ_HOST, 'login'=>RABBITMQ_USER, 'password'=>RABBITMQ_PASS, 'vhost'=>RABBITMQ_VHOST))))
      throw new Exception('AMQP config error.');

    if(!$this->mq->connect())
      throw new Exception('AMQP connection error.');

    if(($this->ch instanceof AMQPChannel && !$this->ch->isConnected()) ||
        !($this->ch instanceof AMQPChannel)) {
      $this->ch = new AMQPChannel($this->mq);
    }

    $ex = new AMQPExchange($this->ch);
    $ex->setName($exchange);

    $ex->setType(AMQP_EX_TYPE_TOPIC);
    $ex->setFlags(AMQP_DURABLE);
    $ex->declare();

    return ($this->exchanges[$exchange] = $ex);
  }

}

