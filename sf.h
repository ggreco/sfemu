#ifndef SF_H

#define SF_H

#include "http_message.h"
#include "ctoken.h"
#include "utils.h"
#include <map>
#include <set>

inline std::string escape_string(const std::string &src)
{
    std::string result;
    for (std::string::const_iterator it = src.begin(); it != src.end(); ++it) {
        if (*it == ' ')
            result.append("%20");
        else
            result.push_back(*it);
    }
    return result;
}

class SFGame
{
    public:
        enum GamblingType { None, Mushrooms, Gold };
    private:        
        std::string name_;
        std::string pwd_;

        const static int ACT_ACCOUNT_CREATE = 1;
        const static int ACT_LOGIN = 2;
        const static int ACT_FORGOT_PASSWORD = 3;
        const static int ACT_CHAR_CREATE = 501;
        const static int ACT_ARBEIT = 502;
        const static int ACT_REQUEST_GUILD = 503;
        const static int ACT_INVENTORY_CHANGE = 504;
        const static int ACT_ARBEIT_CANCEL = 505;
        const static int ACT_REQUEST_NEWWAREZ = 506;
        const static int ACT_POST_READ = 507;
        const static int ACT_POST_DELETE = 508;
        const static int ACT_POST_SEND = 509;
        const static int ACT_QUEST_BEGIN = 510;
        const static int ACT_QUEST_CANCEL = 511;
        const static int ACT_START_FIGHT = 512;
        const static int ACT_REQUEST_CHAR = 513;
        const static int ACT_SET_PLAYER_DESC = 514;
        const static int ACT_RE_LOGIN = 515;
        const static int ACT_SEND_CHAT = 516;
        const static int ACT_GET_CHAT_HISTORY = 517;
        const static int ACT_BUY_BEER = 518;
        const static int ACT_MAINQUEST = 519;
        const static int ACT_REQUEST_TRANS_COUNT = 530;
        const static int ACT_RESEND_EMAIL = 531;
        const static int ACT_VALIDATE = 532;
        const static int ACT_REQUEST_GUILD_NAMES = 533;
        const static int ACT_REVOLT = 534;
        const static int ACT_LOGOUT = 535;
        const static int ACT_POST_SEND_GUILD = 536;
        const static int ACT_WHISPER = 537;
        const static int ACT_SCREEN_CHAR = 4;
        const static int ACT_SCREEN_POST = 5;
        const static int ACT_SCREEN_GILDEN = 6;
        const static int ACT_SCREEN_EHRENHALLE = 7;
        const static int ACT_SCREEN_WELTKARTE = 8;
        const static int ACT_SCREEN_OPTIONEN = 9;
        const static int ACT_SCREEN_TAVERNE = 10;
        const static int ACT_SCREEN_ARENA = 11;
        const static int ACT_SCREEN_ARBEITEN = 12;
        const static int ACT_SCREEN_SCHMIEDE = 13;
        const static int ACT_SCREEN_ZAUBERLADEN = 14;
        const static int ACT_SCREEN_STALL = 15;
        const static int ACT_SCREEN_PILZDEALER = 16;
        const static int ACT_SCREEN_GILDE_GRUENDEN = 17;
        const static int ACT_BUY_MOUNT = 20;
        const static int ACT_BUY_ATTRIB = 21;
        const static int ACT_PLACE_BET = 22;
        const static int ACT_SCREEN_FREMDGILDE = 23;
        const static int ACT_SCREEN_GILDENHALLE = 24;
        const static int ACT_DEALER_AKTION = 25;
        const static int ACT_DEALER_SPONSOR = 26;
        const static int ACT_KILL_POTION = 27;
        const static int ACT_GUILD_FOUND = 101;
        const static int ACT_GUILD_DELETE = 102;
        const static int ACT_GUILD_INVITE = 103;
        const static int ACT_GUILD_EXPEL = 104;
        const static int ACT_GUILD_SET_MASTER = 105;
        const static int ACT_GUILD_SET_OFFICER = 106;
        const static int ACT_GUILD_IMPROVE = 107;
        const static int ACT_GUILD_SET_DESC = 108;
        const static int ACT_GUILD_RENAME = 109;
        const static int ACT_GUILD_JOIN = 110;
        const static int ACT_GUILD_DONATE = 111;
        const static int ACT_GUILD_JOIN_ATTACK = 112;
        const static int ACT_GUILD_JOIN_DEFENSE = 113;
        const static int ACT_GUILD_COMMENCE_ATTACK = 114;
        const static int ACT_INVITE_PLAYER = 115;
        const static int ACT_ALBUM = 116;
        const static int RESP_ACCOUNT_SUCCESS = 1;
        const static int RESP_LOGIN_SUCCESS = 2;
        const static int RESP_FAME_LIST = 3;
        const static int RESP_SCREEN_BUILDCHAR = 4;
        const static int RESP_GUILD_DATA = 101;
        const static int RESP_SAVEGAME_STAY = 102;
        const static int RESP_ARBEIT_ERLEDIGT = 103;
        const static int RESP_ARBEIT_START = 104;
        const static int RESP_ARBEIT_STOP = 105;
        const static int RESP_QUEST_DONE = 106;
        const static int RESP_QUEST_START = 107;
        const static int RESP_QUEST_STOP = 108;
        const static int RESP_PLAYER_DESC_SUCCESS = 109;
        const static int RESP_PLAYER_SCREEN = 111;
        const static int RESP_PLAYER_NOT_FOUND = 112;
        const static int RESP_DEMO_SCREEN = 113;
        const static int RESP_ATTACK_NOT_EXIST = 114;
        const static int RESP_CHANGE_FACE_OK = 115;
        const static int RESP_CHANGE_PASS_OK = 116;
        const static int RESP_CHANGE_NAME_OK = 117;
        const static int RESP_CHANGE_MAIL_OK = 118;
        const static int RESP_DELETE_ACCOUNT_OK = 119;
        const static int RESP_NO_LOGIN = 120;
        const static int RESP_REQUEST_GUILD = 121;
        const static int RESP_MAINQUEST = 122;
        const static int RESP_GUILD_FOUND_SUCCESS = 150;
        const static int RESP_GUILD_DELETE_SUCCESS = 151;
        const static int RESP_GUILD_RENAME_SUCCESS = 152;
        const static int RESP_GUILD_CHANGE_DESC_SUCCESS = 153;
        const static int RESP_GUILD_IMPROVE_SUCCESS = 154;
        const static int RESP_GUILD_OFFICER_SUCCESS = 155;
        const static int RESP_GUILD_EXPEL_SUCCESS = 156;
        const static int RESP_GUILD_INVITE_SUCCESS = 157;
        const static int RESP_GUILD_JOIN_SUCCESS = 158;
        const static int RESP_GUILD_MASTER_SUCCESS = 159;
        const static int RESP_GUILD_DONATE_SUCCESS = 160;
        const static int RESP_CHAT_HISTORY = 161;
        const static int RESP_CHAT_LINE = 162;
        const static int RESP_TRANS_COUNT = 163;
        const static int RESP_EMAIL_RESENT = 164;
        const static int RESP_PASSWORD_SENT = 165;
        const static int RESP_VALIDATE_OK = 166;
        const static int RESP_UPDATE_CHECK = 167;
        const static int RESP_BET_WON = 168;
        const static int RESP_BET_LOST = 169;
        const static int RESP_SCREEN_GILDENHALLE = 170;
        const static int RESP_OTHER_GUILD = 172;
        const static int RESP_SAVEGAME_STAY_ERROR = 173;
        const static int RESP_DEALER_AKTION = 174;
        const static int ERR_DEALER_AKTION = 175;
        const static int RESP_DEALER_SPONSOR = 176;
        const static int ERR_DEALER_SPONSOR = 177;
        const static int RESP_GUILD_FIGHT = 178;
        const static int RESP_GUILD_JOIN_ATTACK_OK = 179;
        const static int RESP_GUILD_JOIN_DEFENSE_OK = 180;
        const static int RESP_GUILD_COMMENCE_ATTACK_OK = 181;
        const static int RESP_GUILD_NAMES = 183;
        const static int RESP_LOGIN_SUCCESS_BOUGHT = 184;
        const static int RESP_REQUEST_GUILD_QUIET = 186;
        const static int RESP_LOGOUT_SUCCESS = 187;
        const static int RESP_QUEST_DONE_PIXEL = 188;
        const static int ACT_QUEST_SKIP = 189;
        const static int RESP_WHISPER_SUCCESS = 190;
        const static int RESP_INVITE_SUCCESS = 191;
        const static int RESP_ALBUM = 192;
        const static int RESP_QUEST_SKIP_ALLOWED = 193;
        const static int RESP_QUEST_SKIP_ALLOWED_START = 194;
        const static int RESP_READ_MESSAGE = 201;
        const static int RESP_MESSAGE_SENT = 202;
        const static int ERR_INBOX_FULL = 203;
        const static int ERR_RECIPIENT_NOT_FOUND = 204;
        const static int ERR_RECIPIENT_SELF = 205;
        const static int ACT_CHANGE_FACE = 801;
        const static int ACT_DELETE_ACCOUNT = 802;
        const static int ACT_CHANGE_NAME = 803;
        const static int ACT_CHANGE_MAIL = 804;
        const static int ACT_CHANGE_PASS = 805;
        const static int ERR_NAME_EXISTS = -1;
        const static int ERR_NAME_TOO_SHORT = -2;
        const static int ERR_PASSWORD_TOO_SHORT = -3;
        const static int ERR_EMAIL_REJECTED = -4;
        const static int ERR_NAME_REJECTED = -5;
        const static int ERR_LOGIN_FAILED = -6;
        const static int ERR_TOO_EXPENSIVE = -7;
        const static int ERR_WRONG_PASSWORD = -8;
        const static int ERR_FACE_DATA_INCORRECT = -9;
        const static int ERR_EMAIL_WRONG = -10;
        const static int ERR_GENDER_OR_RACE = -11;
        const static int ERR_MAIL_EXISTS = -12;
        const static int ERR_ALREADY_IN_GUILD = -13;
        const static int ERR_NO_INDEX_FREE = -14;
        const static int ERR_FIGHT_SELF = -15;
        const static int ERR_GUILD_NOT_FOUND = -16;
        const static int ERR_GUILD_NOT_ALLOWED = -17;
        const static int ERR_GUILD_LACK_MUSH = -18;
        const static int ERR_GUILD_LACK_GOLD = -19;
        const static int ERR_GUILD_BUILDING_NOT_FOUND = -20;
        const static int ERR_GUILD_BUILDING_MAX = -21;
        const static int ERR_GUILD_NOT_MEMBER = -22;
        const static int ERR_GUILD_MASTER_CANT_BE_OFFICER = -23;
        const static int ERR_GUILD_IS_FULL = -24;
        const static int ERR_GUILD_ALREADY_YOU_OTHER = -25;
        const static int ERR_GUILD_NOT_REAL_MEMBER = -26;
        const static int ERR_GUILD_ALREADY_YOU_THIS = -27;
        const static int ERR_GUILD_PLAYER_NOT_FOUND = -28;
        const static int ERR_SUBJECT_TOO_SHORT = -29;
        const static int ERR_GUILD_TOO_EXPENSIVE = -30;
        const static int ERR_GUILD_CHAT_NOT_MEMBER = -31;
        const static int ERR_GUILD_CHAT_HISTORY = -32;
        const static int ERR_GUILD_CHAT_TEXT_ERROR = -33;
        const static int ERR_BEER = -34;
        const static int ERR_BOOST = -42;
        const static int ERR_GUILD_NAME_REJECTED = -43;
        const static int ERR_GUILD_NAME_LENGTH = -44;
        const static int ERR_GUILD_NAME_CHARACTERS = -45;
        const static int ERR_GUILD_EMAIL_VALIDATE = -46;
        const static int ERR_GUILD_MUSH_FREE = -47;
        const static int ERR_ATTACK_AGAIN = -48;
        const static int ERR_REQUEST_PW = -49;
        const static int ERR_VALIDATE = -50;
        const static int ERR_NO_MUSH_BAR = -35;
        const static int ERR_NO_ENDURANCE = -36;
        const static int ERR_WORSE_MOUNT = -37;
        const static int ERR_GUILD_ALREADY_MEMBER = -38;
        const static int ERR_NOT_INVITED = -39;
        const static int ERR_NO_MUSH_PVP = -40;
        const static int ERR_NO_MUSH_MQ = -41;
        const static int ERR_GUILD_DONATE_NEG = -51;
        const static int ERR_GUILD_DONATE_FRA = -52;
        const static int ERR_LOCKED_PAYMENT = -53;
        const static int ERR_LOCKED_ADMIN = -54;
        const static int ERR_TOO_SOON = -55;
        const static int ERR_ACCOUNTS_PER_IP = -56;
        const static int ERR_PLACE_BET = -57;
        const static int ERR_INVENTORY_FULL = -58;
        const static int ERR_GUILD_FIGHT_TOO_EXPENSIVE = -60;
        const static int ERR_GUILD_ALREADY_UNDER_ATTACK = -61;
        const static int ERR_GUILD_ATTACK_DELAY = -62;
        const static int ERR_GUILD_ALREADY_ATTACKING = -63;
        const static int ERR_GUILD_ATTACK_STATUS = -64;
        const static int ERR_SESSION_ID_EXPIRED = -65;
        const static int ERR_STOP_TUNNELING = -66;
        const static int ERR_REVOLT_FAILED = -67;
        const static int ERR_JOINED_TOO_RECENTLY = -68;
        const static int ERR_SERVER_DOWN = -69;
        const static int ERR_MSG_LEVEL_TOO_LOW = -84;
        const static int ERR_MSG_NOT_VALIDATED = -85;
        const static int ERR_INVENTORY_FULL_ADV = -86;
        const static int ERR_INVITE_NOT_VALIDATED = -90;
        const static int ERR_INVITE_TOO_MANY = -91;
        const static int ERR_INVITE_EMAIL_REJECTED = -92;
        const static int ERR_NO_ALBUM = -93;
        const static int SG_ITM_SIZE = 12;
        const static int SG_BACKPACK_SIZE = 5;
        const static int SG_INVENTORY_SIZE = 10;
        const static int SG_ITM_TYP = 0;
        const static int SG_ITM_PIC = 1;
        const static int SG_ITM_SCHADEN_MIN = 2;
        const static int SG_ITM_SCHADEN_MAX = 3;
        const static int SG_ITM_ATTRIBTYP1 = 4;
        const static int SG_ITM_ATTRIBTYP2 = 5;
        const static int SG_ITM_ATTRIBTYP3 = 6;
        const static int SG_ITM_ATTRIBVAL1 = 7;
        const static int SG_ITM_ATTRIBVAL2 = 8;
        const static int SG_ITM_ATTRIBVAL3 = 9;
        const static int SG_ITM_GOLD = 10;
        const static int SG_ITM_MUSH = 11;
        const static int SG_PAYMENT_ID = 1;
        const static int SG_PLAYER_ID = 2;
        const static int SG_LAST_ACTION_DATE = 3;
        const static int SG_REGISTRATION_DATE = 4;
        const static int SG_REGISTRATION_IP = 5;
        const static int SG_MSG_COUNT = 6;
        const static int SG_VALIDATION_IP = 7;
        const static int SG_LEVEL = 8;
        const static int SG_EXP = 9;
        const static int SG_EXP_FOR_NEXTLEVEL = 10;
        const static int SG_HONOR = 11;
        const static int SG_RANK = 12;
        const static int SG_CLASS_RANK = 13;
        const static int SG_GOLD = 14;
        const static int SG_MUSH = 15;
        const static int SG_MUSH_GAINED = 16;
        const static int SG_MUSH_SPENT = 17;
        const static int SG_FACE_1 = 18;
        const static int SG_FACE_2 = 19;
        const static int SG_FACE_3 = 20;
        const static int SG_FACE_4 = 21;
        const static int SG_FACE_5 = 22;
        const static int SG_FACE_6 = 23;
        const static int SG_FACE_7 = 24;
        const static int SG_FACE_8 = 25;
        const static int SG_FACE_9 = 26;
        const static int SG_FACE_10 = 27;
        const static int SG_RACE = 28;
        const static int SG_GENDER = 29;
        const static int SG_CLASS = 30;
        const static int SG_ATTR_STAERKE = 31;
        const static int SG_ATTR_BEWEGLICHKEIT = 32;
        const static int SG_ATTR_AUSDAUER = 33;
        const static int SG_ATTR_INTELLIGENZ = 34;
        const static int SG_ATTR_WILLENSKRAFT = 35;
        const static int SG_ATTR_STAERKE_BONUS = 36;
        const static int SG_ATTR_BEWEGLICHKEIT_BONUS = 37;
        const static int SG_ATTR_AUSDAUER_BONUS = 38;
        const static int SG_ATTR_INTELLIGENZ_BONUS = 39;
        const static int SG_ATTR_WILLENSKRAFT_BONUS = 40;
        const static int SG_ATTR_STAERKE_GEKAUFT = 41;
        const static int SG_ATTR_BEWEGLICHKEIT_GEKAUFT = 42;
        const static int SG_ATTR_AUSDAUER_GEKAUFT = 43;
        const static int SG_ATTR_INTELLIGENZ_GEKAUFT = 44;
        const static int SG_ATTR_WILLENSKRAFT_GEKAUFT = 45;
        const static int SG_ACTION_STATUS = 46;
        const static int SG_ACTION_INDEX = 47;
        const static int SG_ACTION_ENDTIME = 48;
        const static int SG_INVENTORY_OFFS = 49;
        const static int SG_BACKPACK_OFFS = 169;
        const static int SG_QUEST_REROLL_TIME = 229;
        const static int SG_QUEST_OFFER_LEVEL1 = 230;
        const static int SG_QUEST_OFFER_TYPE1 = 233;
        const static int SG_QUEST_OFFER_ENEMY1 = 236;
        const static int SG_QUEST_OFFER_LOCATION1 = 239;
        const static int SG_QUEST_OFFER_DURATION1 = 242;
        const static int SG_QUEST_OFFER_REWARD_ITM1 = 245;
        const static int SG_QUEST_OFFER_EXP1 = 281;
        const static int SG_QUEST_OFFER_GOLD1 = 284;
        const static int SG_MOUNT = 287;
        const static int SG_SHAKES_REROLL_TIME = 288;
        const static int SG_SHAKES_ITEM1 = 289;
        const static int SG_FIDGET_REROLL_TIME = 361;
        const static int SG_FIDGET_ITEM1 = 362;
        const static int SG_NEXT_BATTLE_TIME = 434;
        const static int SG_UNREAD_MESSAGES = 435;
        const static int SG_GUILD_INDEX = 436;
        const static int SG_GUILD_RANK = 437;
        const static int SG_MUSHROOMS_MAY_DONATE = 438;
        const static int SG_ALBUM = 439;
        const static int SG_LAST_GUILD_FIGHT_EXP = 440;
        const static int SG_ACCOUNT_PROTECTION_DATE = 441;
        const static int SG_NEW_DUNGEONS = 442;
        const static int SG_GUILD_JOIN_DATE = 444;
        const static int SG_ARCHIVE_NEW_CALC = 445;
        const static int SG_MUSH_BOUGHT_SINCE_LAST_LOGIN = 446;
        const static int SG_WE_MISS_YOU = 447;
        const static int SG_ARMOR = 448;
        const static int SG_DAMAGE_MIN = 449;
        const static int SG_DAMAGE_MAX = 450;
        const static int SG_LIFE = 451;
        const static int SG_MOUNT_DURATION = 452;
        const static int SG_TRANSACTION_COUNT = 453;
        const static int SG_EVASION = 454;
        const static int SG_MAGICRSISTANCE = 455;
        const static int SG_TIMEBAR_REROLL_TIME = 456;
        const static int SG_TIMEBAR = 457;
        const static int SG_BEERS = 458;
        const static int SG_MQ_STATE = 459;
        const static int SG_MQ_REROLL_TIME = 460;
        const static int SG_PVP_REROLL_TIME = 461;
        const static int SG_EXP_BONUS = 462;
        const static int SG_GOLD_BONUS = 463;
        const static int SG_EMAIL_VALID = 464;
        const static int SG_EMAIL_DATE = 465;
        const static int SG_ACHIEVEMENTS = 466;
        const static int SG_LOCKDURATION = 476;
        const static int SG_FOO = 477;
        const static int SG_BAR = 478;
        const static int SG_HELLO = 479;
        const static int SG_FIRST_PAYMENT = 480;
        const static int SG_DUNGEON_LEVEL = 481;
        const static int SG_DUNGEON_13 = 491;
        const static int SG_LAST_DUNGEON = 492;
        const static int SG_PHP_SESSION = 493;
        const static int SG_POTION_TYPE = 494;
        const static int SG_POTION_DURATION = 497;
        const static int SG_POTION_GAIN = 500;
        const static int SG_POWER_LIFE_POTION = 503;
        const static int SG_LAST_LOGIN_IP = 504;
        const static int SG_MUSHROOM_BOUGHT_AMOUNT = 507;
        const static int SG_MUSHROOM_BOUGHT_DATE = 508;
        const static int SG_GUILD_FIGHT_STATUS = 509;
        const static int SG_EVENT_TRIGGER_COUNT = 510;
        const static int SG_SERVER_TIME = 511;
        const static int ITM_OFFS = 451;
        const static int C_ITEMS_PER_TYPE = 100;

