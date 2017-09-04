/*
 mnemonic.c

 Copyright (c) 2000  Oren Tirosh <oren@hishome.net>

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/

#include <ctype.h>
#include <string.h>


#define MN_BASE   1626    /* cubic root of 2^32, rounded up */
#define MN_REMAINDER  7   /* extra words for 24 bit remainders */
#define MN_WORDS (MN_BASE+MN_REMAINDER) /* total number of words */
#define MN_WORD_BUFLEN  25    /* size for a word buffer+headroom */

#define MN_EOF    0   /* signal end to mn_decode_word_index */

/* result codes */
#define MN_OK   0   /* decoding ok */
#define MN_EREM   -1    /* unexpected arithmetic remainder */
#define MN_EOVERRUN -2    /* output buffer overrun */
#define MN_EOVERRUN24 -3    /* data after 24 bit remainder */
#define MN_EINDEX -4    /* bad word index */
#define MN_EINDEX24 -5    /* unexpected 24 bit remainder word */
#define MN_EENCODING  -6    /* invalid arithmetic encoding */
#define MN_EWORD  -7    /* unrecognized word */
#define MN_EFORMAT  -8    /* bad format string */

/* Sample formats for mn_encode */
#define MN_FDEFAULT     "x-x-x--"
#define MN_F64BITSPERLINE " x-x-x--x-x-x\n"
#define MN_F96BITSPERLINE " x-x-x--x-x-x--x-x-x\n"
#define MN_F128BITSPERLINE  " x-x-x--x-x-x--x-x-x--x-x-x\n"
/* Note that the last format does not fit in a standard 80 character line */

typedef unsigned char mn_byte;    /* 8 bit quantity */
typedef unsigned long mn_word32;  /* temporary value, at least 32 bits */
/* Range checks assume that mn_index is unsigned (=> can't be <0).  --DV */
typedef unsigned int mn_index;    /* index into wordlist */

int         mn_encode (const mn_byte *src , int srcsize, char *dest, int destsize, char *format);

int         mn_words_required (int size);
mn_index    mn_encode_word_index (const mn_byte *src, int srcsize, int n);
const char* mn_encode_word (const mn_byte *src, int srcsize, int n);



const char *mn_wordlist_version =
" Wordlist ver 0.7";

