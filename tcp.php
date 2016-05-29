<?php


class SockException extends Exception{


	public function __construct( ){

		parent::__construct( );
	}
}

class TCP{


	protected $fd 	 	= NULL;
	private $host 		= '';
	private $port 		= 0;
	private $ssl  	 	= 0;
	private $timeout	= 10;
	private $_type		= 0;

	public function __construct( $host, $port, $ssl = 0 , $timeout = 10 ){

		$this->host 	= $host;
		$this->port 	= $port;
		$this->ssl  	= $ssl;
		$this->timeout  = $timeout;

	}
	public function connect( ){

		if( $this->ssl != 0 ) $this->host = 'ssl://' . $this->host;	

		try{

			$this->fd 	= fsockopen($this->host, $this->port, $errno, $errstr, 2 );		
		}catch(\Exception $e){

			$this->fd  	= @socket_create(AF_INET, SOCK_STREAM, SOL_TCP) or $this->__throwException( $errstr, $errno );
			
			$this->_type++;

		}

	}
	public function send( $data ){

		fputs( $this->fd, $data . "\r\n" );

	}
	public function recv( ){

		return fgets( $this->fd ); 
	}
	public function __destruct( ){

		if( $this->fd !== NULL ) fclose( $this->fd );
	}
	private function __throwException( $errstr, $errno = 0 ){

		throw new SockException( $errstr, $errno );

	}
}

class IRC extends TCP{

	public function __construct( ){

		parent::__construct( );

	}
	public function pong( $p ){

		$this->send( "PONG $p" );

	}
	public function join( $channel ){

		if( is_array( $channel ) ):

			foreach( $channel as $c ):

				$this->send( "JOIN $c" );

			endforeach;

			return;

		endif;

		$this->send( "JOIN $channel" );

	}
	public function privmsg( $who, $data ){

		if ( is_array( $data ) ):

			foreach( $data as $d ):

				$this->send( "PRIVMSG $who :$d" );

			endforeach;

			return;

		endif;

		$this->send( "PRIVMSG $who :$data" );

	}
	public function notice( $who, $data ){

		if ( is_array( $data ) ):

			foreach( $data as $d ):

				$this->send( "NOTICE $who :$d" );

			endforeach;

			return;

		endif;

		$this->send( "NOTICE $who :$data" );

	}
}
