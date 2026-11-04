/**
 * data.c - Master dataset initialization and lookup functions.
 * Updated with Pune, tripled realistic sample data across Indian cities,
 * and Indian Rupee (INR ₹) currency throughout.
 * 
 * Part of Trip Planner Web Application
 * Core backend implemented in C (DSA)
 */

#include "data.h"

/* Helper to add an undirected road connection between two locations */
static void addRoadEdge(Graph* g, int srcId, int destId, double distKm, int timeMin, double tollInr) {
    if (srcId < 0 || srcId >= g->numLocations || destId < 0 || destId >= g->numLocations) {
        return;
    }
    /* Edge from src to dest */
    EdgeNode* e1 = (EdgeNode*)malloc(sizeof(EdgeNode));
    e1->destinationId = destId;
    e1->distanceKm = distKm;
    e1->travelTimeMin = timeMin;
    e1->tollCost = tollInr;
    e1->next = g->adjList[srcId];
    g->adjList[srcId] = e1;

    /* Edge from dest to src (undirected road) */
    EdgeNode* e2 = (EdgeNode*)malloc(sizeof(EdgeNode));
    e2->destinationId = srcId;
    e2->distanceKm = distKm;
    e2->travelTimeMin = timeMin;
    e2->tollCost = tollInr;
    e2->next = g->adjList[destId];
    g->adjList[destId] = e2;
}

