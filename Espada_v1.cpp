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
void fuck( char * nick, int i );
// int arraysize( char * array );
/**
This code is sorta fucked right now. It works but there is a room for improvement
g++ -o first first.cpp -std=c++11 -lboost_regex -pthread -lpthread -Wl,--no-as-needed

**/

class Socket{
	
	private:
		char buffer[MAXSIZE];

	public:
		int sock;
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
		char *name;
		char *chan;
		void connect(const char *host , const char *port, const char *n, const char *channel){

			name =  (char *)n;
			chan = (char *)channel; 
			Socket::SockCon( host , port );
			main();
			

		}
//	private: // will be private when im done testing
		void pong( char *data, int size ){
 
			char buff[MAXSIZE];
			char *x = shift(5, data, size);
			sprintf(buff, "PONG %s", x );
			free(x);
			Socket::sendData( buff );
			memset(&buff, 0, sizeof(buff));

		}
		void privmsg( char *who, const char *data = "Trolololololo"){

			char buff[MAXSIZE];
			sprintf(buff, "PRIVMSG %s :%s ", who ,data);
			Socket::sendData( buff );
			memset(&buff, 0, sizeof(buff));
			
		}
		void join( char *chan ){

			char buff[MAXSIZE];
			sprintf(buff, "JOIN %s", chan);
			Socket::sendData( buff );
			memset(&buff, 0 , sizeof(buff));
		}
		void part(  char *chan, const char * msg = "Leaving"){

			char buff[MAXSIZE];
			sprintf(buff, "PART %s :%s", chan, msg);
			Socket::sendData( buff );
			memset(&buff, 0 , sizeof(buff));	
		}
		void nick( char *n ){

			char buff[MAXSIZE];
			sprintf(buff, "NICK %s", n);
			Socket::sendData( buff );
			memset(&buff, 0 , sizeof(buff));
		}
		void user( const char *user = " Ass Ass  Ass ASs" ){

			char buff[MAXSIZE];
			sprintf(buff, "USER %s", user);
			Socket::sendData( buff );
			memset(&buff, 0 , sizeof(buff));
		}
		void main( )
		{

			nick( name );
			user();

			while( 1 ){
				char * buff = Socket::getData();
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
			bool spam = false;
			char *who;
			while( 1 ){
				char * buff = Socket::getData();
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
						if(result[4] == "!pewpewpew"){
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

				}if ( spam == true )
				{
					for(int xyz = 0; xyz < 20; xyz++){
						privmsg(who);
					}
					sleep(2);
				}
				memset(&buff, 0 , sizeof(buff));

			}
		}


};
int main(){
	try{

		for(int i = 0; i < 4 ; i++){
			char nick[MAXSIZE];
			sprintf(nick, "poop%i", i);
			cout << nick;
			thread first(fuck, nick , i);
			first.detach();
		}
		while(1){
			sleep(1);
		}

	}catch( char const* e ){

		cout << "Caught Exception: " << e << endl;

	}
	return 0;
}
void fuck( char * nick, int num ){
	char name[MAXSIZE];
	sprintf(name, "%s%i", nick, num);
	IRC * irc = new IRC();
	irc->connect( "localhost", "1234", name, "#flood");  // irc server here
	//cout << nick;
	memset(&nick, 0 , sizeof(nick));
	memset(&name, 0 , sizeof(nick));
}

char  *shift( int mov, char  * array, int size){
	
	int val = size - mov;
	char *n = (char*)malloc(val);

	for(int i =0;i < val ; i ++ ){
	
		n[i] = array[i+mov];
	
	}
	
	return n;


}

