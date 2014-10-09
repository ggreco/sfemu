#include "sf.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <stdarg.h>
#include "md5.h"

static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";


static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

void SFGame::
login(const std::string &name, const std::string &pwd) {
    name_ = name;
    pwd_ = pwd;
    send_action(ACT_LOGIN, escape_string(name).c_str(),
            md5_pwd_ ? pwd.c_str() : MD5(pwd).hexdigest().c_str(), "v1.60", NULL);
}

std::string base64_decode(const std::string & encoded_string) {
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}

SFGame::
SFGame() :
    sessionid_("00000000000000000000000000000000"),
    endtimer_(0),
    savegame_("/"),
    status_(NotConnected),
    build_rank_(false),
    rank_position_(1),
    gambling_(None),
    bet_value_(1),
    verbose_(false),
    won_(0),
    loss_(0),
    maxbid_(0),
    plit_(players_.end()),
    md5_pwd_(false),
    work_only_(false)
{
}

bool SFGame::
connect(const std::string &host)
{
    lastcommand_ = ::time(NULL);
    
    std::string ip;
    if (struct hostent *h = gethostbyname(host.c_str())) {
        ip = inet_ntoa(*( struct in_addr*)(h->h_addr_list[0]));
    }
    else {
        std::cerr << "Unable to resolve name for " << host << "\n";
        return false;
    }

    bool rc = msg_.Connect(ip, 80);

    if (rc) {
        msg_.ServerName(host);
        status_ = Connected;
    }

    return rc;
}

void SFGame::send_action(int act, ...)
{
    std::ostringstream os;
    os << "request.php?req=" << sessionid_;
    os << std::setw(3) << std::setfill('0') << act;

    va_list ap;
    va_start(ap, act);
   
    if (char *c = va_arg(ap, char *)) {
        do {
            os << c;

            if (c = va_arg(ap, char *))
                os << "%3B";
        } while(c != NULL);
    }

    va_end(ap);
    os << "&random=%2&rnd=" 
        << std::setw(8) << std::setfill('0') << (::rand() % 100000000)
        << std::setw(8) << std::setfill('0') << (::rand() % 100000000)
        << std::setw(6) << std::setfill('0') << (::rand() % 1000000)
        ;
    if (verbose_)
        std::cerr << "Asking <" << os.str() << ">\n";

    msg_.AskData(os.str());
    lastcommand_ = ::time(NULL);
}

char rotate_char[4] = { '|', '/', '-', '\\' };

void SFGame::
resume_action(bool flag)
{
    time_t action_end = to_int(savegame_[SG_ACTION_ENDTIME]);
    time_t server_time = to_int(savegame_[SG_SERVER_TIME]);

    if (server_time == 0)
        ::time(&server_time);

    long delta = action_end - server_time;

    if (delta > 0) {
        selected_quest_length_ = delta;
        wait_action_completion(flag);
    }
    else {
        std::cerr << "Could not figure action end time!!!\n";
        ::sleep(1);
        ask_char_screen();
    }
}

bool SFGame::
check_pending()
{
    if (savegame_[SG_ACTION_STATUS] == "1") {
        std::cerr << "\nDetected working session in progress...\n";
        resume_action(false);
        return true;
    }
    else if (savegame_[SG_ACTION_STATUS] == "2") {
        std::cerr << "\nDetected quest in progress...\n";
        resume_action(true);
        return true;
    }
    return false;
}

void SFGame::
dump_rank()
{
    std::cerr << "\nActual Rank (" << players_.size() << " players)\n\n";

    for (std::map<std::string, Player>::const_iterator it = players_.begin(); 
                                                       it != players_.end(); ++it) 
        std::cerr << it->first << " (" << it->second.guild << ", " << it->second.level << " lvl)\n";

    std::cerr << '\n';
}

void SFGame::
rebuild_album(const std::string &src)
{
    std::string decoded = base64_decode(src);
   
//    std::cerr << "Decoded string size: " << decoded.size() << '\n';

    album_.clear();
    album_.reserve(decoded.length() * 8);

    for (size_t i = 0; i < decoded.length(); ++i) {
        unsigned char d = decoded[i];
//        fprintf(stderr, "%02x", (int)i);
        for (int j = 128; j > 0; j /= 2) {
            if (d & j)
                album_.push_back(true);
            else
                album_.push_back(false);
        }
    }
    std::cerr << '\n';

    check_missing();
}

