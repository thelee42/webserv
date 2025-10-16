NAME = webserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRC = main.cpp httpRequest.cpp Server.cpp configParser.cpp upload.cpp utils_http.cpp httpResponse.cpp utils.cpp
OBJDIR = .obj
OBJ = $(addprefix $(OBJDIR)/, $(SRC:.cpp=.o))
UPLOADROOT = ./www
UPLOADDIR = /uploads

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)
	rm -rf $(UPLOADROOT)/$(UPLOADDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re