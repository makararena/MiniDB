#pragma once
#include "database.h"

const std::string DATA_FOLDER = "./data";

// Saves a table to a file in the "data" folder.
void saveToFile(Database& db, const std::string& filename);

// Loads a table from a file in the "data" folder.
void loadFromFile(Database& db, const std::string& filename);
