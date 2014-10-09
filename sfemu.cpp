#include <iostream>
#include "sf.h"
#include "version.h"

const char *album = "/v/f//////////////f//////v3X8AAA///8AAAAAAAAAAAAAA/////////////////4AAAAAAAAAAAAAAAAA/gD/////////////AAAAAAAAAAAAAP4A//////gAAAAAP4A3////////////////////////AAAAAAAAAAAAAAAAAAAAAAAAA/gD////////AAAAAAAAP4A////////wAAAAAAAD+AP///////8AAAAAAAA/gD////////AAAAAAAAP4A////////wAAAAAAAD+AP///////8AAAAAAAA/AD////////AAAAAAAAPwA////////wAAAAAAAD8AP///////8AAAAAAAA+gD////////AAAAAAAAPwA////////wAAAAAAAD4AP///////8AAAAAAAA/AD////////AAAAAAAAPwA////////wAAAAAAAD8AP///////8AAAAAAAA/AD////////AAAAAAAAPwA////////wAAAAAAAD8AP///////8AAAAAAAA/gAA==";

int main(int argc, char *argv[])
{
    std::cerr << "SFEmu V" << PROGRAM_VERSION << "\n";

    if (argc != 4 && argc != 5)  {
        std::cerr << "\n\nUsage:\n" << argv[0] << " [servername] [user] [password]\n";
        return -1;
    }

    SFGame game;

//    game.rebuild_album(album);
//    return 0;

    if (argc == 5)
        game.work_only(true);

    game.verbose(true);
//    game.md5_password(true);
    if (game.connect(argv[1])) {
        std::cerr << "Connected to " << argv[1] << '\n';

        game.login(argv[2], argv[3]);
       
        game.build_rank(false, 45000);
//        game.gambling_mode(SFGame::Gold);
//        game.verbose(false);

        while (game.wait());
    }
    else
        std::cerr << "Unable to connect to " << argv[1] << "\n";
}
