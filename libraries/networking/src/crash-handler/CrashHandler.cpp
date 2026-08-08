//
//  CrashHandler.cpp
//
//
//  Created by Dale Glass on 25/06/2023.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "CrashHandler.h"
#include "CrashHandlerNone.h"
#include "CrashHandlerSentry.h"

#include <QFileInfo>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QRandomGenerator>


#include "LogHandler.h"

#ifdef Q_OS_LINUX
#include <unistd.h>
#endif



Q_LOGGING_CATEGORY(crash_handler, "overte.crash_handler")


static void crashHandlerLogMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);


// https://www.eff.org/files/2016/09/08/eff_short_wordlist_1.txt
const QStringList CrashHandler::EFF_SHORT_WORDLIST = {
"acid", "acorn", "acre", "acts", "afar", "affix", "aged", "agent", "agile", "aging", "agony",
"ahead", "aide", "aids", "aim", "ajar", "alarm", "alias", "alibi", "alien", "alike", "alive",
"aloe", "aloft", "aloha", "alone", "amend", "amino", "ample", "amuse", "angel", "anger", "angle",
"ankle", "apple", "april", "apron", "aqua", "area", "arena", "argue", "arise", "armed", "armor",
"army", "aroma", "array", "arson", "art", "ashen", "ashes", "atlas", "atom", "attic", "audio",
"avert", "avoid", "awake", "award", "awoke", "axis", "bacon", "badge", "bagel", "baggy", "baked",
"baker", "balmy", "banjo", "barge", "barn", "bash", "basil", "bask", "batch", "bath", "baton",
"bats", "blade", "blank", "blast", "blaze", "bleak", "blend", "bless", "blimp", "blink", "bloat",
"blob", "blog", "blot", "blunt", "blurt", "blush", "boast", "boat", "body", "boil", "bok", "bolt",
"boned", "boney", "bonus", "bony", "book", "booth", "boots", "boss", "botch", "both", "boxer",
"breed", "bribe", "brick", "bride", "brim", "bring", "brink", "brisk", "broad", "broil", "broke",
"brook", "broom", "brush", "buck", "bud", "buggy", "bulge", "bulk", "bully", "bunch", "bunny",
"bunt", "bush", "bust", "busy", "buzz", "cable", "cache", "cadet", "cage", "cake", "calm", "cameo",
"canal", "candy", "cane", "canon", "cape", "card", "cargo", "carol", "carry", "carve", "case",
"cash", "cause", "cedar", "chain", "chair", "chant", "chaos", "charm", "chase", "cheek", "cheer",
"chef", "chess", "chest", "chew", "chief", "chili", "chill", "chip", "chomp", "chop", "chow",
"chuck", "chump", "chunk", "churn", "chute", "cider", "cinch", "city", "civic", "civil", "clad",
"claim", "clamp", "clap", "clash", "clasp", "class", "claw", "clay", "clean", "clear", "cleat",
"cleft", "clerk", "click", "cling", "clink", "clip", "cloak", "clock", "clone", "cloth", "cloud",
"clump", "coach", "coast", "coat", "cod", "coil", "coke", "cola", "cold", "colt", "coma", "come",
"comic", "comma", "cone", "cope", "copy", "coral", "cork", "cost", "cot", "couch", "cough",
"cover", "cozy", "craft", "cramp", "crane", "crank", "crate", "crave", "crawl", "crazy", "creme",
"crepe", "crept", "crib", "cried", "crisp", "crook", "crop", "cross", "crowd", "crown", "crumb",
"crush", "crust", "cub", "cult", "cupid", "cure", "curl", "curry", "curse", "curve", "curvy",
"cushy", "cut", "cycle", "dab", "dad", "daily", "dairy", "daisy", "dance", "dandy", "darn",
"dart", "dash", "data", "date", "dawn", "deaf", "deal", "dean", "debit", "debt", "debug", "decaf",
"decal", "decay", "deck", "decor", "decoy", "deed", "delay", "denim", "dense", "dent", "depth",
"derby", "desk", "dial", "diary", "dice", "dig", "dill", "dime", "dimly", "diner", "dingy",
"disco", "dish", "disk", "ditch", "ditzy", "dizzy", "dock", "dodge", "doing", "doll", "dome",
"donor", "donut", "dose", "dot", "dove", "down", "dowry", "doze", "drab", "drama", "drank",
"draw", "dress", "dried", "drift", "drill", "drive", "drone", "droop", "drove", "drown", "drum",
"dry", "duck", "duct", "dude", "dug", "duke", "duo", "dusk", "dust", "duty", "dwarf", "dwell",
"eagle", "early", "earth", "easel", "east", "eaten", "eats", "ebay", "ebony", "ebook", "echo",
"edge", "eel", "eject", "elbow", "elder", "elf", "elk", "elm", "elope", "elude", "elves", "email",
"emit", "empty", "emu", "enter", "entry", "envoy", "equal", "erase", "error", "erupt", "essay",
"etch", "evade", "even", "evict", "evil", "evoke", "exact", "exit", "fable", "faced", "fact",
"fade", "fall", "false", "fancy", "fang", "fax", "feast", "feed", "femur", "fence", "fend",
"ferry", "fetal", "fetch", "fever", "fiber", "fifth", "fifty", "film", "filth", "final", "finch",
"fit", "five", "flag", "flaky", "flame", "flap", "flask", "fled", "flick", "fling", "flint",
"flip", "flirt", "float", "flock", "flop", "floss", "flyer", "foam", "foe", "fog", "foil", "folic",
"folk", "food", "fool", "found", "fox", "foyer", "frail", "frame", "fray", "fresh", "fried",
"frill", "frisk", "from", "front", "frost", "froth", "frown", "froze", "fruit", "gag", "gains",
"gala", "game", "gap", "gas", "gave", "gear", "gecko", "geek", "gem", "genre", "gift", "gig",
"gills", "given", "giver", "glad", "glass", "glide", "gloss", "glove", "glow", "glue", "goal",
"going", "golf", "gong", "good", "gooey", "goofy", "gore", "gown", "grab", "grain", "grant",
"grape", "graph", "grasp", "grass", "grave", "gravy", "gray", "green", "greet", "grew", "grid",
"grief", "grill", "grip", "grit", "groom", "grope", "growl", "grub", "grunt", "guide", "gulf",
"gulp", "gummy", "guru", "gush", "gut", "guy", "habit", "half", "halo", "halt", "happy", "harm",
"hash", "hasty", "hatch", "hate", "haven", "hazel", "hazy", "heap", "heat", "heave", "hedge",
"hefty", "help", "herbs", "hers", "hub", "hug", "hula", "hull", "human", "humid", "hump", "hung",
"hunk", "hunt", "hurry", "hurt", "hush", "hut", "ice", "icing", "icon", "icy", "igloo", "image",
"ion", "iron", "islam", "issue", "item", "ivory", "ivy", "jab", "jam", "jaws", "jazz", "jeep",
"jelly", "jet", "jiffy", "job", "jog", "jolly", "jolt", "jot", "joy", "judge", "juice", "juicy",
"july", "jumbo", "jump", "junky", "juror", "jury", "keep", "keg", "kept", "kick", "kilt", "king",
"kite", "kitty", "kiwi", "knee", "knelt", "koala", "kung", "ladle", "lady", "lair", "lake",
"lance", "land", "lapel", "large", "lash", "lasso", "last", "latch", "late", "lazy", "left",
"legal", "lemon", "lend", "lens", "lent", "level", "lever", "lid", "life", "lift", "lilac",
"lily", "limb", "limes", "line", "lint", "lion", "lip", "list", "lived", "liver", "lunar", "lunch",
"lung", "lurch", "lure", "lurk", "lying", "lyric", "mace", "maker", "malt", "mama", "mango",
"manor", "many", "map", "march", "mardi", "marry", "mash", "match", "mate", "math", "moan",
"mocha", "moist", "mold", "mom", "moody", "mop", "morse", "most", "motor", "motto", "mount",
"mouse", "mousy", "mouth", "move", "movie", "mower", "mud", "mug", "mulch", "mule", "mull",
"mumbo", "mummy", "mural", "muse", "music", "musky", "mute", "nacho", "nag", "nail", "name",
"nanny", "nap", "navy", "near", "neat", "neon", "nerd", "nest", "net", "next", "niece", "ninth",
"nutty", "oak", "oasis", "oat", "ocean", "oil", "old", "olive", "omen", "onion", "only", "ooze",
"opal", "open", "opera", "opt", "otter", "ouch", "ounce", "outer", "oval", "oven", "owl", "ozone",
"pace", "pagan", "pager", "palm", "panda", "panic", "pants", "panty", "paper", "park", "party",
"pasta", "patch", "path", "patio", "payer", "pecan", "penny", "pep", "perch", "perky", "perm",
"pest", "petal", "petri", "petty", "photo", "plank", "plant", "plaza", "plead", "plot", "plow",
"pluck", "plug", "plus", "poach", "pod", "poem", "poet", "pogo", "point", "poise", "poker",
"polar", "polio", "polka", "polo", "pond", "pony", "poppy", "pork", "poser", "pouch", "pound",
"pout", "power", "prank", "press", "print", "prior", "prism", "prize", "probe", "prong", "proof",
"props", "prude", "prune", "pry", "pug", "pull", "pulp", "pulse", "puma", "punch", "punk", "pupil",
"puppy", "purr", "purse", "push", "putt", "quack", "quake", "query", "quiet", "quill", "quilt",
"quit", "quota", "quote", "rabid", "race", "rack", "radar", "radio", "raft", "rage", "raid",
"rail", "rake", "rally", "ramp", "ranch", "range", "rank", "rant", "rash", "raven", "reach",
"react", "ream", "rebel", "recap", "relax", "relay", "relic", "remix", "repay", "repel", "reply",
"rerun", "reset", "rhyme", "rice", "rich", "ride", "rigid", "rigor", "rinse", "riot", "ripen",
"rise", "risk", "ritzy", "rival", "river", "roast", "robe", "robin", "rock", "rogue", "roman",
"romp", "rope", "rover", "royal", "ruby", "rug", "ruin", "rule", "runny", "rush", "rust", "rut",
"sadly", "sage", "said", "saint", "salad", "salon", "salsa", "salt", "same", "sandy", "santa",
"satin", "sauna", "saved", "savor", "sax", "say", "scale", "scam", "scan", "scare", "scarf",
"scary", "scoff", "scold", "scoop", "scoot", "scope", "score", "scorn", "scout", "scowl", "scrap",
"scrub", "scuba", "scuff", "sect", "sedan", "self", "send", "sepia", "serve", "set", "seven",
"shack", "shade", "shady", "shaft", "shaky", "sham", "shape", "share", "sharp", "shed", "sheep",
"sheet", "shelf", "shell", "shine", "shiny", "ship", "shirt", "shock", "shop", "shore", "shout",
"shove", "shown", "showy", "shred", "shrug", "shun", "shush", "shut", "shy", "sift", "silk",
"silly", "silo", "sip", "siren", "sixth", "size", "skate", "skew", "skid", "skier", "skies",
"skip", "skirt", "skit", "sky", "slab", "slack", "slain", "slam", "slang", "slash", "slate",
"slaw", "sled", "sleek", "sleep", "sleet", "slept", "slice", "slick", "slimy", "sling", "slip",
"slit", "slob", "slot", "slug", "slum", "slurp", "slush", "small", "smash", "smell", "smile",
"smirk", "smog", "snack", "snap", "snare", "snarl", "sneak", "sneer", "sniff", "snore", "snort",
"snout", "snowy", "snub", "snuff", "speak", "speed", "spend", "spent", "spew", "spied", "spill",
"spiny", "spoil", "spoke", "spoof", "spool", "spoon", "sport", "spot", "spout", "spray", "spree",
"spur", "squad", "squat", "squid", "stack", "staff", "stage", "stain", "stall", "stamp", "stand",
"stank", "stark", "start", "stash", "state", "stays", "steam", "steep", "stem", "step", "stew",
"stick", "sting", "stir", "stock", "stole", "stomp", "stony", "stood", "stool", "stoop", "stop",
"storm", "stout", "stove", "straw", "stray", "strut", "stuck", "stud", "stuff", "stump", "stung",
"stunt", "suds", "sugar", "sulk", "surf", "sushi", "swab", "swan", "swarm", "sway", "swear",
"sweat", "sweep", "swell", "swept", "swim", "swing", "swipe", "swirl", "swoop", "swore", "syrup",
"tacky", "taco", "tag", "take", "tall", "talon", "tamer", "tank", "taper", "taps", "tarot",
"tart", "task", "taste", "tasty", "taunt", "thank", "thaw", "theft", "theme", "thigh", "thing",
"think", "thong", "thorn", "those", "throb", "thud", "thumb", "thump", "thus", "tiara", "tidal",
"tidy", "tiger", "tile", "tilt", "tint", "tiny", "trace", "track", "trade", "train", "trait",
"trap", "trash", "tray", "treat", "tree", "trek", "trend", "trial", "tribe", "trick", "trio",
"trout", "truce", "truck", "trump", "trunk", "try", "tug", "tulip", "tummy", "turf", "tusk",
"tutor", "tutu", "tux", "tweak", "tweet", "twice", "twine", "twins", "twirl", "twist", "uncle",
"uncut", "undo", "unify", "union", "unit", "untie", "upon", "upper", "urban", "used", "user",
"usher", "utter", "value", "vapor", "vegan", "venue", "verse", "vest", "veto", "vice", "video",
"view", "viral", "virus", "visa", "visor", "vixen", "vocal", "voice", "void", "volt", "voter",
"vowel", "wad", "wafer", "wager", "wages", "wagon", "wake", "walk", "wand", "wasp", "watch",
"water", "wavy", "wheat", "whiff", "whole", "whoop", "wick", "widen", "widow", "width", "wife",
"wifi", "wilt", "wimp", "wind", "wing", "wink", "wipe", "wired", "wiry", "wise", "wish", "wispy",
"wok", "wolf", "womb", "wool", "woozy", "word", "work", "worry", "wound", "woven", "wrath",
"wreck", "wrist", "xerox", "yahoo", "yam", "yard", "year", "yeast", "yelp", "yield", "yo-yo",
"yodel", "yoga", "yoyo", "yummy", "zebra", "zero", "zesty", "zippy", "zone", "zoom"
};