        enum Status { NotConnected, Connected, Logged, QuestStarted, StartingQuest, WorkStarted, StartingWork};
        HTTPMessage msg_;
        std::string sessionid_;
        CToken savegame_;
        time_t endtimer_;
        time_t lastcommand_;
        int selected_quest_length_;
        Status status_;
        bool build_rank_;
        int rank_position_;
        GamblingType gambling_;
        int bet_value_;
        bool verbose_;
        int won_, loss_, maxbid_;
        bool work_only_;

        void parse(const std::string &, bool silent = false);
        void select_new_quest();
        void resume_action(bool);
        bool check_pending();
        void wait_action_completion(bool quest);
        void do_gambling();
        std::string gambling_stat() const;

        struct Player {
            std::string name;
            std::string guild;
            int level;
            int rank;

            Player() : level(-1), rank(-1) {}

            Player(const std::string &n,
                   const std::string &g,
                   const std::string &l,
                   const std::string &r) :
                name(n), guild(g) {
                if (l[0] == '-')
                    level = to_int(&l[1]);
                else
                    level = to_int(l);
                if (r[0] == '-')
                    rank = to_int(&r[1]);
                else
                    rank = to_int(r);
            }
        };

        std::map<std::string, Player> players_;
        std::vector<bool> album_;
        std::set<int> missing_;
        int rank_size_;
        std::map<std::string, Player>::const_iterator plit_;
        bool md5_pwd_;
           
