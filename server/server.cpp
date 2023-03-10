/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sel-mars <sel-mars@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/08 18:22:31 by sel-mars          #+#    #+#             */
/*   Updated: 2023/03/10 13:45:57 by sel-mars         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
/*                                        Message                                       */
/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

std::ostream& irc::operator<<( std::ostream& o_, irc::message& message_ ) {
	o_ << "\033[2mprefix\t: ";
	if ( message_._prefix.empty() ) o_ << "<empty>";
	else
		o_ << "`\033[22m" << message_._prefix << "\033[2m`";
	o_ << "\n\033[2mcommand\t: ";
	if ( message_._command.empty() ) o_ << "<empty>";
	else
		o_ << "`\033[22m" << message_._command << "\033[2m`";
	o_ << "\n\033[2mparams\t: ";
	if ( message_._params.empty() ) o_ << "<empty>";
	else
		for ( std::size_t idx = 0; idx < message_._params.size(); ++idx )
			o_ << "\n" << idx << "\t--> `\033[22m" << message_._params[ idx ] << "\033[2m`";
	o_ << "\033[22m\n";
	return o_;
}

void irc::message::clear( void ) {
	this->_prefix.clear();
	this->_command.clear();
	this->_params.clear();
}

void irc::message::parseMsg( std::string& msg_ ) {
	std::cout << msg_ << "\n";
	//
	if ( !msg_.find( COLON ) ) {
		this->_prefix = msg_.erase( 0, 1 ).substr( 0, msg_.find( SPACE ) );
		msg_.erase( 0, msg_.find( SPACE ) + 1 );
	}
	this->_command = msg_.substr( 0, msg_.find( SPACE ) );
	msg_.erase( 0, msg_.find( SPACE ) + 1 );
	for ( std::size_t pos_space = msg_.find( SPACE ); !msg_.empty();
		  pos_space				= msg_.find( SPACE ) ) {
		this->_params.push_back( msg_.substr( 0, msg_.find( SPACE ) ) );
		pos_space == std::string::npos ? msg_.erase( 0 ) : msg_.erase( 0, pos_space + 1 );
	}
	//
	std::cout << *this << '\n';
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
/*                                        Server                                        */
/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

irc::server* irc::server::__serv		  = NULL;
std::string	 irc::server::__host_name	  = "";
std::string	 irc::server::__host_addr	  = "";
std::string	 irc::server::__creation_date = "";

/* ctor ───────────────────────────────────────────────────────────────────────────── */

void irc::server::sethostAddr( void ) {
	char			tmp_hstname[ 128 ];
	struct hostent* hostentry;
	if ( gethostname( tmp_hstname, 128 ) ) ERRNO_EXCEPT;
	this->__host_name = tmp_hstname;
	hostentry		  = gethostbyname( tmp_hstname );
	if ( !hostentry ) ERRNO_EXCEPT;
	this->__host_addr = inet_ntoa( *reinterpret_cast< in_addr* >( hostentry->h_addr_list[ 0 ] ) );
}

irc::server::server( const int& ac, char** av ) {
	this->parse_args( ac, av );
	this->_buff = new char[ 513 ];
	// this->_welcome_msg += "\033[2m";
	this->_welcome_msg += "Greetings and welcome to our\n";
	this->_welcome_msg += "Internet Relay Chat server !\n";
	this->_welcome_msg += "We hope you feel at home here.\n";
	// this->_welcome_msg += "\033[22m";
	this->_welcome_msg += "\n";
	this->_shutdown_msg += "\n\n";
	// this->_shutdown_msg += "\033[2m";
	this->_shutdown_msg +=
		"Thank you for usign our IRC server, it will be shutting down for maintenance.\n";
	this->_shutdown_msg += "Until we chat again !";
	// this->_shutdown_msg += "\033[22m";
	this->_shutdown_msg += "\n";
	sethostAddr();

} // ctor

/* dtor ───────────────────────────────────────────────────────────────────────────── */

irc::server::~server( void ) {
	delete[] this->_buff;
	this->_buff = NULL;
} // dtor

/* parse_args ─────────────────────────────────────────────────────────────────────── */

void irc::server::parse_args( const int& ac, char** av ) {
	if ( ac != 3 ) ARGS_EXCEPT;
	std::stringstream ss;
	ss << av[ 1 ];
	ss >> this->_port;
	ss.str( std::string() );
	ss.clear();
	ss << this->_port;
	std::string port_str( &av[ 1 ][ av[ 1 ][ 0 ] && av[ 1 ][ 0 ] == '+' ] );
	if ( ss.str().compare( port_str ) || !this->_port ) PORT_EXCEPT;
	this->_password = av[ 2 ];
} // parse_args

/* signal_handler ─────────────────────────────────────────────────────────────────── */

void irc::server::staticSigHandler( int ) {
	__serv->shutDownServer();
	exit( 0 );
} // signal_handler

/* accept_client ──────────────────────────────────────────────────────────────────── */

void irc::server::acceptClient( void ) {
	pollfd pfd;
	bzero( &pfd, sizeof( pfd ) );
	pfd.fd = accept( this->_sockets.front().fd, NULL, NULL );
	if ( pfd.fd == -1 ) return;
	pfd.events = POLLIN | POLLOUT;
	this->_sockets.push_back( pfd );
	this->_clients.push_back( client() );
	this->_clients.rbegin()->_msg_out = REGISTRATION_SUCCESS( "souf", "souf" );
	std::cout << "\033[2m"
			  << "Client `" << this->_clients.rbegin()->_user._nickname << "` connected"
			  << "\033[22m\n";
} // accept_client

/* diconnect_client ───────────────────────────────────────────────────────────────── */

void irc::server::disconClient( std::vector< pollfd >::iterator& socket_it_,
								std::vector< client >::iterator& client_it_ ) {
	close( socket_it_->fd );
	std::cout << "\033[2m"
			  << "Client `" << client_it_->_user._nickname << "` disconnected"
			  << "\033[22m\n";
	socket_it_ = this->_sockets.erase( socket_it_ ) - 1;
	client_it_ = this->_clients.erase( client_it_ ) - 1;
} // diconnect_client

/* send_message ───────────────────────────────────────────────────────────────────── */

void irc::server::sendMsg( std::vector< pollfd >::iterator& socket_it_,
						   std::vector< client >::iterator& client_it_ ) {
	if ( client_it_->_msg_out.empty() ) return;
	int bytes_sent =
		send( socket_it_->fd, client_it_->_msg_out.c_str(), client_it_->_msg_out.length(), 0 );
	if ( bytes_sent == -1 ) THROW_EXCEPT( "Unable to send() data to client" );
	client_it_->_msg_out.erase( 0, bytes_sent );
} // send_message

/* receive_message ────────────────────────────────────────────────────────────────── */

void irc::server::handleMsg( std::vector< client >::iterator& client_it_ ) {
	client_it_->_message.parseMsg( client_it_->_msg_in.erase( client_it_->_msg_in.size() - 2 ) );
	client_it_->_message.clear();
}

void irc::server::recvMsg( std::vector< pollfd >::iterator& socket_it_,
						   std::vector< client >::iterator& client_it_ ) {
	bzero( _buff, 513 );
	int bytes_rcvd = recv( socket_it_->fd, _buff, 512, 0 );
	if ( bytes_rcvd == -1 ) {
		client_it_->_msg_in.clear();
		THROW_EXCEPT( "Unable to recv() data from client" );
	}
	client_it_->_msg_in.append( _buff, bytes_rcvd );
	if ( client_it_->_msg_in.rfind( CRLF ) ) { this->handleMsg( client_it_ ); }
} // receive_message

/* initialize_server ──────────────────────────────────────────────────────────────── */

void irc::server::initServer( void ) {
	addrinfo hints, *servinfo = NULL, *ptr = NULL;
	bzero( &hints, sizeof( hints ) );
	hints.ai_family	  = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags	  = AI_PASSIVE;
	std::stringstream ss;
	ss << this->_port;
	int i = getaddrinfo( NULL, ss.str().c_str(), &hints, &servinfo );
	if ( i ) i == EAI_SYSTEM ? ERRNO_EXCEPT : ERRCODE_EXCEPT( i );
	pollfd mastersocket;
	bzero( &mastersocket, sizeof( mastersocket ) );
	for ( ptr = servinfo; ptr; ptr = ptr->ai_next )
		if ( ( mastersocket.fd = socket( servinfo->ai_family, servinfo->ai_socktype,
										 servinfo->ai_protocol ) ) == -1 )
			continue;
	mastersocket.events = POLLIN | POLLOUT;
	i					= 1;
	if ( mastersocket.fd == -1 ||
		 setsockopt( mastersocket.fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof( i ) ) == -1 ||
		 fcntl( mastersocket.fd, F_SETFL, O_NONBLOCK ) == -1 ||
		 bind( mastersocket.fd, servinfo->ai_addr, servinfo->ai_addrlen ) == -1 ||
		 listen( mastersocket.fd, BACKLOG ) == -1 )
		ERRNO_EXCEPT;
	this->_sockets.push_back( mastersocket );
	freeaddrinfo( servinfo );
	std::time_t rawtime;
	std::time( &rawtime );
	( this->__creation_date = std::asctime( localtime( &rawtime ) ) )
		.erase( this->__creation_date.end() - 1 );
} // initialize_server

/* run_server ─────────────────────────────────────────────────────────────────────── */

void irc::server::runServer( void ) {
	__serv = this;
	std::signal( SIGINT, server::staticSigHandler );
	std::vector< client >::iterator client_it;
	std::vector< pollfd >::iterator socket_it;
	// start:
	std::cout << "\033[2m\033[4m"
			  << "IRC Server started successfully ~ " << this->__host_addr << ':' << this->_port
			  << "\033[22m\033[24m\n\n";
	while ( 1 ) {
		if ( poll( &this->_sockets.front(), this->_sockets.size(), -1 ) == -1 ) ERRNO_EXCEPT;
		if ( this->_sockets.front().revents == POLLIN ) acceptClient();
		for ( client_it = this->_clients.begin(), socket_it = this->_sockets.begin() + 1;
			  client_it != this->_clients.end(); ++client_it, ++socket_it ) {
			if ( socket_it->revents & ( POLLERR | POLLHUP | POLLNVAL ) ) {
				disconClient( socket_it, client_it );
				continue;
			}
			try {
				if ( socket_it->revents & POLLIN ) recvMsg( socket_it, client_it );
				if ( socket_it->revents & POLLOUT ) sendMsg( socket_it, client_it );
			} catch ( std::exception& e ) { std::cerr << "\033[1;31m" << e.what() << "\033[0m\n"; }
		}
	}
	// restart:
	// 	shutDownServer();
	// 	goto start;
	// shutdown:
	// 	shutDownServer();
	// 	exit( 0 );
} // run_server

/* shut_down_server ─────────────────────────────────────────────────────────────────── */

void irc::server::shutDownServer( void ) {
	std::vector< client >::iterator client_it;
	std::vector< pollfd >::iterator socket_it;
	for ( client_it = this->_clients.end() - 1, socket_it = this->_sockets.end() - 1;
		  socket_it != this->_sockets.begin(); --socket_it, --client_it ) {
		if ( socket_it->revents & POLLOUT ) {
			client_it->_msg_out = _shutdown_msg;
			while ( !client_it->_msg_out.empty() ) sendMsg( socket_it, client_it );
			close( socket_it->fd );
		}
	}
	if ( close( this->_sockets.begin()->fd ) ) ERRNO_EXCEPT;
	std::cout << "\n\n\033[2m\033[4m"
			  << "IRC Server shut down successfully"
			  << "\033[22m\033[24m\n";
} // signal_handler