void SFGame::
SetAlbumEpic(int val, char t, int type, int offset, int cl)
{
    if (!album_[val]) {
        int id = ITM_OFFS + type  * C_ITEMS_PER_TYPE * 15 + offset * 15 + cl;
        std::cerr << t << id << ' ';
        missing_.insert(id);
    }
}

void SFGame::
SetAlbumItems(int val, char t, int type, int offset, int cl)
{
    for (int j = 0; j < 5; ++j)
        if (!album_[val + j]) {
            int id = ITM_OFFS + type  * C_ITEMS_PER_TYPE * 15 + offset * 15 + cl + j * 3;
            std::cerr << t << id << ' ';
            missing_.insert(id);
        }
}

void SFGame::
check_missing()
{
// monsters, 62 pages
    std::cerr << "\nMissing monsters: ";
    for (int page = 0; page < 62; ++page) {
        for (int i = 0; i < 4; ++i)
            if (!album_[page * 4 + i])
                 std::cerr << (page * 4 + i) << ' ';
    }

    missing_.clear();

 // jewels, 25 pages
    std::cerr << "\n\nMissing jewels (n/necklace, r/ring, t/trinket):\n";
    for (int page = 0; page <= 25; ++page) {
        for (int i = 0; i < 4; ++i) {
            if (page <= 5) {
                if (page < 5 || i == 0) // nell'ultima pagina ce n'e' solo 1                    
                    SetAlbumItems(300 + page * 20 + i * 5, 'n', 8, 1 + page * 4 + i, 0);
            }
            else if (page <= 7)
            {
                SetAlbumEpic(510 + (page - 6) * 4 + i, 'N', 8, 50 + (page - 6) * 4 + i, 0);
            }
            else if (page <= 11)
            {
                SetAlbumItems(526 + (page - 8) * 20 + i * 5, 'r', 9, 1 + (page - 8) * 4 + i, 0);
            }
            else if (page <= 13)
            {
               SetAlbumEpic(686 + (page - 12) * 4 + i, 'R', 9, 50 + (page - 12) * 4 + i, 0);
            }
            else if (page <= 23)
            {
                if (page < 23 || i == 0) // nell'ultima pagina solo 1...
                   SetAlbumEpic(702 + (page - 14) * 4 + i, 't', 10, 1 + (page - 14) * 4 + i, 0);
            }
            else if (page <= 25)
            {
                SetAlbumEpic(760 + 16 + (page - 24) * 4 + i, 'T', 10, 50 + (page - 24) * 4 + i, 0);
            }
        }
    }
    std::cerr << "\n\nMissing warrior (w/weapon, s/shield, a/body, b/boots, g/gloves, h/helms, l/belt):\n";
    for (int page = 0; page <= 39; ++page) {
        for (int i = 0; i < 4; ++i) {
            if (page <= 7) {
                if (page < 7 || i <= 1) 
                    SetAlbumItems(776 + 16 + page * 20 + i * 5, 'w', 1, 1 + page * 4 + i, 1);
            }
            else if (page <= 9)
            {
                if (!album_[1076 + 16 + (page - 8) * 4 + i])
                        SetAlbumEpic(1076 + 16 + (page - 8) * 4 + i, 'W', 1, 50 + (page - 8) * 4 + i, 1);
            }
            else if (page <= 12)
            {
                if (page < 12 || i <= 1)
                    SetAlbumItems(1092 + 16 + (page - 10) * 20 + i * 5, 's', 2, 1 + (page - 10) * 4 + i, 1);
            }
            else if (page <= 14)
            {
                SetAlbumEpic(1192 + 16 + (page - 13) * 4 + i, 'S', 2, 50 + (page - 13) * 4 + i, 1);
            }
            else if (page <= 17)
            {
                if (page < 17 || i <= 1)
                    SetAlbumItems(1208 + 16 + (page - 15) * 20 + i * 5, 'a', 3, 1 + (page - 15) * 4 + i, 1);                
            }
            else if (page <= 19)
            {
                SetAlbumEpic(1308 + 16 + (page - 18) * 4 + i, 'A', 3, 50 + (page - 18) * 4 + i, 1);
            }
            else if (page <= 22)
            {
                if (page < 22 || i <= 1)
                    SetAlbumItems(1324 + 16 + (page - 20) * 20 + i * 5, 'b', 4, 1 + (page - 20) * 4 + i, 1);
            }
            else if (page <= 24)
            {
                SetAlbumEpic(1424 + 16 + (page - 23) * 4 + i, 'B', 4, 50 + (page - 23) * 4 + i, 1);
            }
            else if (page <= 27)
            {
                if (page < 27 || i <= 1)
                    SetAlbumItems(1440 + 16 + (page - 25) * 20 + i * 5, 'g', 5, 1 + (page - 25) * 4 + i, 1);
            }
            else if (page <= 29)
            {
                SetAlbumEpic(1540 + 16 + (page - 28) * 4 + i, 'G', 5, 50 + (page - 28) * 4 + i, 1);
            }
            else if (page <= 32)
            {
                if (page < 32 || i <= 1)
                    SetAlbumItems(1556 + 16 + (page - 30) * 20 + i * 5, 'h', 6, 1 + (page - 30) * 4 + i, 1);
            }
            else if (page <= 34)
            {
                SetAlbumEpic(1656 + 16 + (page - 33) * 4 + i, 'H', 6, 50 + (page - 33) * 4 + i, 1);
            }
            else if (page <= 37)
            {
                if (page < 37 || i <= 1)
                    SetAlbumItems(1672 + 16 + (page - 35) * 20 + i * 5, 'l', 7, 1 + (page - 35) * 4 + i, 1);
            }
            else if (page <= 39)
            {
                SetAlbumEpic(1772 + 16 + (page - 38) * 4 + i, 'L', 7, 50 + (page - 38) * 4 + i, 1);
            }
        }
    }

    for (int albumCat = 3; albumCat <= 4; ++albumCat) {
        if (albumCat == 3)
            std::cerr << "\n\nMissing mage (w/weapon, a/body, b/boots, g/gloves, h/helms, l/belt):\n";
        else
            std::cerr << "\n\nMissing hunter (w/weapon, a/body, b/boots, g/gloves, h/helms, l/belt):\n";

        int hunterOffs = (albumCat == 3 ? (0) : (696)) + 16;

        for (int page = 0; page <= 29; ++page) {
            for (int i = 0; i < 4; ++i) {
                if (page <= 2)
                {
                    if (page < 2 || i <= 1)
                        SetAlbumItems(1788 + hunterOffs + page * 20 + i * 5, 'w', 1, 1 + page * 4 + i, (albumCat - 1));
                }
                else if (page <= 4)
                {
                    SetAlbumEpic(1888 + hunterOffs + (page - 3) * 4 + i, 'W', 1, 50 + (page - 3) * 4 + i, (albumCat - 1));
                }
                else if (page <= 7)
                {
                    if (page < 7 || i <= 1)
                        SetAlbumItems(1904 + hunterOffs + (page - 5) * 20 + i * 5, 'a', 3, 1 + (page - 5) * 4 + i, (albumCat - 1));
                }
                else if (page <= 9)
                {
                    SetAlbumEpic(2004 + hunterOffs + (page - 8) * 4 + i, 'A', 3, 50 + (page - 8) * 4 + i, (albumCat - 1));
                }
                else if (page <= 12)
                {
                    if (page < 12 || i <= 1)
                        SetAlbumItems(2020 + hunterOffs + (page - 10) * 20 + i * 5, 'b', 4, 1 + (page - 10) * 4 + i, (albumCat - 1));
                }
                else if (page <= 14)
                {
                    SetAlbumEpic(2120 + hunterOffs + (page - 13) * 4 + i, 'B', 4, 50 + (page - 13) * 4 + i, (albumCat - 1));
                }
                else if (page <= 17)
                {
                    if (page < 17 || i <= 1)
                        SetAlbumItems(2136 + hunterOffs + (page - 15) * 20 + i * 5, 'g', 5, 1 + (page - 15) * 4 + i, (albumCat - 1));
                }
                else if (page <= 19)
                {
                    SetAlbumEpic(2236 + hunterOffs + (page - 18) * 4 + i, 'G', 5, 50 + (page - 18) * 4 + i, (albumCat - 1));
                }
                else if (page <= 22)
                {
                    if (page < 22 || i <= 1)
                        SetAlbumItems(2252 + hunterOffs + (page - 20) * 20 + i * 5, 'h', 6, 1 + (page - 20) * 4 + i, (albumCat - 1));
                }
                else if (page <= 24) {
                    SetAlbumEpic(2352 + hunterOffs + (page - 23) * 4 + i, 'H', 6, 50 + (page - 23) * 4 + i, (albumCat - 1));
                }
                else if (page <= 27)
                {
                    if (page < 27 || i <= 1)
                        SetAlbumItems(2368 + hunterOffs + (page - 25) * 20 + i * 5, 'l', 7, 1 + (page - 25) * 4 + i, (albumCat - 1));
                }
                else if (page <= 29)
                {
                    SetAlbumEpic(2468 + hunterOffs + (page - 28) * 4 + i, 'L', 7, 50 + (page - 28) * 4 + i, (albumCat - 1));
                }
            }
        }
    }
    std::cerr << "\n\nTotal missing items: " << missing_.size() << "\n\n";
}