const char *mn_words[MN_WORDS + 1] = { 0,
  "academy",  "acrobat",  "active",   "actor",    "adam",     "admiral",
  "adrian",   "africa",   "agenda",   "agent",    "airline",  "airport",
  "aladdin",  "alarm",    "alaska",   "albert",   "albino",   "album",
  "alcohol",  "alex",     "algebra",  "alibi",    "alice",    "alien",
  "alpha",    "alpine",   "amadeus",  "amanda",   "amazon",   "amber",
  "america",  "amigo",    "analog",   "anatomy",  "angel",    "animal",
  "antenna",  "antonio",  "apollo",   "april",    "archive",  "arctic",
  "arizona",  "arnold",   "aroma",    "arthur",   "artist",   "asia",
  "aspect",   "aspirin",  "athena",   "athlete",  "atlas",    "audio",
  "august",   "austria",  "axiom",    "aztec",    "balance",  "ballad",
  "banana",   "bandit",   "banjo",    "barcode",  "baron",    "basic",
  "battery",  "belgium",  "berlin",   "bermuda",  "bernard",  "bikini",
  "binary",   "bingo",    "biology",  "block",    "blonde",   "bonus",
  "boris",    "boston",   "boxer",    "brandy",   "bravo",    "brazil",
  "bronze",   "brown",    "bruce",    "bruno",    "burger",   "burma",
  "cabinet",  "cactus",   "cafe",     "cairo",    "cake",     "calypso",
  "camel",    "camera",   "campus",   "canada",   "canal",    "cannon",
  "canoe",    "cantina",  "canvas",   "canyon",   "capital",  "caramel",
  "caravan",  "carbon",   "cargo",    "carlo",    "carol",    "carpet",
  "cartel",   "casino",   "castle",   "castro",   "catalog",  "caviar",
  "cecilia",  "cement",   "center",   "century",  "ceramic",  "chamber",
  "chance",   "change",   "chaos",    "charlie",  "charm",    "charter",
  "chef",     "chemist",  "cherry",   "chess",    "chicago",  "chicken",
  "chief",    "china",    "cigar",    "cinema",   "circus",   "citizen",
  "city",     "clara",    "classic",  "claudia",  "clean",    "client",
  "climax",   "clinic",   "clock",    "club",     "cobra",    "coconut",
  "cola",     "collect",  "colombo",  "colony",   "color",    "combat",
  "comedy",   "comet",    "command",  "compact",  "company",  "complex",
  "concept",  "concert",  "connect",  "consul",   "contact",  "context",
  "contour",  "control",  "convert",  "copy",     "corner",   "corona",
  "correct",  "cosmos",   "couple",   "courage",  "cowboy",   "craft",
  "crash",    "credit",   "cricket",  "critic",   "crown",    "crystal",
  "cuba",     "culture",  "dallas",   "dance",    "daniel",   "david",
  "decade",   "decimal",  "deliver",  "delta",    "deluxe",   "demand",
  "demo",     "denmark",  "derby",    "design",   "detect",   "develop",
  "diagram",  "dialog",   "diamond",  "diana",    "diego",    "diesel",
  "diet",     "digital",  "dilemma",  "diploma",  "direct",   "disco",
  "disney",   "distant",  "doctor",   "dollar",   "dominic",  "domino",
  "donald",   "dragon",   "drama",    "dublin",   "duet",     "dynamic",
  "east",     "ecology",  "economy",  "edgar",    "egypt",    "elastic",
  "elegant",  "element",  "elite",    "elvis",    "email",    "energy",
  "engine",   "english",  "episode",  "equator",  "escort",   "ethnic",
  "europe",   "everest",  "evident",  "exact",    "example",  "exit",
  "exotic",   "export",   "express",  "extra",    "fabric",   "factor",
  "falcon",   "family",   "fantasy",  "fashion",  "fiber",    "fiction",
  "fidel",    "fiesta",   "figure",   "film",     "filter",   "final",
  "finance",  "finish",   "finland",  "flash",    "florida",  "flower",
  "fluid",    "flute",    "focus",    "ford",     "forest",   "formal",
  "format",   "formula",  "fortune",  "forum",    "fragile",  "france",
  "frank",    "friend",   "frozen",   "future",   "gabriel",  "galaxy",
  "gallery",  "gamma",    "garage",   "garden",   "garlic",   "gemini",
  "general",  "genetic",  "genius",   "germany",  "global",   "gloria",
  "golf",     "gondola",  "gong",     "good",     "gordon",   "gorilla",
  "grand",    "granite",  "graph",    "green",    "group",    "guide",
  "guitar",   "guru",     "hand",     "happy",    "harbor",   "harmony",
  "harvard",  "havana",   "hawaii",   "helena",   "hello",    "henry",
  "hilton",   "history",  "horizon",  "hotel",    "human",    "humor",
  "icon",     "idea",     "igloo",    "igor",     "image",    "impact",
  "import",   "index",    "india",    "indigo",   "input",    "insect",
  "instant",  "iris",     "italian",  "jacket",   "jacob",    "jaguar",
  "janet",    "japan",    "jargon",   "jazz",     "jeep",     "john",
  "joker",    "jordan",   "jumbo",    "june",     "jungle",   "junior",
  "jupiter",  "karate",   "karma",    "kayak",    "kermit",   "kilo",
  "king",     "koala",    "korea",    "labor",    "lady",     "lagoon",
  "laptop",   "laser",    "latin",    "lava",     "lecture",  "left",
  "legal",    "lemon",    "level",    "lexicon",  "liberal",  "libra",
  "limbo",    "limit",    "linda",    "linear",   "lion",     "liquid",
  "liter",    "little",   "llama",    "lobby",    "lobster",  "local",
  "logic",    "logo",     "lola",     "london",   "lotus",    "lucas",
  "lunar",    "machine",  "macro",    "madam",    "madonna",  "madrid",
  "maestro",  "magic",    "magnet",   "magnum",   "major",    "mama",
  "mambo",    "manager",  "mango",    "manila",   "marco",    "marina",
  "market",   "mars",     "martin",   "marvin",   "master",   "matrix",
  "maximum",  "media",    "medical",  "mega",     "melody",   "melon",
  "memo",     "mental",   "mentor",   "menu",     "mercury",  "message",
  "metal",    "meteor",   "meter",    "method",   "metro",    "mexico",
  "miami",    "micro",    "million",  "mineral",  "minimum",  "minus",
  "minute",   "miracle",  "mirage",   "miranda",  "mister",   "mixer",
  "mobile",   "model",    "modem",    "modern",   "modular",  "moment",
  "monaco",   "monica",   "monitor",  "mono",     "monster",  "montana",
  "morgan",   "motel",    "motif",    "motor",    "mozart",   "multi",
  "museum",   "music",    "mustang",  "natural",  "neon",     "nepal",
  "neptune",  "nerve",    "neutral",  "nevada",   "news",     "ninja",
  "nirvana",  "normal",   "nova",     "novel",    "nuclear",  "numeric",
  "nylon",    "oasis",    "object",   "observe",  "ocean",    "octopus",
  "olivia",   "olympic",  "omega",    "opera",    "optic",    "optimal",
  "orange",   "orbit",    "organic",  "orient",   "origin",   "orlando",
  "oscar",    "oxford",   "oxygen",   "ozone",    "pablo",    "pacific",
  "pagoda",   "palace",   "pamela",   "panama",   "panda",    "panel",
  "panic",    "paradox",  "pardon",   "paris",    "parker",   "parking",
  "parody",   "partner",  "passage",  "passive",  "pasta",    "pastel",
  "patent",   "patriot",  "patrol",   "patron",   "pegasus",  "pelican",
  "penguin",  "pepper",   "percent",  "perfect",  "perfume",  "period",
  "permit",   "person",   "peru",     "phone",    "photo",    "piano",
  "picasso",  "picnic",   "picture",  "pigment",  "pilgrim",  "pilot",
  "pirate",   "pixel",    "pizza",    "planet",   "plasma",   "plaster",
  "plastic",  "plaza",    "pocket",   "poem",     "poetic",   "poker",
  "polaris",  "police",   "politic",  "polo",     "polygon",  "pony",
  "popcorn",  "popular",  "postage",  "postal",   "precise",  "prefix",
  "premium",  "present",  "price",    "prince",   "printer",  "prism",
  "private",  "product",  "profile",  "program",  "project",  "protect",
  "proton",   "public",   "pulse",    "puma",     "pyramid",  "queen",
  "radar",    "radio",    "random",   "rapid",    "rebel",    "record",
  "recycle",  "reflex",   "reform",   "regard",   "regular",  "relax",
  "report",   "reptile",  "reverse",  "ricardo",  "ringo",    "ritual",
  "robert",   "robot",    "rocket",   "rodeo",    "romeo",    "royal",
  "russian",  "safari",   "salad",    "salami",   "salmon",   "salon",
  "salute",   "samba",    "sandra",   "santana",  "sardine",  "school",
  "screen",   "script",   "second",   "secret",   "section",  "segment",
  "select",   "seminar",  "senator",  "senior",   "sensor",   "serial",
  "service",  "sheriff",  "shock",    "sierra",   "signal",   "silicon",
  "silver",   "similar",  "simon",    "single",   "siren",    "slogan",
  "social",   "soda",     "solar",    "solid",    "solo",     "sonic",
  "soviet",   "special",  "speed",    "spiral",   "spirit",   "sport",
  "static",   "station",  "status",   "stereo",   "stone",    "stop",
  "street",   "strong",   "student",  "studio",   "style",    "subject",
  "sultan",   "super",    "susan",    "sushi",    "suzuki",   "switch",
  "symbol",   "system",   "tactic",   "tahiti",   "talent",   "tango",
  "tarzan",   "taxi",     "telex",    "tempo",    "tennis",   "texas",
  "textile",  "theory",   "thermos",  "tiger",    "titanic",  "tokyo",
  "tomato",   "topic",    "tornado",  "toronto",  "torpedo",  "total",
  "totem",    "tourist",  "tractor",  "traffic",  "transit",  "trapeze",
  "travel",   "tribal",   "trick",    "trident",  "trilogy",  "tripod",
  "tropic",   "trumpet",  "tulip",    "tuna",     "turbo",    "twist",
  "ultra",    "uniform",  "union",    "uranium",  "vacuum",   "valid",
  "vampire",  "vanilla",  "vatican",  "velvet",   "ventura",  "venus",
  "vertigo",  "veteran",  "victor",   "video",    "vienna",   "viking",
  "village",  "vincent",  "violet",   "violin",   "virtual",  "virus",
  "visa",     "vision",   "visitor",  "visual",   "vitamin",  "viva",
  "vocal",    "vodka",    "volcano",  "voltage",  "volume",   "voyage",
  "water",    "weekend",  "welcome",  "western",  "window",   "winter",
  "wizard",   "wolf",     "world",    "xray",     "yankee",   "yoga",
  "yogurt",   "yoyo",     "zebra",    "zero",     "zigzag",   "zipper",
  "zodiac",   "zoom",     "abraham",  "action",   "address",  "alabama",
  "alfred",   "almond",   "ammonia",  "analyze",  "annual",   "answer",
  "apple",    "arena",    "armada",   "arsenal",  "atlanta",  "atomic",
  "avenue",   "average",  "bagel",    "baker",    "ballet",   "bambino",
  "bamboo",   "barbara",  "basket",   "bazaar",   "benefit",  "bicycle",
  "bishop",   "blitz",    "bonjour",  "bottle",   "bridge",   "british",
  "brother",  "brush",    "budget",   "cabaret",  "cadet",    "candle",
  "capitan",  "capsule",  "career",   "cartoon",  "channel",  "chapter",
  "cheese",   "circle",   "cobalt",   "cockpit",  "college",  "compass",
  "comrade",  "condor",   "crimson",  "cyclone",  "darwin",   "declare",
  "degree",   "delete",   "delphi",   "denver",   "desert",   "divide",
  "dolby",    "domain",   "domingo",  "double",   "drink",    "driver",
  "eagle",    "earth",    "echo",     "eclipse",  "editor",   "educate",
  "edward",   "effect",   "electra",  "emerald",  "emotion",  "empire",
  "empty",    "escape",   "eternal",  "evening",  "exhibit",  "expand",
  "explore",  "extreme",  "ferrari",  "first",    "flag",     "folio",
  "forget",   "forward",  "freedom",  "fresh",    "friday",   "fuji",
  "galileo",  "garcia",   "genesis",  "gold",     "gravity",  "habitat",
  "hamlet",   "harlem",   "helium",   "holiday",  "house",    "hunter",
  "ibiza",    "iceberg",  "imagine",  "infant",   "isotope",  "jackson",
  "jamaica",  "jasmine",  "java",     "jessica",  "judo",     "kitchen",
  "lazarus",  "letter",   "license",  "lithium",  "loyal",    "lucky",
  "magenta",  "mailbox",  "manual",   "marble",   "mary",     "maxwell",
  "mayor",    "milk",     "monarch",  "monday",   "money",    "morning",
  "mother",   "mystery",  "native",   "nectar",   "nelson",   "network",
  "next",     "nikita",   "nobel",    "nobody",   "nominal",  "norway",
  "nothing",  "number",   "october",  "office",   "oliver",   "opinion",
  "option",   "order",    "outside",  "package",  "pancake",  "pandora",
  "panther",  "papa",     "patient",  "pattern",  "pedro",    "pencil",
  "people",   "phantom",  "philips",  "pioneer",  "pluto",    "podium",
  "portal",   "potato",   "prize",    "process",  "protein",  "proxy",
  "pump",     "pupil",    "python",   "quality",  "quarter",  "quiet",
  "rabbit",   "radical",  "radius",   "rainbow",  "ralph",    "ramirez",
  "ravioli",  "raymond",  "respect",  "respond",  "result",   "resume",
  "retro",    "richard",  "right",    "risk",     "river",    "roger",
  "roman",    "rondo",    "sabrina",  "salary",   "salsa",    "sample",
  "samuel",   "saturn",   "savage",   "scarlet",  "scoop",    "scorpio",
  "scratch",  "scroll",   "sector",   "serpent",  "shadow",   "shampoo",
  "sharon",   "sharp",    "short",    "shrink",   "silence",  "silk",
  "simple",   "slang",    "smart",    "smoke",    "snake",    "society",
  "sonar",    "sonata",   "soprano",  "source",   "sparta",   "sphere",
  "spider",   "sponsor",  "spring",   "acid",     "adios",    "agatha",
  "alamo",    "alert",    "almanac",  "aloha",    "andrea",   "anita",
  "arcade",   "aurora",   "avalon",   "baby",     "baggage",  "balloon",
  "bank",     "basil",    "begin",    "biscuit",  "blue",     "bombay",
  "brain",    "brenda",   "brigade",  "cable",    "carmen",   "cello",
  "celtic",   "chariot",  "chrome",   "citrus",   "civil",    "cloud",
  "common",   "compare",  "cool",     "copper",   "coral",    "crater",
  "cubic",    "cupid",    "cycle",    "depend",   "door",     "dream",
  "dynasty",  "edison",   "edition",  "enigma",   "equal",    "eric",
  "event",    "evita",    "exodus",   "extend",   "famous",   "farmer",
  "food",     "fossil",   "frog",     "fruit",    "geneva",   "gentle",
  "george",   "giant",    "gilbert",  "gossip",   "gram",     "greek",
  "grille",   "hammer",   "harvest",  "hazard",   "heaven",   "herbert",
  "heroic",   "hexagon",  "husband",  "immune",   "inca",     "inch",
  "initial",  "isabel",   "ivory",    "jason",    "jerome",   "joel",
  "joshua",   "journal",  "judge",    "juliet",   "jump",     "justice",
  "kimono",   "kinetic",  "leonid",   "lima",     "maze",     "medusa",
  "member",   "memphis",  "michael",  "miguel",   "milan",    "mile",
  "miller",   "mimic",    "mimosa",   "mission",  "monkey",   "moral",
  "moses",    "mouse",    "nancy",    "natasha",  "nebula",   "nickel",
  "nina",     "noise",    "orchid",   "oregano",  "origami",  "orinoco",
  "orion",    "othello",  "paper",    "paprika",  "prelude",  "prepare",
  "pretend",  "profit",   "promise",  "provide",  "puzzle",   "remote",
  "repair",   "reply",    "rival",    "riviera",  "robin",    "rose",
  "rover",    "rudolf",   "saga",     "sahara",   "scholar",  "shelter",
  "ship",     "shoe",     "sigma",    "sister",   "sleep",    "smile",
  "spain",    "spark",    "split",    "spray",    "square",   "stadium",
  "star",     "storm",    "story",    "strange",  "stretch",  "stuart",
  "subway",   "sugar",    "sulfur",   "summer",   "survive",  "sweet",
  "swim",     "table",    "taboo",    "target",   "teacher",  "telecom",
  "temple",   "tibet",    "ticket",   "tina",     "today",    "toga",
  "tommy",    "tower",    "trivial",  "tunnel",   "turtle",   "twin",
  "uncle",    "unicorn",  "unique",   "update",   "valery",   "vega",
  "version",  "voodoo",   "warning",  "william",  "wonder",   "year",
  "yellow",   "young",    "absent",   "absorb",   "accent",   "alfonso",
  "alias",    "ambient",  "andy",     "anvil",    "appear",   "apropos",
  "archer",   "ariel",    "armor",    "arrow",    "austin",   "avatar",
  "axis",     "baboon",   "bahama",   "bali",     "balsa",    "bazooka",
  "beach",    "beast",    "beatles",  "beauty",   "before",   "benny",
  "betty",    "between",  "beyond",   "billy",    "bison",    "blast",
  "bless",    "bogart",   "bonanza",  "book",     "border",   "brave",
  "bread",    "break",    "broken",   "bucket",   "buenos",   "buffalo",
  "bundle",   "button",   "buzzer",   "byte",     "caesar",   "camilla",
  "canary",   "candid",   "carrot",   "cave",     "chant",    "child",
  "choice",   "chris",    "cipher",   "clarion",  "clark",    "clever",
  "cliff",    "clone",    "conan",    "conduct",  "congo",    "content",
  "costume",  "cotton",   "cover",    "crack",    "current",  "danube",
  "data",     "decide",   "desire",   "detail",   "dexter",   "dinner",
  "dispute",  "donor",    "druid",    "drum",     "easy",     "eddie",
  "enjoy",    "enrico",   "epoxy",    "erosion",  "except",   "exile",
  "explain",  "fame",     "fast",     "father",   "felix",    "field",
  "fiona",    "fire",     "fish",     "flame",    "flex",     "flipper",
  "float",    "flood",    "floor",    "forbid",   "forever",  "fractal",
  "frame",    "freddie",  "front",    "fuel",     "gallop",   "game",
  "garbo",    "gate",     "gibson",   "ginger",   "giraffe",  "gizmo",
  "glass",    "goblin",   "gopher",   "grace",    "gray",     "gregory",
  "grid",     "griffin",  "ground",   "guest",    "gustav",   "gyro",
  "hair",     "halt",     "harris",   "heart",    "heavy",    "herman",
  "hippie",   "hobby",    "honey",    "hope",     "horse",    "hostel",
  "hydro",    "imitate",  "info",     "ingrid",   "inside",   "invent",
  "invest",   "invite",   "iron",     "ivan",     "james",    "jester",
  "jimmy",    "join",     "joseph",   "juice",    "julius",   "july",
  "justin",   "kansas",   "karl",     "kevin",    "kiwi",     "ladder",
  "lake",     "laura",    "learn",    "legacy",   "legend",   "lesson",
  "life",     "light",    "list",     "locate",   "lopez",    "lorenzo",
  "love",     "lunch",    "malta",    "mammal",   "margo",    "marion",
  "mask",     "match",    "mayday",   "meaning",  "mercy",    "middle",
  "mike",     "mirror",   "modest",   "morph",    "morris",   "nadia",
  "nato",     "navy",     "needle",   "neuron",   "never",    "newton",
  "nice",     "night",    "nissan",   "nitro",    "nixon",    "north",
  "oberon",   "octavia",  "ohio",     "olga",     "open",     "opus",
  "orca",     "oval",     "owner",    "page",     "paint",    "palma",
  "parade",   "parent",   "parole",   "paul",     "peace",    "pearl",
  "perform",  "phoenix",  "phrase",   "pierre",   "pinball",  "place",
  "plate",    "plato",    "plume",    "pogo",     "point",    "polite",
  "polka",    "poncho",   "powder",   "prague",   "press",    "presto",
  "pretty",   "prime",    "promo",    "quasi",    "quest",    "quick",
  "quiz",     "quota",    "race",     "rachel",   "raja",     "ranger",
  "region",   "remark",   "rent",     "reward",   "rhino",    "ribbon",
  "rider",    "road",     "rodent",   "round",    "rubber",   "ruby",
  "rufus",    "sabine",   "saddle",   "sailor",   "saint",    "salt",
  "satire",   "scale",    "scuba",    "season",   "secure",   "shake",
  "shallow",  "shannon",  "shave",    "shelf",    "sherman",  "shine",
  "shirt",    "side",     "sinatra",  "sincere",  "size",     "slalom",
  "slow",     "small",    "snow",     "sofia",    "song",     "sound",
  "south",    "speech",   "spell",    "spend",    "spoon",    "stage",
  "stamp",    "stand",    "state",    "stella",   "stick",    "sting",
  "stock",    "store",    "sunday",   "sunset",   "support",  "sweden",
  "swing",    "tape",     "think",    "thomas",   "tictac",   "time",
  "toast",    "tobacco",  "tonight",  "torch",    "torso",    "touch",
  "toyota",   "trade",    "tribune",  "trinity",  "triton",   "truck",
  "trust",    "type",     "under",    "unit",     "urban",    "urgent",
  "user",     "value",    "vendor",   "venice",   "verona",   "vibrate",
  "virgo",    "visible",  "vista",    "vital",    "voice",    "vortex",
  "waiter",   "watch",    "wave",     "weather",  "wedding",  "wheel",
  "whiskey",  "wisdom",   "deal",     "null",     "nurse",    "quebec",
  "reserve",  "reunion",  "roof",     "singer",   "verbal",   "amen",
  "ego",      "fax",      "jet",      "job",      "rio",      "ski",
  "yes"
};

