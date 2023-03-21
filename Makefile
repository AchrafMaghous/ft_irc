# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: sel-mars <sel-mars@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/03/08 14:44:10 by sel-mars          #+#    #+#              #
#    Updated: 2023/03/21 11:27:23 by sel-mars         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	=	ircserv

CPP		=	c++

FLAGS	=	-O3 -std=c++98 -Wall -Wextra -Werror -Wpedantic -Wno-c++11-extensions

COMMANDS = $(addprefix commands/,$(addsuffix .cpp,commands JOIN NICK PASS PONG QUIT USER PRIVMSG))

SERVER = $(addprefix server/,$(addsuffix .cpp,channel_ops client_ops message_ops server_ops server))

CHANNEL = $(addprefix channel/, channel.cpp)

SRCS	=	main.cpp  $(addprefix srcs/,message.cpp utils.cpp $(COMMANDS) $(SERVER) $(CHANNEL))

all: $(NAME)

$(NAME): $(SRCS)
	@$(CPP) $(FLAGS) $(SRCS) -o $@

clean:
	@rm -f $(NAME)

fclean: clean

re: fclean all

.PHONY: all $(NAME) clean fclean re