void SFGame::
do_gambling()
{
    if (bet_value_ > maxbid_)
        maxbid_ = bet_value_;

    std::cerr << "Gambling mode: won " << won_ << " on " << (won_ + loss_);
    if ((won_ + loss_) > 0)
        std::cerr << ", " << (won_ * 100 / (won_ + loss_)) << "%, max bid " << maxbid_ << " units";
    if (gambling_ == Mushrooms) {
        std::cerr << ", bidding " << bet_value_  << " mushrooms...\n";
        place_mush_bet(bet_value_);
    } else if (gambling_ == Gold) {
        std::cerr << ", bidding " << (bet_value_ * 5) << " gold...\n";
        place_gold_bet(bet_value_ * 5);
    }
}

std::string SFGame::
gambling_stat() const
{
    std::ostringstream os;

    if (gambling_ == Gold)
        os << "gold: " << (to_int(savegame_[SG_GOLD]) / 100);
    else if (gambling_ == Mushrooms)
        os << "mush:" << savegame_[SG_MUSH];
    else
        os << "NO GAMBLING MODE";

    return os.str();
}

bool SFGame::
wait()
{
    std::string response;

    int rc = msg_.WaitMsg(2000, response);

    if (rc == HTTPMessage::TIMEOUT) {
        std::cerr << '.';
    }
    else if (rc == HTTPMessage::QUIT) {
        msg_.Disconnect();
        std::cerr << "\n\nConnection closed by foreign host\n\n";
        do {
            std::cerr << "Trying reconnect in 2 seconds...\n";
            ::sleep(2);
        }
        while(!connect(msg_.ServerName()));

        ask_char_screen();
    }
    else {
        if (!response.empty()) {
            if (verbose_)
                std::cerr << "Received <" << response << ">\n";

            int idx = 0;

            if (response[idx] == '+')
                    ++idx;

            if (response[idx] == 'E') {
                int err = atoi(&response[idx + 1]);
                std::cerr << "Error: " << err << "!\n";
    
                if (err > 0)
                    err = -err;

                switch (err) {
                    case ERR_SESSION_ID_EXPIRED:
                    case ERR_LOGIN_FAILED:
                        std::cerr << "\n*** Login failed, trying relogin!\n\n";
                        ::sleep(1);
                        msg_.Disconnect();
                        do {
                            std::cerr << "Trying reconnect in 2 seconds...\n";
                            ::sleep(2);
                        }
                        while(!connect(msg_.ServerName()));
                        
                        login(name_, pwd_);
                        break;
                    case ERR_INVENTORY_FULL:
                        std::cerr << "\n*** Inventory full, do something!\n\n";
                        break;
                }
            }
            else {
                CToken t(";", response); 

                switch(atoi(response.substr(idx, 3).c_str())) {
                    case RESP_PLAYER_SCREEN:
                        {
                            CToken t("/");
                            std::string to_tokenize = "/";
                            to_tokenize.append(&response[idx + 3]);

                            t.tokenize(to_tokenize);


                            std::cerr << " class " << t[SG_CLASS] << ": ";
                            for (int i = 0; i < 10; ++i) {
                                int p1 = to_int(t[SG_INVENTORY_OFFS + i * SG_ITM_SIZE + SG_ITM_TYP]);
                                int p2 = to_int(t[SG_INVENTORY_OFFS + i * SG_ITM_SIZE + SG_ITM_PIC]);
                                int l8 = 0, p4 = 0;
                                for (int l11 = 0; l11 < 8; ++l11)
                                    l8 += to_int(t[SG_INVENTORY_OFFS + i * SG_ITM_SIZE + SG_ITM_SCHADEN_MIN + l11]);
                                
                                l8 %= 5;
                                while (p2 >= 1000) {
                                    p2 -= 1000;
                                    p4 ++;
                                }
                                int itm = ITM_OFFS + p1 * C_ITEMS_PER_TYPE * 15 + p2 * 15 + p4 + l8 * 3;
                                if (itm != 0) {
                                    std::cerr << itm << ' ';

                                    if (missing_.find(itm) != missing_.end()) {
                                        std::cerr << "\n\n***Found missing item " << itm 
                                                  << " on PG " << plit_->first << " (lvl " << plit_->second.level << ")\n";
                                    }
                                }
                            }
                            std::cerr << '\n';


                            ++plit_;

                            if (plit_ != players_.end()) {
                                ::sleep(1);
                                ask_hall_screen(plit_->first);
                            }
                            else
                                std::cerr << "\n**** SEARCH COMPLETED ****\n\n";
                        }
                        break;
                    case ACT_SCREEN_EHRENHALLE:
                        {
                            CToken t("/");
                            t.tokenize(&response[idx + 3]);

                            ::sleep(1);

                            if (players_.size() < rank_size_) {
                                for (int i = 0; i < t.size(); i += 5) {
                                    if (t[i].empty() || t[i + 1].empty())
                                        continue;

                                    Player p(t[i + 1], // name
                                            t[i + 2], // guild
                                            t[i + 3], // livello
                                            t[i] // rank
                                            ); 

                                    players_[p.name] = p;
                                }
                                rank_position_ += t.size() / 5;

                                std::cerr << "Building characters DB, actual size " << players_.size() << "\n";
                                ask_hall_screen(rank_position_);
                            }
                            else {
                                if (plit_ == players_.end())
                                    plit_ = players_.begin();

                                if (to_lower(plit_->first) == to_lower(name_))
                                    plit_++;

                                std::cerr << "Querying " << plit_->first << "...";
                                ask_char(plit_->first);
                            }
//                            dump_rank();
                        }                        
                        break;
                    case RESP_BET_WON:
                        std::cerr << "*** We WON!!!, OLD " << gambling_stat();
                        parse(&response[idx + 3], true);
                        won_++;
                        std::cerr << ", NEW " << gambling_stat() << '\n'; 
                        ::sleep(2);
                        bet_value_ = 1;
                        do_gambling();
                        break;
                    case RESP_BET_LOST:
                        std::cerr << "*** We LOST :(, OLD " << gambling_stat();
                        loss_++;
                        parse(&response[idx + 3], true);
                        std::cerr << ", NEW " << gambling_stat() << '\n'; 
                        ::sleep(2);
                        bet_value_ *= 2;
                        do_gambling();
                        break;
                    case RESP_ALBUM:
                        rebuild_album(&response[idx + 3]);
                        break;
                    case RESP_LOGIN_SUCCESS:
                        parse(t[0]);
                        status_ = Logged;
                        sessionid_ = t[2];
                        std::cerr << "\nNew session ID <" << sessionid_ << ">\n";
                        ::sleep(1);
                        ask_char_screen();
                        break;
                    case ACT_SCREEN_ARBEITEN:
                        ::sleep(1);
                        if (!check_pending())
                            start_work();
                        break;
                    case RESP_ARBEIT_START:
                        wait_action_completion(false);
                        break;
                    case ACT_SCREEN_TAVERNE:
                    case RESP_ARBEIT_ERLEDIGT: // sembra risponda questo se si chiede tavern
                        ::sleep(1);

                        if (endtimer_) {
                            endtimer_ = 0;
                            ask_char_screen();
                        }
                        else if (!check_pending()) {
                            if (gambling_ != None)
                                do_gambling();
                            else if (work_only_ == false && to_int(savegame_[SG_TIMEBAR]) > 0)
                                select_new_quest();
                            else {
                                std::cerr << "\n*** No time available, preparing a working session...\n\n";
                                ask_work_screen();
                            }
                        }
                        break;
                    case RESP_SCREEN_BUILDCHAR:
                        parse(t[0]);
                        ::sleep(1);
                        ask_tavern_screen();
                        break;
                    case RESP_QUEST_START:
                        wait_action_completion(true);
                        break;
                    case RESP_QUEST_DONE:
                        status_ = Logged;
                        endtimer_ = 0;
                        ::sleep(1);
                        ask_char_screen();

                        break;

                }
            }
        }
        else
            std::cerr << '-';
        
        if ((lastcommand_ + 20) < ::time(NULL))
            ask_char_screen();

        if (endtimer_ > 0 && endtimer_ < ::time(NULL)) 
            ask_tavern_screen();

    }

    return true;
}

