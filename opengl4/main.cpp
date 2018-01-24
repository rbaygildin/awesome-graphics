/**
 * © Roman Yarnykh, 2017
 */

#include "app.h"

/**
 * Entry point for application
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {
    App::start(argc, argv);
    App::render();
    App::destroy();
    return 0;
}

