/**
 * data.c - Master dataset initialization and lookup functions.
 * 
 * Part of Trip Planner Web Application
 * Core backend implemented in C (DSA)
 */

#include "data.h"

/* Helper to add an undirected road connection between two locations */
static void addRoadEdge(Graph* g, int srcId, int destId, double distKm, int timeMin, double toll) {
    if (srcId < 0 || srcId >= g->numLocations || destId < 0 || destId >= g->numLocations) {
        return;
    }
    /* Edge from src to dest */
    EdgeNode* e1 = (EdgeNode*)malloc(sizeof(EdgeNode));
    e1->destinationId = destId;
    e1->distanceKm = distKm;
    e1->travelTimeMin = timeMin;
    e1->tollCost = toll;
    e1->next = g->adjList[srcId];
    g->adjList[srcId] = e1;

    /* Edge from dest to src (undirected road) */
    EdgeNode* e2 = (EdgeNode*)malloc(sizeof(EdgeNode));
    e2->destinationId = srcId;
    e2->distanceKm = distKm;
    e2->travelTimeMin = timeMin;
    e2->tollCost = toll;
    e2->next = g->adjList[destId];
    g->adjList[destId] = e2;
}

void initMasterData(MasterData* data) {
    memset(data, 0, sizeof(MasterData));
    Graph* g = &data->graph;

    /* Initialize adjacency list pointers */
    for (int i = 0; i < MAX_LOCATIONS; i++) {
        g->adjList[i] = NULL;
    }

    /* -------------------------------------------------------------
       1. LOCATIONS (12 Destinations)
       ------------------------------------------------------------- */
    Location locs[] = {
        { 0,  "Delhi",      "NCR",          "National capital with millennia of heritage and bustling markets", 28.6139, 77.2090 },
        { 1,  "Agra",       "Uttar Pradesh","Home of the iconic Taj Mahal and Mughal architectural wonders",    27.1767, 78.0081 },
        { 2,  "Jaipur",     "Rajasthan",    "The Pink City, celebrated for majestic forts and royal palaces",    26.9124, 75.7873 },
        { 3,  "Udaipur",    "Rajasthan",    "City of Lakes with romantic palatial architecture",                24.5854, 73.7125 },
        { 4,  "Jodhpur",    "Rajasthan",    "The Sun City and Blue City guarded by mighty Mehrangarh Fort",     26.2389, 73.0243 },
        { 5,  "Varanasi",   "Uttar Pradesh","Spiritual capital of India along the sacred banks of the Ganges",   25.3176, 82.9739 },
        { 6,  "Mumbai",     "Maharashtra",  "India's financial epicenter with coastal vibrancy and heritage",    19.0760, 72.8777 },
        { 7,  "Goa",        "Goa",          "Tropical paradise with sun-kissed beaches and Portuguese charm",    15.2993, 74.1240 },
        { 8,  "Manali",     "Himachal",     "High-altitude Himalayan resort nestled in Beas River Valley",      32.2432, 77.1892 },
        { 9,  "Shimla",     "Himachal",     "Former British summer capital with colonial lanes and pines",       31.1048, 77.1734 },
        { 10, "Amritsar",   "Punjab",       "Spiritual center of Sikhism and home to the sacred Golden Temple",  31.6340, 74.8723 },
        { 11, "Rishikesh",  "Uttarakhand",  "Yoga capital of the world by the pristine rapids of the Ganges",    30.0869, 78.2676 },
        { 12, "Port Blair", "Andaman",      "Remote island archipelago in the Bay of Bengal (Island/Isolated)",  11.6234, 92.7265 }
    };

    g->numLocations = sizeof(locs) / sizeof(locs[0]);
    for (int i = 0; i < g->numLocations; i++) {
        g->locations[i] = locs[i];
    }

    /* -------------------------------------------------------------
       2. ROAD NETWORK (Graph Edges with Distances, Times, Tolls)
       Notice: Port Blair (id 12) has no road edges, demonstrating
       unreachable-location handling in Dijkstra's algorithm.
       ------------------------------------------------------------- */
    /* Delhi connections */
    addRoadEdge(g, 0, 1,  233.0, 210, 8.50);   /* Delhi <-> Agra via Yamuna Expressway */
    addRoadEdge(g, 0, 2,  280.0, 270, 7.00);   /* Delhi <-> Jaipur via NH48 */
    addRoadEdge(g, 0, 9,  343.0, 420, 9.00);   /* Delhi <-> Shimla */
    addRoadEdge(g, 0, 10, 450.0, 480, 11.0);   /* Delhi <-> Amritsar via Grand Trunk Rd */
    addRoadEdge(g, 0, 11, 240.0, 300, 5.50);   /* Delhi <-> Rishikesh */

    /* Agra connections */
    addRoadEdge(g, 1, 2,  240.0, 240, 6.00);   /* Agra <-> Jaipur via Bikaner-Agra Rd */
    addRoadEdge(g, 1, 5,  610.0, 540, 15.0);   /* Agra <-> Varanasi via Agra-Lucknow Expy */

    /* Rajasthan circuit */
    addRoadEdge(g, 2, 4,  335.0, 330, 8.00);   /* Jaipur <-> Jodhpur */
    addRoadEdge(g, 2, 3,  395.0, 390, 9.50);   /* Jaipur <-> Udaipur */
    addRoadEdge(g, 4, 3,  250.0, 280, 5.00);   /* Jodhpur <-> Udaipur */

    /* Western corridor */
    addRoadEdge(g, 3, 6,  750.0, 720, 22.0);   /* Udaipur <-> Mumbai via NH48 */
    addRoadEdge(g, 6, 7,  590.0, 600, 18.0);   /* Mumbai <-> Goa via NH66 */

    /* Northern mountain links */
    addRoadEdge(g, 9, 8,  248.0, 420, 4.00);   /* Shimla <-> Manali */
    addRoadEdge(g, 10, 9, 310.0, 420, 6.50);   /* Amritsar <-> Shimla */
    addRoadEdge(g, 11, 9, 230.0, 360, 5.00);   /* Rishikesh <-> Shimla */

    /* Spiritual corridor */
    addRoadEdge(g, 5, 0,  820.0, 780, 24.0);   /* Varanasi <-> Delhi direct highway */

    /* -------------------------------------------------------------
       3. TOURIST ATTRACTIONS (36+ Attractions with details)
       ------------------------------------------------------------- */
    Attraction attrs[] = {
        /* Delhi (id 0) */
        { 1, 0, "Red Fort",               "Historical", 12.0, 120, 930, 1730, 4.6, "Mughal imperial palace citadel built by Shah Jahan" },
        { 2, 0, "Qutub Minar",            "Historical", 10.0,  90, 700, 1900, 4.7, "73m soaring victory minaret with intricate carving" },
        { 3, 0, "Humayun's Tomb",         "Historical", 10.0,  90, 600, 1800, 4.8, "Precursor to Taj Mahal with lush charbagh Persian gardens" },
        { 4, 0, "India Gate & Kartavya",  "Cultural",    0.0,  60, 600, 2200, 4.7, "National war memorial arch illuminated at night" },
        { 5, 0, "Chandni Chowk Food Walk","Food",       15.0, 120, 1000, 2100, 4.9, "Legendary street food lanes with parathas and jalebis" },

        /* Agra (id 1) */
        { 6, 1, "Taj Mahal",              "Historical", 25.0, 180, 600, 1830, 5.0, "Iconic ivory-white marble mausoleum of eternal love" },
        { 7, 1, "Agra Fort",              "Historical", 14.0, 120, 600, 1800, 4.6, "Sprawling red sandstone fortress of Mughal emperors" },
        { 8, 1, "Fatehpur Sikri",         "Historical", 10.0, 150, 700, 1800, 4.5, "Deserted 16th-century Mughal capital and royal ruins" },
        { 9, 1, "Mehtab Bagh",            "Nature",      5.0,  60, 600, 1900, 4.4, "Moonlight garden offering sunset Taj reflection views" },

        /* Jaipur (id 2) */
        { 10, 2, "Amber Palace",          "Historical", 15.0, 180, 800, 1730, 4.8, "Hilltop fortress with Sheesh Mahal mirror mosaics" },
        { 11, 2, "Hawa Mahal",            "Cultural",    8.0,  60, 900, 1700, 4.5, "High-screened Palace of Winds with 953 ornate windows" },
        { 12, 2, "City Palace Jaipur",    "Cultural",   18.0, 120, 930, 1700, 4.6, "Royal residence featuring courtyards and textile museum" },
        { 13, 2, "Jantar Mantar",         "Historical",  6.0,  75, 900, 1630, 4.6, "UNESCO astronomical observatory with world's largest sundial" },

        /* Udaipur (id 3) */
        { 14, 3, "City Palace Udaipur",   "Cultural",   15.0, 150, 900, 1730, 4.8, "Enormous lakeside palace complex overlooking Lake Pichola" },
        { 15, 3, "Lake Pichola Boat Ride","Nature",     12.0,  75, 900, 1800, 4.9, "Scenic golden-hour boat cruise around Lake Palace" },
        { 16, 3, "Saheliyon-ki-Bari",     "Nature",      4.0,  60, 900, 1900, 4.3, "Courtyard fountains and marble elephant water gardens" },

        /* Jodhpur (id 4) */
        { 17, 4, "Mehrangarh Fort",       "Historical", 15.0, 180, 900, 1700, 4.9, "Imposing cliff-edge citadel towering over the Blue City" },
        { 18, 4, "Jaswant Thada",         "Cultural",    5.0,  60, 900, 1700, 4.6, "Carved white marble cenotaph beside tranquil lake" },
        { 19, 4, "Umaid Bhawan Palace",   "Cultural",    6.0,  90, 900, 1700, 4.5, "Art Deco palace museum and royal private residence" },

        /* Varanasi (id 5) */
        { 20, 5, "Dashashwamedh Ghat",    "Religious",   0.0, 120, 500, 2100, 4.9, "Fabled riverbank hosting spectacular evening Ganga Aarti" },
        { 21, 5, "Kashi Vishwanath",      "Religious",   5.0,  90, 400, 2200, 4.8, "Golden Temple dedicated to Lord Shiva in sacred ancient lanes" },
        { 22, 5, "Sarnath Buddhist Site", "Historical",  8.0, 120, 800, 1700, 4.6, "Site where Gautama Buddha delivered his first sermon" },

        /* Mumbai (id 6) */
        { 23, 6, "Gateway of India",      "Historical",  0.0,  60, 600, 2300, 4.7, "Majestic seaside arch monument facing Mumbai Harbour" },
        { 24, 6, "Marine Drive Promenade","Nature",      0.0,  90, 600, 2359, 4.8, "The Queen's Necklace shoreline boulevard and sea breeze" },
        { 25, 6, "Elephanta Caves",       "Historical", 12.0, 210, 900, 1700, 4.6, "UNESCO rock-cut cave temples dedicated to Lord Shiva" },

        /* Goa (id 7) */
        { 26, 7, "Basilica of Bom Jesus", "Religious",   2.0,  60, 900, 1730, 4.7, "16th-century Baroque church holding St. Francis Xavier tomb" },
        { 27, 7, "Palolem Beach",         "Nature",      0.0, 180, 600, 2200, 4.8, "Crescent bay with calm waters and palm-fringed shacks" },
        { 28, 7, "Dudhsagar Waterfalls",  "Adventure",  20.0, 240, 800, 1600, 4.7, "Four-tiered massive cascade amidst Bhagwan Mahaveer reserve" },

        /* Manali (id 8) */
        { 29, 8, "Solang Valley",         "Adventure",  25.0, 210, 830, 1700, 4.8, "Adrenaline hub for paragliding, zorbing, and winter snow" },
        { 30, 8, "Hadimba Temple",        "Religious",   2.0,  60, 800, 1800, 4.5, "Wooden pagoda shrine inside towering cedar deodar forest" },
        { 31, 8, "Rohtang Pass Tour",     "Adventure",  30.0, 300, 700, 1600, 4.9, "High mountain pass at 13,058 ft with breathtaking vistas" },

        /* Shimla (id 9) */
        { 32, 9, "The Mall & Ridge",      "Cultural",    0.0, 120, 800, 2200, 4.6, "Pedestrian promenade overlooking snow-capped Himalayan peaks" },
        { 33, 9, "Jakhoo Temple & Ropeway","Religious",  8.0,  90, 800, 1800, 4.5, "Hilltop Hanuman shrine with panoramic cable car ride" },

        /* Amritsar (id 10) */
        { 34, 10, "Golden Temple (Harmandir Sahib)", "Religious", 0.0, 180, 400, 2300, 5.0, "Most sacred Sikh gurdwara lined in pure gold and holy pond" },
        { 35, 10, "Wagah Border Ceremony","Cultural",   0.0, 120, 1500, 1830, 4.9, "Electrifying daily military lowering-of-flags ritual" },

        /* Rishikesh (id 11) */
        { 36, 11, "Ganga White Water Rafting", "Adventure", 22.0, 180, 800, 1600, 4.9, "Thrilling Class III/IV river rapids along the Himalayan Ganges" },
        { 37, 11, "Triveni Ghat Evening Aarti", "Religious", 0.0,  75, 1700, 2000, 4.8, "Mesmerizing floating oil lamps and Vedic chants" }
    };

    data->numAttractions = sizeof(attrs) / sizeof(attrs[0]);
    for (int i = 0; i < data->numAttractions; i++) {
        data->attractions[i] = attrs[i];
    }

    /* -------------------------------------------------------------
       4. HOTELS (30+ across Budget, Standard, Luxury tiers)
       ------------------------------------------------------------- */
    Hotel hts[] = {
        /* Delhi */
        { 1, 0, "Zostel Delhi",           "Budget",    24.0, 4.4, 2.5, "Free Wi-Fi, Rooftop Cafe, AC, Lockers" },
        { 2, 0, "Bloomrooms @ New Delhi", "Standard",  65.0, 4.5, 1.8, "Complimentary Breakfast, Work Desk, Cloud Beds" },
        { 3, 0, "The Imperial New Delhi", "Luxury",   280.0, 4.9, 0.8, "Heritage Spa, Outdoor Pool, Fine Dining, Butler Service" },

        /* Agra */
        { 4, 1, "Moustache Agra",         "Budget",    20.0, 4.3, 1.2, "Taj View Terrace, Cafe, Social Lounge" },
        { 5, 1, "Tajview IHCL SeleQtions","Standard",  85.0, 4.6, 2.0, "Pool, Taj View Rooms, Multi-cuisine Restaurant" },
        { 6, 1, "The Oberoi Amarvilas",   "Luxury",   420.0, 5.0, 0.6, "Direct Taj Mahal views from every room, Royal Spa, Fine Dining" },

        /* Jaipur */
        { 7, 2, "Backpacker Panda Jaipur","Budget",    18.0, 4.3, 3.0, "Wi-Fi, Shared Kitchen, Air Conditioning" },
        { 8, 2, "Alsisar Haveli",         "Standard",  75.0, 4.6, 1.5, "Heritage Architecture, Courtyard Pool, Folk Shows" },
        { 9, 2, "Rambagh Palace",         "Luxury",   390.0, 4.9, 2.2, "Former Maharaja Palace, Peacocks in Gardens, Luxury Spa" },

        /* Udaipur */
        { 10, 3, "Bunkyard Hostel",       "Budget",    22.0, 4.5, 1.0, "Lakefront Terrace, Music Nights, Cafe" },
        { 11, 3, "Amet Haveli",           "Standard",  95.0, 4.7, 0.8, "Lakeside Dining Ambrai, Rajput Architecture, Pool" },
        { 12, 3, "Taj Lake Palace",       "Luxury",   450.0, 5.0, 0.5, "18th-century floating marble palace on Lake Pichola" },

        /* Jodhpur */
        { 13, 4, "Hostel LaVie Jodhpur",  "Budget",    19.0, 4.4, 1.5, "Fort View Rooftop, Yoga, Wi-Fi" },
        { 14, 4, "Pal Haveli",            "Standard",  70.0, 4.6, 1.1, "Heritage Rooms, Rooftop Restaurant overlooking Fort" },
        { 15, 4, "Umaid Bhawan Palace Hotel", "Luxury", 440.0, 4.9, 3.5, "Royal Art Deco Residence, Champagne Breakfast, Palace Museum" },

        /* Varanasi */
        { 16, 5, "Stops Hostel Varanasi", "Budget",    18.0, 4.3, 2.0, "Courtyard, Walking Tours, Hindi Lessons" },
        { 17, 5, "BrijRama Palace",       "Standard", 140.0, 4.8, 0.2, "Historic Ghat Palace, Classical Sitar Evenings, River Views" },
        { 18, 5, "Taj Ganges Varanasi",   "Luxury",   260.0, 4.8, 3.8, "12 Acres of Green Gardens, Luxury Wellness, World-class Dining" },

        /* Mumbai */
        { 19, 6, "Cohostel Bandra",       "Budget",    28.0, 4.3, 8.0, "Trendy Suburb Location, Wi-Fi, Social Hub" },
        { 20, 6, "Residency Hotel Fort",  "Standard",  90.0, 4.5, 1.2, "Colonial Quarter Walkability, Gourmet Breakfast" },
        { 21, 6, "The Taj Mahal Palace",  "Luxury",   380.0, 4.9, 0.1, "Legendary Sea-facing Landmark, Signature Butler Service, Historic Lobby" },

        /* Goa */
        { 22, 7, "Wanderers Hostel Goa",  "Budget",    22.0, 4.4, 0.5, "Swimming Pool, Beach Proximity, Garden Hammocks" },
        { 23, 7, "The Figueiredo House",  "Standard",  80.0, 4.6, 4.0, "Grand Portuguese Estate, Curated Dining, Quiet Countryside" },
        { 24, 7, "Taj Exotica Resort & Spa", "Luxury", 320.0, 4.8, 0.3, "Mediterranean-inspired 56-acre Sanctuary, Private Beach Access" },

        /* Manali */
        { 25, 8, "Alt Life Old Manali",   "Budget",    20.0, 4.4, 2.0, "Riverside Vibe, Live Acoustic Music, High-speed Wi-Fi" },
        { 26, 8, "The Himalayan Resort",  "Standard",  95.0, 4.7, 1.5, "Victorian Castle Architecture, Apple Orchards, Heated Pool" },
        { 27, 8, "Span Resort & Spa",     "Luxury",   280.0, 4.8, 5.0, "Helipad, River Beas Frontage, Luxury Cottage Suites" },

        /* Shimla */
        { 28, 9, "The Hosteller Shimla",  "Budget",    22.0, 4.3, 1.0, "Mountain Balconies, Game Room, Cafe" },
        { 29, 9, "Clarkes Hotel Shimla",  "Standard",  90.0, 4.6, 0.5, "Heritage Colonial Dining, Direct Mall Road Access" },
        { 30, 9, "Wildflower Hall Shimla","Luxury",   360.0, 4.9, 8.0, "Lord Kitchener's Former Residence, Cedar Forest, Infinity Whirlpool" },

        /* Amritsar */
        { 31, 10, "City Backpackers Amritsar", "Budget", 16.0, 4.2, 0.8, "5-min Walk to Golden Temple, Clean Dorms" },
        { 32, 10, "Radisson Blu Amritsar",     "Standard", 75.0, 4.5, 4.0, "Modern Comfort, Airport Shuttle, Pool" },
        { 33, 10, "Taj Swarna Amritsar",       "Luxury",  210.0, 4.8, 3.2, "Luxury Spa, Fine Dining Punjabi Cuisine, Infinity Pool" },

        /* Rishikesh */
        { 34, 11, "Bunk Stay Rishikesh",       "Budget",   18.0, 4.5, 0.6, "Rooftop Yoga, Ganga River Panorama, Organic Cafe" },
        { 35, 11, "Aloha On The Ganges",       "Standard", 95.0, 4.6, 1.5, "Direct River Access, Infinity Pool, Ayurvedic Spa" },
        { 36, 11, "Ananda in the Himalayas",   "Luxury",  520.0, 5.0, 9.0, "World-renowned Maharaja Wellness Palace, Vedanta, Hydrotherapy" }
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