        void check_missing();
        void SetAlbumEpic(int, char, int type, int offset, int cl);
        void SetAlbumItems(int, char, int type, int offset, int cl);
    public:
        void md5_password(bool flag) { md5_pwd_ = flag; }
        void rebuild_album(const std::string &);
        void send_action(int action, ...);
        SFGame();
        void work_only(bool val) { work_only_ = val; }

        void verbose(bool flag) { verbose_ = flag; }
        void build_rank(bool flag, int ranksize = 2000) { build_rank_ = flag; rank_size_ = ranksize; }
        void gambling_mode(GamblingType flag) { gambling_ = flag; }
        bool connect(const std::string &host);
        void login(const std::string &name, const std::string &pwd);
        void start_work(int hours = 1) {
            status_ = StartingWork;
            selected_quest_length_ = 3600 * hours;
            send_action(ACT_ARBEIT, to_string(hours).c_str(), NULL);
        }
        void fight_battle(const std::string &opponent) {
            send_action(ACT_START_FIGHT, escape_string(opponent).c_str(), NULL);
        }
        void ask_tavern_screen() {
            send_action(ACT_SCREEN_TAVERNE, NULL);
        }
        void ask_work_screen() {
            send_action(ACT_SCREEN_ARBEITEN, NULL);
        }
        void quest_begin(int quest /* 0, 1, 2 */) {
            status_ = StartingQuest;
            send_action(ACT_QUEST_BEGIN, to_string(quest + 1).c_str(), "0", NULL);
        }
        void ask_char_screen() {
            send_action(ACT_SCREEN_CHAR, NULL);
        }
        void ask_hall_screen(int position = 1) {
            send_action(ACT_SCREEN_EHRENHALLE, "", to_string(position).c_str(), NULL);
        }
        void ask_hall_screen(const std::string &name) {
            send_action(ACT_SCREEN_EHRENHALLE, escape_string(name).c_str(), "-1", NULL);
        }
        void ask_char(const std::string &name) {
            send_action(ACT_REQUEST_CHAR, escape_string(name).c_str(), NULL);
        }
        void ask_album() {
            send_action(ACT_ALBUM, NULL);
        }
        void place_gold_bet(int gold) {
            send_action(ACT_PLACE_BET, to_string(gold * 100).c_str(), NULL);
        }
        void place_mush_bet(int mushrooms) {
            send_action(ACT_PLACE_BET, to_string(mushrooms).c_str(), NULL);
        }

        void dump_rank();
        bool wait();
};

#endif