/*
 * mn_words_required
 *
 * Description:
 *  Calculate the number of words required to encode data using mnemonic
 *  encoding.
 *
 * Parameters:
 *  size - Size in bytes of data to be encoded
 *
 * Return value:
 *  number of words required for the encoding
 */

int
mn_words_required (int size)
{
  return ((size + 1) * 3) / 4;
}


/*
 * mn_encode_word_index
 *
 * Description:
 *  Perform one step of encoding binary data into words. Returns word index.
 *
 * Parameters:
 *   src - Pointer to data buffer to encode
 *   srcsize - Size in bytes of data to encode
 *   n - Sequence number of word to encode
 *       0 <= n < mn_words_required(srcsize)
 *
 * Return value:
 *   0 - no more words to encode / n is out of range
 *   1..MN_WORDS - word index. May be used as index to the mn_words[] array
 */

mn_index mn_encode_word_index (const mn_byte *src, int srcsize, int n)
{
  mn_word32 x = 0;    /* Temporary for MN_BASE arithmetic */
  int offset;     /* Offset into src */
  int remaining;    /* Octets remaining to end of src */
  int extra = 0;    /* Index 7 extra words for 24 bit data */
  int i;
  //const mn_byte *src = vsrc;

  if (n < 0 || n >= mn_words_required (srcsize))
    return 0;     /* word out of range */
  offset = (n / 3) * 4;   /* byte offset into src */
  remaining = srcsize - offset;
  if (remaining <= 0)
    return 0;
  if (remaining >= 4)
    remaining = 4;
  for (i = 0; i < remaining; i++)
    x |= (unsigned long)src[offset + i] << (i * 8); /* endianness-agnostic */

  switch (n % 3)
    {
    case 2:     /* Third word of group */
      if (remaining == 3) /*  special case for 24 bits */
  extra = MN_BASE;  /*  use one of the 7 3-letter words */
      x /= (MN_BASE * MN_BASE);
      break;
    case 1:     /* Second word of group */
      x /= MN_BASE;
    }
  return x % MN_BASE + extra + 1;
}