void SFGame::
wait_action_completion(bool quest)
{
    if (build_rank_) {
        ::sleep(1);
        ask_hall_screen(rank_position_);        
        return;
    }

    if (quest) {
        status_ = QuestStarted;
        endtimer_ = selected_quest_length_ + ::time(NULL);

        std::cerr << "\nQuest in progress...\n";
    }
    else {
        status_ = WorkStarted;
        endtimer_ = selected_quest_length_ + ::time(NULL);

        std::cerr << "\nWork in progress...\n";
    }

    msg_.Disconnect();
    int pos = 0;
    while (endtimer_ > ::time(NULL)) {
        pos ++;
        if (pos == 4)
            pos = 0;

        int delta = endtimer_ - ::time(NULL);
        std::cerr << rotate_char[pos] << " -> "
            << std::setw(2) << std::setfill('0') << (delta / 60) << ':'
            << std::setw(2) << std::setfill('0') << (delta % 60) << "      \r";
        ::usleep(500000);
    }
    std::cerr << "\n\n";

    do {
        ::sleep(1);
        std::cerr << "Reconnecting to " << msg_.ServerName() << "...\n";
    }
    while (!connect(msg_.ServerName()));
    std::cerr << "OK\n";
}

void SFGame::
select_new_quest()
{
    double bestvalue = 0.0;
    int best_quest = -1;

    for (int i = 0; i < 3; ++i) {
         double val = ((double)to_int(savegame_[SG_QUEST_OFFER_EXP1 + i])) / 
                      ((double)to_int(savegame_[SG_QUEST_OFFER_DURATION1 + i]));

         if (val > bestvalue) { 
             bestvalue = val;
             best_quest = i;
         }
    }
    selected_quest_length_ = to_int(savegame_[SG_QUEST_OFFER_DURATION1 + best_quest]);

    std::cerr << "\n*** Selected best quest " << best_quest + 1 
              << ", duration " << selected_quest_length_
              << ", exp " << savegame_[SG_QUEST_OFFER_EXP1 + best_quest] << "\n\n";
    quest_begin(best_quest);
}