CrashHandler& CrashHandler::getInstance() {
#if HAS_SENTRY
    static CrashHandlerSentry sharedInstance;
#else
    static CrashHandlerNone sharedInstance;
#endif

    return sharedInstance;
}

CrashHandler::CrashHandler(QObject *parent) : QObject(parent) {
    auto rng = QRandomGenerator::global();

    _supportTag = EFF_SHORT_WORDLIST[rng->bounded(EFF_SHORT_WORDLIST.size())] + "-" +
                  EFF_SHORT_WORDLIST[rng->bounded(EFF_SHORT_WORDLIST.size())] + "-" +
                  EFF_SHORT_WORDLIST[rng->bounded(EFF_SHORT_WORDLIST.size())];
}


void CrashHandler::setPath(const QString &path) {
    QFileInfo fi(path);

    if (isStarted()) {
        qCWarning(crash_handler) << "Crash handler already started, too late to set the path.";
    }

    if (fi.isFile()) {
        _path = fi.absolutePath();
    } else {
        _path = path;
    }
}

bool CrashHandler::start() {
    if (isStarted()) {
        //qCWarning(crash_handler) << "Crash handler already started";
        return false;
    }

    auto started = startCrashHandler();
    setStarted(started);

    if ( started ) {
        qCInfo(crash_handler) << "Crash handler started. Support tag:" << getSupportTag();
        std::size_t countAdded = 0;

        {
            std::lock_guard<std::mutex> lock(_annotationsMutex);
            for(const auto &item : _annotations) {
                setTag(item.first, item.second);
            }

            countAdded = _annotations.size();
            _annotations.clear();
        }

        qCDebug(crash_handler) << "Forwarded" << countAdded << "annotations";


        qInstallMessageHandler(crashHandlerLogMessage);
        qCInfo(crash_handler) << "Installed crash handler log message handler";


    } else {
        qCWarning(crash_handler) << "Crash handler failed to start";
    }

    return started;
}