/*
 * mn_encode_word
 *
 * Description:
 *  Perform one step of encoding binary data into words. Returns pointer
 *  to word.
 *
 * Parameters:
 *   src - Pointer to data buffer to encode
 *   srcsize - Size of data to encode in bytes
 *   n - Sequence number of word to encode.
 *       0 <= n < mn_words_required(srcsize)
 *
 * Return value:
 *   NULL - no more words to encode / n is out of range
 *   valid pointer - pointer to null-terminated lowercase word. length<=7
 */

const char *
mn_encode_word (const mn_byte *src, int srcsize, int n)
{
  return mn_words[mn_encode_word_index (src, srcsize, n)];
}

/*
 * mn_encode
 *
 * Description:
 *  Encode a binary data buffer into a null-terminated sequence of words.
 *  The word separators are taken from the format string.
 *
 * Parameters:
 *  src      - Pointer to the beginning of the binary data buffer.
 *  srcsize  - Size in bytes of binary data buffer
 *  dest     - Pointer to the beginning of a character buffer
 *  destsize - Size in characters of character buffer
 *  format   - Null-terminated string describing the output format.
 *             In the format string any letter or sequence of letters
 *             acts as a placeholder for the encoded words. The word
 *             placeholders are separated by one or more non-letter
 *             characters. When the encoder reaches the end of the
 *             format string it starts reading it again.
 *             For sample formats see MN_F* constants in mnemonic.h
 *         If format is empty or NULL the format MN_FDEFAULT
 *         is used.
 *
 * Return value:
 *  MN_OK(=0)
 *  Encoding was successful.
 *  MN_EOVERRUN
 *  Output size exceeds size of destination buffer
 *  MN_EFORMAT
 *  Invalid format string. This function enforces formats which
 *  will result in a string which can be successfully decoded by
 *  the mn_decode function.
 */