void initMasterData(MasterData* data) {
    memset(data, 0, sizeof(MasterData));
    Graph* g = &data->graph;

    for (int i = 0; i < MAX_LOCATIONS; i++) {
        g->adjList[i] = NULL;
    }

    /* -------------------------------------------------------------
       1. LOCATIONS (Expanded Indian Destinations including Pune)
       ------------------------------------------------------------- */
    Location locs[] = {
        { 0,  "Delhi",      "NCR",          "National capital with millennia of heritage and bustling markets",  28.6139, 77.2090 },
        { 1,  "Agra",       "Uttar Pradesh","Home of the iconic Taj Mahal and Mughal architectural wonders",     27.1767, 78.0081 },
        { 2,  "Jaipur",     "Rajasthan",    "The Pink City, celebrated for majestic forts and royal palaces",     26.9124, 75.7873 },
        { 3,  "Udaipur",    "Rajasthan",    "City of Lakes with romantic palatial architecture",                 24.5854, 73.7125 },
        { 4,  "Jodhpur",    "Rajasthan",    "The Sun City guarded by the mighty Mehrangarh Fort",                26.2389, 73.0243 },
        { 5,  "Varanasi",   "Uttar Pradesh","Spiritual capital of India along the sacred banks of the Ganges",    25.3176, 82.9739 },
        { 6,  "Mumbai",     "Maharashtra",  "India's coastal financial hub with colonial charm and sea drives",   19.0760, 72.8777 },
        { 7,  "Goa",        "Goa",          "Tropical paradise with sun-kissed beaches and Portuguese heritage", 15.2993, 74.1240 },
        { 8,  "Manali",     "Himachal",     "High-altitude Himalayan valley nestled in pine forests",            32.2432, 77.1892 },
        { 9,  "Shimla",     "Himachal",     "Colonial summer capital with scenic Mall Road and ridge views",      31.1048, 77.1734 },
        { 10, "Amritsar",   "Punjab",       "Spiritual center of Sikhism and home to the Golden Temple",         31.6340, 74.8723 },
        { 11, "Rishikesh",  "Uttarakhand",  "Yoga capital on the sacred Ganges surrounded by foothills",         30.0869, 78.2676 },
        { 12, "Port Blair", "Andaman",      "Remote tropical island archipelago in the Bay of Bengal",           11.6234, 92.7265 },
        { 13, "Pune",       "Maharashtra",  "Cultural capital of Maharashtra with historic forts & green hills", 18.5204, 73.8567 },
        { 14, "Bengaluru",  "Karnataka",    "Garden city and tech metropolis with pleasant climate and parks",   12.9716, 77.5946 },
        { 15, "Hyderabad",  "Telangana",    "City of Pearls and Nizams, renowned for Charminar and Biryani",     17.3850, 78.4867 },
        { 16, "Kochi",      "Kerala",       "Queen of the Arabian Sea with tranquil backwaters and spice docks",  9.9312, 76.2673 },
        { 17, "Kolkata",    "West Bengal",  "City of Joy with grand colonial architecture and literary legacy",  22.5726, 88.3639 }
    };

    g->numLocations = sizeof(locs) / sizeof(locs[0]);
    for (int i = 0; i < g->numLocations; i++) {
        g->locations[i] = locs[i];
    }

    /* -------------------------------------------------------------
       2. ROAD NETWORK (Tolls in INR ₹)
       ------------------------------------------------------------- */
    /* Pune connections */
    addRoadEdge(g, 13, 6,  148.0, 160, 320.0);   /* Pune <-> Mumbai (Mumbai-Pune Expressway) */
    addRoadEdge(g, 13, 7,  440.0, 510, 480.0);   /* Pune <-> Goa via NH48 / Kolhapur */
    addRoadEdge(g, 13, 14, 840.0, 840, 850.0);   /* Pune <-> Bengaluru via NH48 */
    addRoadEdge(g, 13, 15, 560.0, 570, 620.0);   /* Pune <-> Hyderabad via NH65 */
    addRoadEdge(g, 13, 3,  890.0, 840, 920.0);   /* Pune <-> Udaipur */

    /* Northern routes */
    addRoadEdge(g, 0, 1,   233.0, 210, 450.0);   /* Delhi <-> Agra */
    addRoadEdge(g, 0, 2,   280.0, 270, 380.0);   /* Delhi <-> Jaipur */
    addRoadEdge(g, 0, 9,   343.0, 420, 420.0);   /* Delhi <-> Shimla */
    addRoadEdge(g, 0, 10,  450.0, 480, 520.0);   /* Delhi <-> Amritsar */
    addRoadEdge(g, 0, 11,  240.0, 300, 290.0);   /* Delhi <-> Rishikesh */
    addRoadEdge(g, 1, 2,   240.0, 240, 310.0);   /* Agra <-> Jaipur */
    addRoadEdge(g, 1, 5,   610.0, 540, 750.0);   /* Agra <-> Varanasi */

    /* Rajasthan circuit */
    addRoadEdge(g, 2, 4,   335.0, 330, 400.0);   /* Jaipur <-> Jodhpur */
    addRoadEdge(g, 2, 3,   395.0, 390, 480.0);   /* Jaipur <-> Udaipur */
    addRoadEdge(g, 4, 3,   250.0, 280, 320.0);   /* Jodhpur <-> Udaipur */

    /* Western & Southern links */
    addRoadEdge(g, 3, 6,   750.0, 720, 850.0);   /* Udaipur <-> Mumbai */
    addRoadEdge(g, 6, 7,   590.0, 600, 680.0);   /* Mumbai <-> Goa */
    addRoadEdge(g, 14, 15, 570.0, 540, 650.0);   /* Bengaluru <-> Hyderabad */
    addRoadEdge(g, 14, 16, 540.0, 580, 580.0);   /* Bengaluru <-> Kochi */
    addRoadEdge(g, 5, 17,  680.0, 720, 720.0);   /* Varanasi <-> Kolkata */

    /* Mountains */
    addRoadEdge(g, 9, 8,   248.0, 420, 220.0);   /* Shimla <-> Manali */
    addRoadEdge(g, 10, 9,  310.0, 420, 350.0);   /* Amritsar <-> Shimla */
    addRoadEdge(g, 11, 9,  230.0, 360, 280.0);   /* Rishikesh <-> Shimla */

    /* -------------------------------------------------------------
       3. TOURIST ATTRACTIONS (Tripled to 115+ Attractions across India)
       Prices in Indian Rupees (INR ₹)
       ------------------------------------------------------------- */
    Attraction attrs[] = {
        /* PUNE (id 13) - 14 Realistic Attractions */
        { 1,  13, "Shaniwar Wada",               "Historical", 50.0,  90,  800, 1830, 4.6, "18th-century grand seat of the Peshwas of Maratha Empire with majestic Delhi Gate" },
        { 2,  13, "Aga Khan Palace",              "Historical", 100.0, 90,  900, 1730, 4.7, "National landmark of Italian arches and serene lawns where Mahatma Gandhi was interned" },
        { 3,  13, "Sinhagad Fort",                "Adventure",  50.0, 180,  600, 1800, 4.9, "Historic cliff fortress perched high in Sahyadri mountains with views and famous kanda bhaji" },
        { 4,  13, "Dagdusheth Ganpati Temple",    "Religious",   0.0,  60,  600, 2230, 5.0, "Celebrated Hindu temple housing the revered golden deity blessed by millions of pilgrims" },
        { 5,  13, "Raja Dinkar Kelkar Museum",    "Cultural",   100.0, 120, 1000, 1730, 4.6, "Collection of 20,000+ rare Indian medieval sculptures, musical instruments and royal craft" },
        { 6,  13, "Pataleshwar Cave Temple",      "Historical",  0.0,  45,  830, 1730, 4.5, "8th-century rock-cut basalt cave temple dedicated to Lord Shiva in heart of Pune" },
        { 7,  13, "Saras Baug & Peshwa Lake",     "Nature",      0.0,  60,  600, 2100, 4.4, "Lush landscaped gardens with central lotus pond, jogging tracks and evening fountain shows" },
        { 8,  13, "Osho Teerth Park",             "Nature",     100.0, 90,  600, 1800, 4.6, "Zen-styled public sanctuary with Japanese stone pathways, bamboo groves and tranquil streams" },
        { 9,  13, "Vetal Tekdi Sunset Peak",      "Nature",      0.0,  90,  530, 1930, 4.8, "Pune's highest hilltop ridge offering panoramic city skyline views and nature trails" },
        { 10, 13, "Khadakwasla Dam & Chowpatty",  "Nature",      0.0,  90,  700, 1900, 4.5, "Popular scenic reservoir for lake breeze, fresh roasted corn, and water panorama" },
        { 11, 13, "Rajiv Gandhi Zoological Park", "Nature",      80.0, 150, 930, 1730, 4.4, "130-acre wildlife reserve featuring leopard enclosures, white tigers and snake rescue park" },
        { 12, 13, "FC Road & Vaishali Food Trail","Food",       350.0, 90,  700, 2300, 4.9, "Legendary student hub for sabudana vada, SPDP, bun maska and south Indian filter coffee" },
        { 13, 13, "Koregaon Park Cafe Walk",      "Food",       600.0, 120, 1100, 2330, 4.8, "Chic tree-lined boulevard lined with artisanal bakeries, global bistros and indie cafes" },
        { 14, 13, "Parvati Hill Temple Complex",  "Religious",   0.0,  90,  500, 2000, 4.7, "Ancient Peshwa shrine atop 103 stone stairs offering cool morning breezes and history museum" },

        /* DELHI (id 0) */
        { 15, 0, "Red Fort Citadel",              "Historical", 80.0, 120, 930, 1730, 4.6, "Mughal imperial citadel of red sandstone built by Emperor Shah Jahan" },
        { 16, 0, "Qutub Minar",                   "Historical", 80.0,  90, 700, 1900, 4.7, "73-meter soaring Afghan-style victory tower with intricate Islamic carvings" },
        { 17, 0, "Humayun's Tomb",                "Historical", 80.0,  90, 600, 1800, 4.8, "UNESCO Persian garden tomb masterpiece that inspired the Taj Mahal design" },
        { 18, 0, "India Gate & Kartavya Path",    "Cultural",    0.0,  60, 600, 2200, 4.7, "Grand national memorial boulevard illuminated with evening water fountains" },
        { 19, 0, "Chandni Chowk Street Food",     "Food",       400.0, 120, 1000, 2100, 4.9, "Legendary Old Delhi food lanes with stuffed parathas, jalebis and spicy chaat" },
        { 20, 0, "Akshardham Temple",             "Cultural",    0.0, 180, 930, 1830, 4.9, "Stunning pink sandstone Hindu spiritual complex with boat ride and musical fountain" },
        { 21, 0, "Lotus Temple",                  "Religious",   0.0,  60, 900, 1730, 4.6, "Marble lotus petal Bahai sanctuary open to contemplation of all faiths" },

        /* AGRA (id 1) */
        { 22, 1, "Taj Mahal",                     "Historical", 250.0, 180, 600, 1830, 5.0, "The world's greatest monument to love in pure white Makrana marble" },
        { 23, 1, "Agra Fort",                     "Historical", 120.0, 120, 600, 1800, 4.6, "Sprawling walled fortress of Mughal rulers overlooking the Yamuna River" },
        { 24, 1, "Fatehpur Sikri Royal Ruins",    "Historical", 100.0, 150, 700, 1800, 4.5, "Akbar's deserted red sandstone royal capital and soaring Buland Darwaza" },
        { 25, 1, "Mehtab Bagh Sunset Garden",     "Nature",      50.0,  60, 600, 1900, 4.5, "Charbagh river park providing picturesque sunset views of the Taj Mahal" },
        { 26, 1, "Sadar Bazaar Petha Trail",      "Food",       250.0,  60, 1000, 2200, 4.4, "Heritage confection lanes famous for authentic Agra petha and dalmoth" },

        /* JAIPUR (id 2) */
        { 27, 2, "Amber Palace & Fort",           "Historical", 200.0, 180, 800, 1730, 4.8, "Hilltop royal fortress renowned for mirror mosaics of Sheesh Mahal" },
        { 28, 2, "Hawa Mahal",                    "Cultural",   100.0,  60, 900, 1700, 4.6, "Palace of Winds with 953 honeycombed carved lattice pink sandstone windows" },
        { 29, 2, "City Palace Jaipur",            "Cultural",   250.0, 120, 930, 1700, 4.7, "Regal living palace museum showcasing royal textiles, armory and courtyards" },
        { 30, 2, "Jantar Mantar Observatory",     "Historical",  80.0,  75, 900, 1630, 4.6, "UNESCO collection of 19 architectural astronomical instruments and sundial" },
        { 31, 2, "Nahargarh Fort Sunset Point",   "Adventure",  100.0, 120, 900, 1900, 4.8, "Cliffside fortress ramparts with aerial evening panorama over Jaipur city" },
        { 32, 2, "Chokhi Dhani Ethnic Resort",    "Cultural",   900.0, 180, 1700, 2300, 4.7, "Traditional Rajasthani village fair with folk dancers, camel rides and thali" },

        /* UDAIPUR (id 3) */
        { 33, 3, "City Palace Udaipur",           "Cultural",   300.0, 150, 900, 1730, 4.8, "Magnificent granite and marble palace complex overlooking Lake Pichola" },
        { 34, 3, "Lake Pichola Boat Cruise",      "Nature",     400.0,  75, 900, 1800, 4.9, "Sunset boat cruise offering close views of Jag Mandir and Lake Palace" },
        { 35, 3, "Saheliyon-ki-Bari",             "Nature",      50.0,  60, 900, 1900, 4.4, "Historic garden of fountains, marble pavilions and lotus ponds" },
        { 36, 3, "Bagore Ki Haveli Folk Show",    "Cultural",   150.0,  75, 1900, 2030, 4.8, "Lakeside evening Rajasthani folk dances and puppet artistry" },
        { 37, 3, "Monsoon Palace Sajjangarh",     "Nature",     120.0,  90, 900, 1800, 4.6, "Hilltop palace built to track monsoon clouds across the Aravalli range" },

        /* JODHPUR (id 4) */
        { 38, 4, "Mehrangarh Citadel",            "Historical", 200.0, 180, 900, 1700, 4.9, "Colossal cliffside fortress towering 400 feet above the indigo Blue City" },
        { 39, 4, "Jaswant Thada Cenotaph",        "Cultural",    50.0,  60, 900, 1700, 4.6, "Intricately carved translucent marble memorial by a quiet lake" },
        { 40, 4, "Umaid Bhawan Palace",           "Cultural",   100.0,  90, 900, 1700, 4.6, "One of the world's largest private residences with Art Deco royal museum" },
        { 41, 4, "Mandore Gardens",               "Historical",  50.0,  90, 800, 1800, 4.4, "Ancient royal cenotaphs and rock terraces of Marwar rulers" },

        /* VARANASI (id 5) */
        { 42, 5, "Dashashwamedh Ghat Ganga Aarti", "Religious",  0.0, 120, 500, 2100, 5.0, "Mesmerizing evening brass lamp ritual on the sacred river steps" },
        { 43, 5, "Kashi Vishwanath Corridor",    "Religious",   0.0,  90, 400, 2200, 4.9, "Revered Jyotirlinga temple connecting directly to the holy Ganges" },
        { 44, 5, "Sarnath Buddhist Deer Park",    "Historical", 100.0, 120, 800, 1700, 4.7, "Sacred site where Lord Buddha preached his first sermon after enlightenment" },
        { 45, 5, "Sunrise Boat Ride on Ganges",   "Nature",     300.0,  90, 530, 800, 4.9, "Peaceful morning rowing boat through misty heritage ghats of Varanasi" },

        /* MUMBAI (id 6) */
        { 46, 6, "Gateway of India",              "Historical",  0.0,  60, 600, 2300, 4.7, "Iconic seaside basalt triumphal arch facing the Arabian Sea" },
        { 47, 6, "Marine Drive Queen's Necklace", "Nature",      0.0,  90, 600, 2359, 4.8, "Sweeping coastal promenade celebrated for evening breezes and sunset skyline" },
        { 48, 6, "Elephanta Caves UNESCO",        "Historical", 260.0, 210, 900, 1700, 4.6, "Rock-cut cave temples dedicated to Shiva on an island reached by harbour ferry" },
        { 49, 6, "Chhatrapati Shivaji Terminus",  "Cultural",    0.0,  45, 600, 2200, 4.7, "Victorian Gothic UNESCO railway headquarters with gargoyles and stained glass" },
        { 50, 6, "Bandra Bandstand & Fort",       "Nature",      0.0,  75, 600, 2100, 4.6, "Rocky seashore walkway beside Portuguese fort and Bollywood star residences" },
        { 51, 6, "Juhu Chowpatty Beach Food",     "Food",       300.0,  90, 1600, 2359, 4.7, "Lively beach fair with pav bhaji, bhel puri, kulfi and sea waves" },

        /* GOA (id 7) */
        { 52, 7, "Basilica of Bom Jesus",         "Religious",   50.0,  60, 900, 1730, 4.7, "16th-century UNESCO Baroque church holding mortal remains of St. Francis Xavier" },
        { 53, 7, "Palolem Beach Crescent",        "Nature",      0.0, 180, 600, 2200, 4.8, "Idyllic crescent white-sand bay lined with coconut palms and beach huts" },
        { 54, 7, "Dudhsagar Waterfalls Trek",     "Adventure",  500.0, 240, 800, 1600, 4.8, "Towering 4-tiered milk-white cascade deep within the Western Ghats jungle" },
        { 55, 7, "Fort Aguada & Lighthouse",      "Historical",  50.0,  90, 900, 1800, 4.6, "17th-century Portuguese fortress overlooking Sinquerim beach and Arabian Sea" },
        { 56, 7, "Fontainhas Latin Quarter",      "Cultural",    0.0,  90, 800, 1900, 4.8, "Colourful Portuguese heritage lanes with azulejos tiles and indie cafes" },
        { 57, 7, "Anjuna Flea Market & Sunset",   "Cultural",    0.0, 120, 1000, 2000, 4.6, "Bohemian beachside shopping for handmade jewelry, crafts and live music" },

        /* MANALI (id 8) */
        { 58, 8, "Solang Valley Snow Sports",     "Adventure",  600.0, 210, 830, 1700, 4.8, "Premier adventure hub for paragliding, zorbing and winter skiing" },
        { 59, 8, "Hadimba Temple Forest Shrine",  "Religious",   30.0,  60, 800, 1800, 4.6, "Ancient multi-tiered wooden pagoda surrounded by towering cedar trees" },
        { 60, 8, "Rohtang Pass High Altitude",    "Adventure", 1000.0, 300, 700, 1600, 4.9, "Glacial pass at 13,058 ft offering breathtaking snow peaks" },
        { 61, 8, "Old Manali Riverside Cafes",    "Food",       450.0, 120, 900, 2200, 4.7, "Bohemian apple orchard village with trout fish dining and live acoustic music" },

        /* SHIMLA (id 9) */
        { 62, 9, "The Ridge & Mall Road",         "Cultural",    0.0, 120, 800, 2200, 4.7, "Pedestrian boulevard overlooking snow peaks with colonial Christ Church" },
        { 63, 9, "Jakhoo Hill & Temple Ropeway",  "Religious",  250.0,  90, 800, 1800, 4.6, "Highest hill peak with giant Lord Hanuman statue and cable car ride" },
        { 64, 9, "Kalka-Shimla Toy Train",        "Historical", 150.0, 180, 800, 1600, 4.8, "UNESCO heritage narrow-gauge rail journey traversing 102 mountain tunnels" },
        { 65, 9, "Kufri Snow Viewpoint",          "Adventure",  300.0, 120, 900, 1700, 4.5, "Popular hill outpost for horse riding, yak photography and snow slopes" },

        /* AMRITSAR (id 10) */
        { 66, 10, "Golden Temple (Harmandir Sahib)", "Religious", 0.0, 180, 400, 2300, 5.0, "Most sacred Sikh shrine plated with pure gold surrounded by holy Amrit Sarovar" },
        { 67, 10, "Wagah Border Beating Retreat",   "Cultural",   0.0, 120, 1500, 1830, 4.9, "Electrifying daily military ceremony at the India-Pakistan border gate" },
        { 68, 10, "Jallianwala Bagh Memorial",      "Historical",  0.0,  60, 630, 1930, 4.7, "Somber national historical garden commemorating the 1919 independence movement" },
        { 69, 10, "Amritsari Kulcha Food Trail",    "Food",      250.0,  60, 800, 1600, 4.9, "Clay oven baked crisp kulchas served with spiced chole and homemade butter" },

        /* RISHIKESH (id 11) */
        { 70, 11, "Ganga White Water Rafting",      "Adventure", 650.0, 180, 800, 1600, 4.9, "Exhilarating Class III & IV rapids along the scenic Himalayan Ganges river" },
        { 71, 11, "Triveni Ghat Evening Aarti",      "Religious",   0.0,  75, 1700, 2000, 4.9, "Sacred convergence ghat with chanting priests and floating brass leaf lamps" },
        { 72, 11, "Beatles Ashram (Chaurasi Kutia)", "Cultural",  150.0,  90, 900, 1630, 4.7, "Forest ashram where The Beatles composed the White Album, featuring murals" },
        { 73, 11, "Ram Jhula & Laxman Jhula",       "Cultural",    0.0,  60, 600, 2200, 4.6, "Iconic suspension bridges spanning the turquoise Ganges with mountain views" },

        /* BENGALURU (id 14) */
        { 74, 14, "Lalbagh Botanical Garden",       "Nature",      50.0, 120, 600, 1900, 4.7, "240-acre botanical haven with century-old trees and Victorian glasshouse" },
        { 75, 14, "Bangalore Palace",               "Historical", 250.0, 120, 1000, 1730, 4.5, "Tudor-style royal estate with fortified towers, wood carvings and gardens" },
        { 76, 14, "Cubbon Park & Vidhana Soudha",   "Nature",       0.0,  90, 600, 1900, 4.7, "Sprawling green heart of Bengaluru facing the Neo-Dravidian statehouse" },
        { 77, 14, "VV Puram Food Street",           "Food",       300.0,  90, 1730, 2300, 4.8, "Sensory vegetarian street food trail with dosas, obbattu and rose gulkand" },

        /* HYDERABAD (id 15) */
        { 78, 15, "Charminar & Laad Bazaar",        "Historical",  35.0,  90, 930, 1730, 4.7, "16th-century landmark with 4 grand minarets flanked by lacquer bangle lanes" },
        { 79, 15, "Golconda Fort & Light Show",     "Historical",  80.0, 150, 900, 1730, 4.8, "Acoustic medieval diamond citadel where the Koh-i-Noor diamond was kept" },
        { 80, 15, "Hussain Sagar Lake & Buddha",    "Nature",      50.0,  75, 800, 2100, 4.6, "Heart-shaped lake featuring world's tallest monolithic standing Buddha" },
        { 81, 15, "Paradise Hyderabadi Biryani",    "Food",       450.0,  75, 1100, 2300, 4.9, "World-famed Dum Biryani cooked with fragrant basmati and saffron spices" },

        /* KOCHI (id 16) */
        { 82, 16, "Chinese Fishing Nets Fort Kochi","Cultural",    0.0,  60, 600, 2100, 4.7, "Historic cantilevered shore nets silhouetted against Arabian Sea sunsets" },
        { 83, 16, "Mattancherry Dutch Palace",      "Historical",  25.0,  75, 945, 1700, 4.5, "16th-century Portuguese palace featuring exquisite Hindu mythological murals" },
        { 84, 16, "Jew Town & Paradesi Synagogue",  "Cultural",    20.0,  60, 1000, 1700, 4.6, "Antique shops and 1568 synagogue with hand-painted Chinese porcelain tiles" },
        { 85, 16, "Kerala Backwaters Day Cruise",   "Nature",     800.0, 210, 830, 1700, 4.9, "Serene canal voyage on traditional kettuvallam through palm backwaters" },

        /* KOLKATA (id 17) */
        { 86, 17, "Victoria Memorial Hall",         "Historical",  50.0, 120, 1000, 1700, 4.8, "Monumental white Makrana marble palace surrounded by 64 acres of gardens" },
        { 87, 17, "Howrah Bridge & Hooghly Walk",   "Cultural",    0.0,  60, 500, 2300, 4.8, "Cantilevered engineering marvel spanning the busy waters of Hooghly River" },
        { 88, 17, "Dakshineswar Kali Temple",       "Religious",   0.0,  90, 600, 2030, 4.9, "19th-century navaratna temple where mystic Ramakrishna Paramahamsa served" },
        { 89, 17, "Park Street Dining & Sweets",    "Food",       400.0,  90, 1100, 2300, 4.9, "Iconic colonial dining avenue with kathi rolls, chelo kababs and rasgullas" },

        /* PORT BLAIR & ANDAMANS (id 12) */
        { 90, 12, "Cellular Jail National Memorial", "Historical", 50.0, 120, 900, 1700, 4.9, "Historic colonial prison complex renowned for its poignant evening light and sound show" },
        { 91, 12, "Radhanagar Beach Havelock",       "Nature",      0.0, 180, 600, 1830, 4.9, "Acclaimed as one of Asia's most pristine white-sand beaches with turquoise lagoon waters" },
        { 92, 12, "Ross Island British Ruins",       "Historical", 100.0, 120, 830, 1630, 4.7, "Old British administrative headquarters overtaken by giant banyan tree roots and deer" },
        { 93, 12, "Elephant Beach Coral Reef",       "Adventure",  750.0, 180, 800, 1600, 4.8, "Vibrant coral reef hub renowned for sea walking, glass-bottom boats and snorkeling" },

        /* BENGALURU EXPANDED (id 14) */
        { 94, 14, "Bannerghatta Biological Park",    "Nature",     350.0, 210, 930, 1700, 4.6, "Sprawling safari sanctuary with tigers, lions, butterfly conservatory and rescue center" },
        { 95, 14, "Nandi Hills Sunrise Point",       "Adventure",   50.0, 180, 600, 1800, 4.7, "Ancient hill fortress shrouded in mist, offering spectacular golden sunrise panoramas" },
        { 96, 14, "Tipu Sultan's Summer Palace",     "Historical",  50.0,  60, 830, 1730, 4.4, "Ornate teakwood palace with fluted pillars, frescoes and historical Anglo-Mysore exhibits" },

        /* HYDERABAD EXPANDED (id 15) */
        { 97, 15, "Salar Jung Museum",               "Cultural",   100.0, 180, 1000, 1700, 4.8, "One of the world's largest one-man collections featuring the Veiled Rebecca and musical clock" },
        { 98, 15, "Qutb Shahi Tombs",                "Historical",  50.0,  90, 930, 1730, 4.7, "Majestic domed royal mausoleums set in landscaped heritage gardens of Ibrahim Bagh" },
        { 99, 15, "Ramoji Film City",                "Adventure", 1350.0, 360, 900, 1730, 4.6, "World's largest film studio complex featuring themed movie sets, stunt shows and eco tour" },

        /* KOCHI EXPANDED (id 16) */
        { 100, 16, "Fort Kochi Beach & Promenade",   "Nature",       0.0,  90, 600, 2200, 4.6, "Historic seaside promenade with heritage trees, sunset viewpoints and maritime history" },
        { 101, 16, "Kerala Kathakali Centre",        "Cultural",   400.0,  90, 1700, 2000, 4.9, "Classical dance theatre showcasing elaborate makeup rituals, mudras and Kathakali recitals" },
        { 102, 16, "Hill Palace Museum Tripunithura","Historical",  50.0, 120, 900, 1630, 4.5, "Largest archaeological palace museum in Kerala with royal crown, carriage and deer park" },

        /* KOLKATA EXPANDED (id 17) */
        { 103, 17, "Indian Museum Kolkata",          "Cultural",    75.0, 150, 1000, 1700, 4.7, "Ninth oldest museum in the world with rare Egyptian mummies, fossils and Mughal art" },
        { 104, 17, "Princep Ghat & River Cruise",    "Nature",     150.0,  90, 600, 2200, 4.8, "Grecian monument along Hooghly River offering traditional wooden boat sunset rides" },
        { 105, 17, "College Street Book Alley",      "Cultural",   200.0,  90, 1000, 2030, 4.9, "World's largest secondhand book market surrounding the legendary Indian Coffee House" },

        /* PUNE EXPANDED (id 13) */
        { 106, 13, "Lonavala & Karla Buddhist Caves","Historical",  60.0, 180, 830, 1700, 4.7, "Ancient 2nd-century BC rock-cut chaitya hall with vaulted ceiling and grand carved pillars" },
        { 107, 13, "National Defence Academy Khadakwasla","Cultural", 0.0, 90, 900, 1600, 4.8, "Premier joint military training academy nestled in picturesque Sahyadri foothills" },

        /* MUMBAI EXPANDED (id 6) */
        { 108, 6, "Sanjay Gandhi National Park & Caves","Nature",   90.0, 210, 730, 1730, 4.6, "Dense protected urban forest housing 109 rock-cut Buddhist caves dating from 1st century BC" },
        { 109, 6, "Haji Ali Dargah Coastal Shrine",  "Religious",    0.0,  60, 600, 2200, 4.7, "Revered 15th-century Indo-Islamic mosque located on an offshore islet in Arabian Sea" },

        /* GOA EXPANDED (id 7) */
        { 110, 7, "Sahakari Spice Farm Ponda",       "Nature",     500.0, 120, 900, 1630, 4.7, "Lush organic plantation tour featuring vanilla, cinnamon, cardamom and Goan buffet" },
        { 111, 7, "Chapora Fort Sunset",             "Historical",   0.0,  90, 800, 1900, 4.6, "Iconic hilltop fortification made famous in pop culture with views over Vagator beach" },

        /* DELHI EXPANDED (id 0) */
        { 112, 0, "Lodhi Art District & Gardens",    "Cultural",     0.0,  90, 600, 2000, 4.7, "Open-air street art murals alongside heritage 15th-century Sayyid and Lodhi tombs" },
        { 113, 0, "Dilli Haat Crafts & Cuisines",    "Cultural",    50.0, 120, 1030, 2200, 4.7, "Bustling open-air crafts bazaar with artisan stalls and culinary treats from every state" },

        /* JAIPUR EXPANDED (id 2) */
        { 114, 2, "Jaigarh Fort & Jaivana Cannon",   "Historical", 150.0, 120, 900, 1700, 4.6, "Rugged hilltop fortress connected to Amber Fort housing the world's largest wheeled cannon" },
        { 115, 2, "Galta Ji Monkey Temple",          "Religious",    0.0,  90, 600, 1900, 4.5, "Ancient Hindu pilgrimage complex with sacred natural water tanks set in a mountain pass" },

        /* UDAIPUR EXPANDED (id 3) */
        { 116, 3, "Fateh Sagar Lake & Nehru Park",   "Nature",     150.0,  90, 800, 2100, 4.7, "Artificial freshwater lake featuring an island garden park accessible by boat" },
        { 117, 3, "Jagdish Temple Old City",         "Religious",    0.0,  60, 530, 2200, 4.8, "Striking 1651 Indo-Aryan temple with towering carved shikhara in heart of old Udaipur" },

        /* RISHIKESH EXPANDED (id 11) */
        { 118, 11, "Neelkanth Mahadev Temple",       "Religious",    0.0, 120, 600, 1900, 4.7, "Revered Shiva shrine nestled in dense mountain valleys at 4,360 ft" },
        { 119, 11, "Kunjapuri Devi Sunrise Trek",    "Adventure",  200.0, 150, 500, 1100, 4.9, "Panoramic vantage point offering views of high Himalayan peaks like Swargarohini" },

        /* AMRITSAR EXPANDED (id 10) */
        { 120, 10, "Partition Museum Town Hall",     "Cultural",    20.0, 120, 1000, 1800, 4.8, "World's first museum dedicated to the poignant stories and artifacts of the 1947 partition" },
        { 121, 10, "Gobindgarh Fort & Live Shows",   "Historical", 180.0, 150, 1000, 2200, 4.6, "18th-century military fort holding the Toshakhana treasury and evening light spectacles" }
    };

    data->numAttractions = sizeof(attrs) / sizeof(attrs[0]);
    for (int i = 0; i < data->numAttractions; i++) {
        data->attractions[i] = attrs[i];
    }

    /* -------------------------------------------------------------
       4. HOTELS (Tripled sample data across all tiers in INR ₹)
       ------------------------------------------------------------- */
    Hotel hts[] = {
        /* PUNE (id 13) - 8 Diverse Options */
        { 1,  13, "Zostel Pune (Koregaon Park)", "Budget",   1200.0, 4.5, 3.2, "Free Wi-Fi, Rooftop Cafe, AC, Community Lounge, Work Desk" },
        { 2,  13, "FabHotel Prime Shivaji Nagar", "Budget",   1800.0, 4.3, 1.2, "Clean AC Rooms, Free Breakfast, Walking to FC Road" },
        { 3,  13, "Lemon Tree Hotel Hinjawadi",   "Standard", 4200.0, 4.6, 9.5, "Outdoor Pool, Fitness Center, Citrus Cafe, High-speed Wi-Fi" },
        { 4,  13, "Pride Hotel Shivaji Nagar",     "Standard", 4800.0, 4.5, 1.5, "Central Location, Multi-cuisine Dining, Banquet, Free Breakfast" },
        { 5,  13, "Royal Orchid Central Pune",    "Standard", 5400.0, 4.6, 4.0, "Kalyani Nagar Location, Swimming Pool, Pinxx Restaurant" },
        { 6,  13, "The Ritz-Carlton Pune",        "Luxury",  18500.0, 4.9, 4.5, "Golf Course Views, Luxury Spa, Ukiyo Japanese Dining, Butler" },
        { 7,  13, "JW Marriott Hotel Senapati Bapat","Luxury",14000.0, 4.9, 2.5, "Rooftop Lounge, 5 Award-winning Restaurants, Quan Spa, Pool" },
        { 8,  13, "Conrad Pune by Hilton",        "Luxury",  16500.0, 4.8, 3.8, "Art Deco Luxury, Temperature Controlled Pool, 6 Dining Venues" },

        /* DELHI (id 0) */
        { 9,  0,  "Zostel Delhi",                 "Budget",   1300.0, 4.4, 2.5, "Free Wi-Fi, Rooftop Cafe, AC, Lockers" },
        { 10, 0,  "Bloomrooms @ New Delhi",       "Standard", 4500.0, 4.5, 1.8, "Complimentary Breakfast, Work Desk, Cloud Beds" },
        { 11, 0,  "The Imperial New Delhi",       "Luxury",  19500.0, 4.9, 0.8, "Heritage Spa, Outdoor Pool, Fine Dining, Butler Service" },
        { 12, 0,  "The Leela Palace New Delhi",   "Luxury",  24000.0, 5.0, 3.0, "Rooftop Infinity Pool, Royal Suites, Espa Wellness" },

        /* AGRA (id 1) */
        { 13, 1,  "Moustache Agra",               "Budget",   1100.0, 4.3, 1.2, "Taj View Terrace, Cafe, Social Lounge" },
        { 14, 1,  "Tajview IHCL SeleQtions",      "Standard", 5200.0, 4.6, 2.0, "Pool, Taj View Rooms, Multi-cuisine Restaurant" },
        { 15, 1,  "The Oberoi Amarvilas",         "Luxury",  32000.0, 5.0, 0.6, "Direct Taj Mahal views from every room, Royal Spa" },
        { 16, 1,  "ITC Mughal Agra",              "Luxury",  12500.0, 4.7, 2.5, "35 Acres Mughal Gardens, Kaya Kalp Spa, Pool" },

        /* JAIPUR (id 2) */
        { 17, 2,  "Backpacker Panda Jaipur",      "Budget",   1100.0, 4.3, 3.0, "Wi-Fi, Shared Kitchen, Air Conditioning" },
        { 18, 2,  "Alsisar Haveli Heritage",      "Standard", 5500.0, 4.6, 1.5, "Heritage Architecture, Courtyard Pool, Folk Shows" },
        { 19, 2,  "Rambagh Palace",               "Luxury",  28000.0, 5.0, 2.2, "Former Maharaja Palace, Peacocks in Gardens, Luxury Spa" },
        { 20, 2,  "Jai Mahal Palace",             "Luxury",  16000.0, 4.8, 2.0, "18-Acre Mughal Gardens, Palace Architecture, Polo Bar" },

        /* UDAIPUR (id 3) */
        { 21, 3,  "Bunkyard Hostel Lakefront",    "Budget",   1400.0, 4.5, 1.0, "Lakefront Terrace, Music Nights, Cafe" },
        { 22, 3,  "Amet Haveli Heritage",         "Standard", 6500.0, 4.7, 0.8, "Lakeside Dining Ambrai, Rajput Architecture, Pool" },
        { 23, 3,  "Taj Lake Palace",              "Luxury",  34000.0, 5.0, 0.5, "18th-century floating marble palace on Lake Pichola" },
        { 24, 3,  "The Leela Palace Udaipur",     "Luxury",  31000.0, 4.9, 0.7, "Lake Pichola Views, Royal Boat Arrival, Luxury Spa" },

        /* JODHPUR (id 4) */
        { 25, 4,  "Hostel LaVie Jodhpur",         "Budget",   1200.0, 4.4, 1.5, "Fort View Rooftop, Yoga, Wi-Fi" },
        { 26, 4,  "Pal Haveli Heritage",          "Standard", 4600.0, 4.6, 1.1, "Heritage Rooms, Rooftop Restaurant overlooking Fort" },
        { 27, 4,  "Umaid Bhawan Palace Hotel",    "Luxury",  35000.0, 5.0, 3.5, "Royal Art Deco Residence, Champagne Breakfast" },

        /* VARANASI (id 5) */
        { 28, 5,  "Stops Hostel Varanasi",        "Budget",   1200.0, 4.3, 2.0, "Courtyard, Walking Tours, Hindi Lessons" },
        { 29, 5,  "BrijRama Palace on Ghat",      "Standard", 8500.0, 4.8, 0.2, "Historic Ghat Palace, Classical Sitar, River Views" },
        { 30, 5,  "Taj Ganges Varanasi",          "Luxury",  14500.0, 4.8, 3.8, "12 Acres Greenery, Luxury Wellness, Gourmet Dining" },

        /* MUMBAI (id 6) */
        { 31, 6,  "Cohostel Bandra",              "Budget",   1800.0, 4.3, 8.0, "Trendy Suburb Location, Wi-Fi, Social Hub" },
        { 32, 6,  "Residency Hotel Fort",         "Standard", 5500.0, 4.5, 1.2, "Heritage Quarter Walkability, Gourmet Breakfast" },
        { 33, 6,  "The Taj Mahal Palace",         "Luxury",  26000.0, 5.0, 0.1, "Sea-facing Landmark, Signature Butler, Historic Lobby" },
        { 34, 6,  "Trident Nariman Point",        "Luxury",  15500.0, 4.8, 1.0, "Marine Drive Views, High Floor Dining, Outdoor Pool" },

        /* GOA (id 7) */
        { 35, 7,  "Wanderers Hostel Goa",         "Budget",   1300.0, 4.4, 0.5, "Swimming Pool, Beach Proximity, Garden Hammocks" },
        { 36, 7,  "The Figueiredo House",         "Standard", 5200.0, 4.6, 4.0, "Grand Portuguese Estate, Curated Dining, Countryside" },
        { 37, 7,  "Taj Exotica Resort & Spa",     "Luxury",  22000.0, 4.8, 0.3, "Mediterranean-inspired 56-acre Sanctuary, Private Beach" },
        { 38, 7,  "W Goa Vagator",                "Luxury",  21000.0, 4.7, 0.8, "Cliffside Luxury, Rock Pool, Sunset Lounge" },

        /* MANALI (id 8) */
        { 39, 8,  "Alt Life Old Manali",          "Budget",   1200.0, 4.4, 2.0, "Riverside Vibe, Live Acoustic Music, High-speed Wi-Fi" },
        { 40, 8,  "The Himalayan Castle Resort",  "Standard", 6200.0, 4.7, 1.5, "Victorian Castle Architecture, Apple Orchards, Pool" },
        { 41, 8,  "Span Resort & Spa River Beas", "Luxury",  18000.0, 4.8, 5.0, "Helipad, River Beas Frontage, Luxury Cottage Suites" },

        /* SHIMLA (id 9) */
        { 42, 9,  "The Hosteller Shimla",         "Budget",   1400.0, 4.3, 1.0, "Mountain Balconies, Game Room, Cafe" },
        { 43, 9,  "Clarkes Hotel Mall Road",      "Standard", 5800.0, 4.6, 0.5, "Heritage Colonial Dining, Direct Mall Road Access" },
        { 44, 9,  "Wildflower Hall Oberoi",       "Luxury",  27000.0, 5.0, 8.0, "Former Residence of Lord Kitchener, Cedar Forest, Whirlpool" },

        /* AMRITSAR (id 10) */
        { 45, 10, "City Backpackers Amritsar",    "Budget",   1000.0, 4.2, 0.8, "5-min Walk to Golden Temple, Clean Dorms" },
        { 46, 10, "Radisson Blu Amritsar",        "Standard", 4800.0, 4.5, 4.0, "Modern Comfort, Airport Shuttle, Pool" },
        { 47, 10, "Taj Swarna Amritsar",          "Luxury",  13500.0, 4.8, 3.2, "Luxury Spa, Fine Dining Punjabi Cuisine, Infinity Pool" },

        /* RISHIKESH (id 11) */
        { 48, 11, "Bunk Stay Rishikesh",          "Budget",   1100.0, 4.5, 0.6, "Rooftop Yoga, River Panorama, Organic Cafe" },
        { 49, 11, "Aloha On The Ganges",          "Standard", 6500.0, 4.6, 1.5, "Direct River Access, Infinity Pool, Ayurvedic Spa" },
        { 50, 11, "Ananda in the Himalayas",      "Luxury",  38000.0, 5.0, 9.0, "World-renowned Maharaja Wellness Palace, Vedanta, Hydrotherapy" },

        /* BENGALURU (id 14) */
        { 51, 14, "Zostel Bengaluru Indiranagar", "Budget",   1400.0, 4.4, 4.0, "Vibrant Craft Beer Hub, High-speed Wi-Fi, Rooftop" },
        { 52, 14, "The Chancery Pavilion",        "Standard", 4900.0, 4.5, 1.0, "Centrally Located, Rooftop Pool, Multi-cuisine Buffet" },
        { 53, 14, "The Leela Palace Bengaluru",   "Luxury",  19000.0, 5.0, 5.0, "Royal Vijayanagara Architecture, 7 Acres Lush Gardens" },

        /* HYDERABAD (id 15) */
        { 54, 15, "Aditya Hometel Ameerpet",      "Budget",   1500.0, 4.3, 3.0, "Modern Rooms, Breakfast, Business Center" },
        { 55, 15, "Marigold Hotel Greenlands",    "Standard", 5200.0, 4.6, 2.5, "Spa, City Views, Pool, Fine Dining" },
        { 56, 15, "Taj Falaknuma Palace",         "Luxury",  36000.0, 5.0, 6.0, "1894 Scorpion-shaped Palace, Horse Carriage Arrival" },

        /* KOCHI (id 16) */
        { 57, 16, "Zostel Kochi Fort Kochi",      "Budget",   1200.0, 4.5, 0.8, "Heritage Quarter, Walking to Beach & Nets, Cafe" },
        { 58, 16, "Old Harbour Hotel",            "Standard", 6200.0, 4.7, 0.5, "300-year-old Portuguese Garden Estate, Ayurvedic Pool" },
        { 59, 16, "Brunton Boatyard CGH Earth",   "Luxury",  17500.0, 4.9, 0.2, "Restored Victorian Shipyard, Harbour Views, History" },

        /* KOLKATA (id 17) */
        { 60, 17, "Backpackers Park Kolkata",     "Budget",   1100.0, 4.3, 1.5, "Near Park Street, Clean Bunks, Wi-Fi" },
        { 61, 17, "The Peerless Inn Kolkata",     "Standard", 4600.0, 4.5, 0.8, "Esplanade Central, Authentic Bengali Aaheli Dining" },
        { 62, 17, "The Oberoi Grand Kolkata",     "Luxury",  18000.0, 4.9, 0.5, "The Grande Dame of Chowringhee, Colonial Luxury" },

        /* PORT BLAIR & ANDAMANS (id 12) */
        { 63, 12, "SeaShell Port Blair",                 "Standard", 5800.0, 4.6, 2.0, "Sea Facing Rooms, Rooftop Bar, Seafood Restaurant, Dive Booking" },
        { 64, 12, "Taj Exotica Resort & Spa Andamans",   "Luxury",  34000.0, 5.0, 1.2, "Radhanagar Bay Frontage, Luxury Stilted Villas, Olympic Pool, Spa" },
        { 65, 12, "Symphony Palms Beach Resort",         "Standard", 4800.0, 4.5, 1.8, "Private White Sand Shore, Spa, Multi-cuisine Cafe, Snorkel Tours" },
        { 66, 12, "Hotel Shompen Port Blair",            "Budget",   1800.0, 4.2, 1.5, "Centrally Located, Free Breakfast, AC Rooms, Tour Assistance" },

        /* PUNE EXPANDED (id 13) */
        { 67, 13, "Hyatt Pune (Kalyani Nagar)",          "Luxury",  11500.0, 4.8, 3.5, "Outdoor Swimming Pool, Baan Tao Pan-Asian, Luxury Spa, Serene Garden" },
        { 68, 13, "Sheraton Grand Pune Bund Garden",     "Luxury",  13000.0, 4.8, 2.0, "Majestic Heritage Decor, Rooftop Bar Chingari, Swimming Pool, Spa" },
        { 69, 13, "The Central Park Hotel",              "Standard", 4500.0, 4.4, 1.8, "Near Pune Station, Time After Time Restaurant, Rooftop Pool, Gym" },
        { 70, 13, "Treebo Trend Deccan Royale",          "Budget",   1900.0, 4.3, 1.0, "Heart of Deccan Gymkhana, Free Breakfast, Fast Wi-Fi, AC Rooms" },

        /* BENGALURU EXPANDED (id 14) */
        { 71, 14, "Taj West End Bengaluru",              "Luxury",  22000.0, 5.0, 1.5, "20 Acres Heritage Forest, Colonial Suites, Mynt Cafe, Blue Ginger" },
        { 72, 14, "ITC Gardenia Bengaluru",              "Luxury",  17500.0, 4.9, 0.8, "LEED Platinum Certified, Luxury Spa, Cubbon Pavilion, Helipad" },
        { 73, 14, "Ibis Bengaluru City Centre",          "Standard", 3900.0, 4.4, 0.5, "Overlooking Cubbon Park, Spice It Cafe, 24/7 Fitness, Modern Rooms" },
        { 74, 14, "Locul Central Koramangala",           "Budget",   1200.0, 4.4, 3.5, "Co-working Space, Rooftop Cafe, Social Dorms, Free Wi-Fi" },

        /* HYDERABAD EXPANDED (id 15) */
        { 75, 15, "Park Hyatt Hyderabad (Banjara Hills)","Luxury",  16000.0, 4.9, 3.0, "Opulent Atrium, The Living Room, Outdoor Pool, Luxury Residence Suites" },
        { 76, 15, "ITC Kakatiya Begumpet",               "Luxury",  12500.0, 4.7, 2.8, "Deccan Mughal Architecture, Dakshin Restaurant, Wellness Spa" },
        { 77, 15, "Mercure Hyderabad KCP Banjara Hills", "Standard", 4600.0, 4.5, 2.0, "Hussain Sagar Views, Rooftop Lounge, Cayenne All-day Dining" },
        { 78, 15, "Shepherd Stories Gachibowli",         "Budget",   1100.0, 4.3, 4.5, "Boutique Hostel, Terrace Garden, Wi-Fi, High-speed Co-working" },

        /* KOCHI EXPANDED (id 16) */
        { 79, 16, "Grand Hyatt Kochi Bolgatty",          "Luxury",  18500.0, 5.0, 2.5, "Lake Vembanad Island Waterfront, Santata Spa, Infinity Marina Pools" },
        { 80, 16, "Taj Malabar Resort & Spa Willingdon", "Luxury",  15000.0, 4.8, 1.8, "Harbour Views, Dolphin Watching Cruises, Rice Boat Restaurant" },
        { 81, 16, "Forte Kochi Heritage Hotel",          "Standard", 6800.0, 4.7, 0.4, "Jew Street Proximity, Colonial Dutch Architecture, Swimming Pool" },
        { 82, 16, "Santa Maria Hostel Fort Kochi",       "Budget",   1100.0, 4.4, 0.6, "Heritage Courtyard, Bicycle Rentals, Walking to Beach, Free Wi-Fi" },

        /* KOLKATA EXPANDED (id 17) */
        { 83, 17, "ITC Royal Bengal Kolkata",            "Luxury",  16500.0, 4.9, 4.0, "Grand Aristocratic Architecture, Royal Vega Dining, Kaya Kalp Spa" },
        { 84, 17, "Taj Bengal Kolkata (Alipore)",        "Luxury",  15500.0, 4.8, 3.2, "Lush Atrium Garden, Sonargaon Bengali Restaurant, Outdoor Pool" },
        { 85, 17, "Kenilworth Hotel Kolkata",            "Standard", 5100.0, 4.5, 1.0, "Near Victoria Memorial, Aromas Cafe, Victorian Pub, Spa" },
        { 86, 17, "Offbeat CCU Bunkers Kolkata",         "Budget",   1000.0, 4.3, 3.5, "Rooftop Sports Arena, High-speed Wi-Fi, Trendy Cafe, Bunks" },

        /* DELHI EXPANDED (id 0) */
        { 87, 0,  "Taj Mahal Hotel New Delhi (Mansingh)","Luxury",  21000.0, 4.9, 1.2, "Mughal Architecture, House of Ming, Rick's Bar, Luxury Spa" },
        { 88, 0,  "The Claridges New Delhi",             "Standard", 7500.0, 4.7, 1.0, "Historic Colonial Bungalow, Dhaba Restaurant, Lush Green Lawns" },
        { 89, 0,  "Joey's Hostel New Delhi",             "Budget",   1100.0, 4.4, 3.0, "Yamuna Riverside, Terrace Lounges, Social Events, Free Wi-Fi" },

        /* AGRA EXPANDED (id 1) */
        { 90, 1,  "Courtyard by Marriott Agra",          "Standard", 4800.0, 4.5, 3.0, "Outdoor Pool, MoMo Cafe, Anise Indian Dining, Fitness Center" },
        { 91, 1,  "Bedweiser Backpackers Agra",          "Budget",    950.0, 4.2, 1.8, "Rooftop Taj Views, Game Room, Cafe, Shared Kitchen" },

        /* JAIPUR EXPANDED (id 2) */
        { 92, 2,  "Samode Haveli Jaipur",                "Luxury",  19500.0, 4.9, 1.8, "175-year-old Royal Mansion, Courtyard Jacuzzi Pool, Fine Dining" },
        { 93, 2,  "Shahpura Haveli Heritage",            "Standard", 5200.0, 4.6, 2.5, "Traditional Frescoes, Rooftop Restaurant, Rajput Hospitality" },
        { 94, 2,  "Horn OK Please Hostel Jaipur",        "Budget",   1000.0, 4.5, 2.0, "Design-led Hostel, Terrace Lounge, Clean AC Dorms, Wi-Fi" },

        /* UDAIPUR EXPANDED (id 3) */
        { 95, 3,  "The Oberoi Udaivilas",                "Luxury",  39000.0, 5.0, 1.5, "Moated Palatial Grandeur, Domes, Peacocks, Sunset Boat Rides" },
        { 96, 3,  "Fateh Garh Heritage Sanctuary",       "Standard", 6900.0, 4.7, 4.0, "Heritage Perched on Hilltop, Vintage Car Museum, Dual Pools" },
        { 97, 3,  "Moustache Udaipur Lakefront",         "Budget",   1200.0, 4.4, 0.9, "Terrace Overlooking Pichola, Puppet Shows, Community Kitchen" },

        /* JODHPUR EXPANDED (id 4) */
        { 98, 4,  "RAAS Jodhpur Stepwell Palace",        "Luxury",  22000.0, 4.9, 0.5, "Haveli at Base of Mehrangarh, Stepwell Dining, Luxury Pool" },
        { 99, 4,  "Ranbanka Palace Heritage",            "Standard", 4900.0, 4.5, 2.2, "Colonial Marwar Splendor, Polo Heritage Bar, Garden Courtyard" },
        { 100, 4, "CrashPad Hostel Jodhpur",             "Budget",    900.0, 4.3, 1.8, "Indigo Blue City Views, Rooftop Chai, Free Wi-Fi, AC Dorms" },

        /* VARANASI EXPANDED (id 5) */
        { 101, 5, "Nadesar Palace by Taj",               "Luxury",  29000.0, 5.0, 4.2, "Historic 1835 Royal Retreat, Horse Carriage, Mango Orchards" },
        { 102, 5, "Amritara Suryauday Haveli",           "Standard", 6800.0, 4.7, 0.1, "Direct Shivala Ghat Frontage, Sunrise Yoga Terrace, Heritage Rooms" },
        { 103, 5, "Moustache Varanasi Ghats",            "Budget",   1000.0, 4.4, 0.5, "Near Assi Ghat, Rooftop Ganga View, Travel Desk, Free Wi-Fi" },

        /* MUMBAI EXPANDED (id 6) */
        { 104, 6, "Four Seasons Hotel Mumbai (Worli)",   "Luxury",  17000.0, 4.8, 4.5, "AER Rooftop Bar, Arabian Sea Panorama, Outdoor Heated Pool" },
        { 105, 6, "Fariyas Hotel Colaba",                "Standard", 6200.0, 4.5, 0.5, "Walking to Gateway of India, Tavern Bar, Pool, Harbor Views" },
        { 106, 6, "Horn OK Please Hostel Bandra",        "Budget",   1500.0, 4.5, 6.0, "100-year-old Portuguese Bungalow, Hip Bandra Hub, AC Rooms" },

        /* GOA EXPANDED (id 7) */
        { 107, 7, "Alila Diwa Goa (Majorda)",            "Luxury",  16000.0, 4.8, 1.0, "Infinity Pool over Paddy Fields, Vivo Restaurant, Spa Alila" },
        { 108, 7, "Vivenda Dos Palhacos",                "Standard", 5800.0, 4.7, 2.0, "Restored 1929 Estate, Heritage Bar in Lorry Front, Plunge Pool" },
        { 109, 7, "The Funky Monkey Hostel Anjuna",      "Budget",   1000.0, 4.4, 0.8, "Tropical Garden, Hammocks, Beach Walkable, Weekly BBQs" },

        /* MANALI EXPANDED (id 8) */
        { 110, 8, "Solang Valley Resort Manali",         "Standard", 5500.0, 4.6, 7.0, "Snow Peak Vistas, Dhaula Dhar Restaurant, Riverfront Lawns" },
        { 111, 8, "Moustache Manali Rohtang Way",        "Budget",   1100.0, 4.3, 2.5, "Himalayan Forest Trails, Bonfire Nights, High-speed Wi-Fi" },

        /* SHIMLA EXPANDED (id 9) */
        { 112, 9, "The Oberoi Cecil Shimla",             "Luxury",  24000.0, 4.9, 1.8, "Grand Heritage Ballroom, Cedar Garden, Indoor Heated Pool, Spa" },
        { 113, 9, "Woodville Palace Heritage",           "Standard", 5400.0, 4.5, 1.2, "Former Raja Residence, Vintage Hollywood Bar, Pine Woods" },

        /* AMRITSAR EXPANDED (id 10) */
        { 114, 10, "Hyatt Regency Amritsar",             "Luxury",  10500.0, 4.7, 3.5, "Vitality Pool, Punjab Province Restaurant, Luxury Spa" },
        { 115, 10, "Ramada by Wyndham Amritsar",         "Standard", 4600.0, 4.5, 0.9, "Walking to Golden Temple, Rooftop Pool, Multi-cuisine Cafe" },

        /* RISHIKESH EXPANDED (id 11) */
        { 116, 11, "Taj Rishikesh Resort & Spa",         "Luxury",  32000.0, 4.9, 8.5, "Private Ganges Beachfront, Ganga Aarti on Steps, Jiva Spa" },
        { 117, 11, "Live Free Hostel Rishikesh",         "Budget",   1000.0, 4.4, 0.8, "Rooftop Cafe, Yoga Sessions, Mountain Views, High-speed Wi-Fi" }
    };

    data->numHotels = sizeof(hts) / sizeof(hts[0]);
    for (int i = 0; i < data->numHotels; i++) {
        data->hotels[i] = hts[i];
    }
}

const Location* findLocationById(const MasterData* data, int id) {
    if (!data) return NULL;
    for (int i = 0; i < data->graph.numLocations; i++) {
        if (data->graph.locations[i].id == id) {
            return &data->graph.locations[i];
        }
    }
    return NULL;
}

const Location* findLocationByName(const MasterData* data, const char* name) {
    if (!data || !name) return NULL;
    for (int i = 0; i < data->graph.numLocations; i++) {
        if (strcasecmp(data->graph.locations[i].name, name) == 0) {
            return &data->graph.locations[i];
        }
    }
    return NULL;
}

const Attraction* findAttractionById(const MasterData* data, int id) {
    if (!data) return NULL;
    for (int i = 0; i < data->numAttractions; i++) {
        if (data->attractions[i].id == id) {
            return &data->attractions[i];
        }
    }
    return NULL;
}

const Hotel* findHotelById(const MasterData* data, int id) {
    if (!data) return NULL;
    for (int i = 0; i < data->numHotels; i++) {
        if (data->hotels[i].id == id) {
            return &data->hotels[i];
        }
    }
    return NULL;
}