void CrashHandler::startMonitor(QCoreApplication *app) {
   // startCrashHookMonitor(app);
}

void CrashHandler::setEnabled(bool enabled) {
    start();

    if (enabled != _crashReportingEnabled) {
        _crashReportingEnabled = enabled;
        setCrashReportingEnabled(enabled);

        emit enabledChanged(enabled);
    }
}

void CrashHandler::setUrl(const QString &url) {
    // This can be called both from the settings system in an assignment client
    // and from the commandline parser. We only emit a warning if the commandline
    // argument causes the domain setting to be ignored.

    if (isStarted() && url != _crashUrl) {
        qCWarning(crash_handler) << "Setting crash reporting URL to " << url << "after the crash handler is already running has no effect";
    } else {
        _crashUrl = url;
    }
}

void CrashHandler::setToken(const QString &token) {
    if (isStarted() && token != _crashToken) {
        qCWarning(crash_handler) << "Setting crash reporting token to " << token << "after the crash handler is already running has no effect";
    } else {
        _crashToken = token;
    }
}

void CrashHandler::setAnnotation(const std::string &key, const char *value) {
    setAnnotation(key, std::string(value));
}

void CrashHandler::setAnnotation(const std::string &key, const QString &value) {
    setAnnotation(key, value.toStdString());
}