int
mn_encode (const mn_byte *src, int srcsize, char *dest, int destsize, char *format)
{
  int n;
  char *fmt;
  char *destend = dest + destsize;
  const char *word;

  if (format == 0 || format[0] == '\0')
    format = MN_FDEFAULT;
  fmt = format;
  for (n = 0; n < mn_words_required (srcsize); n++)
    {
      while (dest < destend && *fmt != '\0' && !isalpha (*fmt))
  *dest++ = *fmt++;
      if (dest >= destend)
  return MN_EOVERRUN;
      if (*fmt == '\0')
  {
    if (isalpha (fmt[-1]) && isalpha (format[0]))
      return MN_EFORMAT;
    fmt = format;
    while (dest < destend && *fmt != '\0' && !isalpha (*fmt))
      *dest++ = *fmt++;
    if (!isalpha (*fmt))
      return MN_EFORMAT;
  }
      word = mn_encode_word (src, srcsize, n);
      if (word == 0)
  return MN_EOVERRUN; /* shouldn't happen, actually */

      while (isalpha (*fmt))
  fmt++;
      while (dest < destend && *word != '\0')
  *dest++ = *word++;
    }
  if (dest < destend)
    *dest++ = '\0';
  else
    return MN_EOVERRUN;
  return MN_OK;
}
