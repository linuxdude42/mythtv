#ifndef MAIN_HELPERS_H
#define MAIN_HELPERS_H

// C++ headers
#include <iostream>
#include <fstream>

class MythBackendCommandLineParser;
class QString;
class QSize;

bool setupTVs(bool isPrimary, bool &error);
void cleanup(void);
int  handle_command(const MythBackendCommandLineParser &cmdline);
int  connect_to_primary(void);
void print_warnings(const MythBackendCommandLineParser &cmdline);
int  run_backend(MythBackendCommandLineParser &cmdline);

#endif // MAIN_HELPERS_H
