#pragma once
#include "database.h"

// Saves file to the data folder
void saveToFile(Database& db, const std::string& filename);

// Loads file from the data folder
void loadFromFile(Database& db, const std::string& filename);

// Deletes file from data folder
void deleteFile(Database& db, const std::string& command);