void CrashHandler::setAnnotation(const std::string &key, const std::string &value) {
    if (!isStarted()) {
        std::lock_guard<std::mutex> lock(_annotationsMutex);
        _annotations[key] = value;
        return;
    }

    setTag(key, value);
}


void CrashHandler::logMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    // Some of the logging is very spammy and Sentry seems to have a problem with shutting down
    // in that case.
    {
        std::lock_guard<std::mutex> lock(_logMutex);

        if (msg == _previousMessage) {
            _repeatCount++;
            return;
        }

        if (_repeatCount > 0) {
            sendLogMessage(QtMsgType::QtWarningMsg, QMessageLogContext(), QString("Previous log message repeated %1 times").arg(_repeatCount));
        }

        _previousMessage = msg;
        _repeatCount = 0;
    }

    if (isLogStreamingEnabled()) {
        sendLogMessage(type, context, msg);
    }
}

// Locate the full path to the binary's directory
QString CrashHandler::findBinaryDir() {
    // Normally we'd just use QCoreApplication::applicationDirPath(), but we can't.
    // That function needs the QApplication to be created first, and Crashpad is initialized as early as possible,
    // which is well before QApplication, so that function throws out a warning and returns ".".
    //
    // So we must do things the hard way here. In particular this is needed to correctly handle things in AppImage
    // on Linux. On Windows and MacOS falling back to argv[0] should be fine.

#ifdef Q_OS_LINUX
    // Find outselves by looking at /proc/<PID>/exe
    pid_t ourPid = getpid();
    QString exeLink = QString("/proc/%1/exe").arg(ourPid);
    qCDebug(crash_handler) << "Looking at" << exeLink;

    QFileInfo exeLinkInfo(exeLink);
    if (exeLinkInfo.isSymLink()) {
        QFileInfo exeInfo(exeLinkInfo.symLinkTarget());
        qCDebug(crash_handler) << "exe symlink points at" << exeInfo;
        return exeInfo.absoluteDir().absolutePath();
    } else {
        qCWarning(crash_handler) << exeLink << "isn't a symlink. /proc not mounted?";
    }

#endif

    return QString();
}


static void crashHandlerLogMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    CrashHandler::getInstance().logMessage(type, context, msg);

    // We only get one log handler, so call the normal one here since we replaced it
    LogHandler::getInstance().verboseMessageHandler(type, context, msg);
}