void SFGame::
parse(const std::string &src, bool silent)
{
    savegame_.tokenize("0/" + src);

    if (silent)
        return;

    std::cerr <<
                 "\nLVL     : " << savegame_[SG_LEVEL] << 
                 " GOLD: "   << (to_int(savegame_[SG_GOLD])/100) <<
                 " Action: " << savegame_[SG_ACTION_STATUS] << '/' << savegame_[SG_ACTION_INDEX] << '/' << savegame_[SG_ACTION_ENDTIME] <<
                 "\nEXP     : " << savegame_[SG_EXP] << '/' << savegame_[SG_EXP_FOR_NEXTLEVEL] <<
                 " (" << savegame_[SG_TIMEBAR] << " time left)";

    for (int i = 0; i < 3; ++i) {
        std::cerr << "\nQUEST " << (i + 1) << " exp:" << savegame_[SG_QUEST_OFFER_EXP1 + i]
                  << " gold:" << (to_int(savegame_[SG_QUEST_OFFER_GOLD1 + i])/100)
                  << " len:" << savegame_[SG_QUEST_OFFER_DURATION1 + i] 
                  << " enemy:" << savegame_[SG_QUEST_OFFER_ENEMY1 + i] 
                  << " rwd:" << savegame_[SG_QUEST_OFFER_REWARD_ITM1 + i] ;
    }

    std::cerr << "\n";
}

