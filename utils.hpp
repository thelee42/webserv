#pragma once

#include "includes.hpp"

std::string		ft_tostring(int value);
std::size_t		skipSpaces(const std::string& line, std::size_t pos);

int				ft_back(std::string tmp, char occ);
int				empty_line(std::string line);
int				toInt(const std::string &s);

void			ft_popback(std::string &tmp);

bool			isValidPath(const std::string& path);
