#pragma once
#include "database.h"

void saveToFile(Database& db, const std::string& filename);
void loadFromFile(Database& db, const std::string& filename);
