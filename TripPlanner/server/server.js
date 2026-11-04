/**
 * server.js - Node.js Express API Server for Trip Planner.
 * Communicates with the C DSA executable via child processes and JSON streams.
 * Configured for single-city planning, Exact TSP optimization, and Indian Rupee (INR ₹) budgets.
 */

const express = require('express');
const cors = require('cors');
const path = require('path');
const { execFile } = require('child_process');
const fs = require('fs');

const app = express();
const PORT = process.env.PORT || 3000;

app.use(cors());
app.use(express.json());

// Serve static frontend files
const frontendDir = path.join(__dirname, '../frontend');
app.use(express.static(frontendDir));

// Locate the C executable
function getCExecutablePath() {
    const candidates = [
        path.join(__dirname, '../backend/trip_planner.exe'),
        path.join(__dirname, '../backend/trip_planner'),
        path.join(process.cwd(), 'TripPlanner/backend/trip_planner.exe'),
        path.join(process.cwd(), 'TripPlanner/backend/trip_planner'),
        path.join(process.cwd(), 'backend/trip_planner.exe'),
        path.join(process.cwd(), 'backend/trip_planner')
    ];
    for (const p of candidates) {
        if (fs.existsSync(p)) return p;
    }
    return candidates[0];
}

/**
 * Executes the C binary with specified arguments and returns parsed JSON.
 */
function runCBackend(args) {
    return new Promise((resolve, reject) => {
        const binPath = getCExecutablePath();
        const rootCwd = path.join(__dirname, '..');

        execFile(binPath, ['--api', ...args], { cwd: rootCwd, maxBuffer: 10 * 1024 * 1024 }, (err, stdout, stderr) => {
            if (err) {
                console.error(`C execution error [${binPath}]:`, err, stderr);
                return reject(new Error(stderr || err.message));
            }
            try {
                const trimmed = stdout.trim();
                const json = JSON.parse(trimmed);
                resolve(json);
            } catch (parseErr) {
                console.error('Failed to parse C JSON stdout:\n', stdout);
                reject(new Error(`Invalid JSON received from backend: ${parseErr.message}`));
            }
        });
    });
}

/* -------------------------------------------------------------
   REST API ENDPOINTS
   ------------------------------------------------------------- */

// Health Check
app.get('/api/health', (req, res) => {
    res.json({
        status: 'online',
        system: 'Trip Planner Web Engine',
        timestamp: new Date().toISOString()
    });
});

// 1. Get All Locations
app.get('/api/locations', async (req, res) => {
    try {
        const data = await runCBackend(['get_locations']);
        res.json(data);
    } catch (err) {
        res.status(500).json({ success: false, error: err.message });
    }
});

// 2. Get All Attractions
app.get('/api/attractions', async (req, res) => {
    try {
        const data = await runCBackend(['get_attractions']);
        res.json(data);
    } catch (err) {
        res.status(500).json({ success: false, error: err.message });
    }
});

// 3. Get All Hotels
app.get('/api/hotels', async (req, res) => {
    try {
        const data = await runCBackend(['get_hotels']);
        res.json(data);
    } catch (err) {
        res.status(500).json({ success: false, error: err.message });
    }
});

// 4. Shortest Path Route
app.post('/api/route', async (req, res) => {
    try {
        const { srcId, destId } = req.body;
        if (srcId === undefined || destId === undefined) {
            return res.status(400).json({ success: false, error: 'srcId and destId are required.' });
        }
        const data = await runCBackend(['plan_route', String(srcId), String(destId)]);
        res.json(data);
    } catch (err) {
        res.status(500).json({ success: false, error: err.message });
    }
});

// 5. Itinerary Optimization (Exact TSP + Day Schedule)
app.post('/api/itinerary', async (req, res) => {
    try {
        const { numDays = 3, attrIds = [] } = req.body;
        const csv = attrIds.length > 0 ? attrIds.join(',') : '_NONE_';
        const data = await runCBackend(['optimize_itinerary', String(numDays), csv]);
        res.json(data);
    } catch (err) {
        res.status(500).json({ success: false, error: err.message });
    }
});

// 6. Budget Calculation
app.post('/api/budget', async (req, res) => {
    try {
        const {
            userBudget = 25000,
            numDays = 3,
            numTravellers = 2,
            travelStyle = 'Standard',
            destLocationId = 13,
            selectedHotelId = -1,
            routeDistKm = 35,
            activityCost = 500
        } = req.body;

        const data = await runCBackend([
            'calculate_budget',
            String(userBudget),
            String(numDays),
            String(numTravellers),
            String(travelStyle),
            String(destLocationId),
            String(selectedHotelId),
            String(routeDistKm),
            String(activityCost)
        ]);
        res.json(data);
    } catch (err) {
        res.status(500).json({ success: false, error: err.message });
    }
});

// 7. Attraction Search
app.get('/api/search', async (req, res) => {
    try {
        const query = (req.query.query || '').trim() || '_EMPTY_';
        const category = (req.query.category || '').trim() || '_EMPTY_';
        const data = await runCBackend(['search_attractions', query, category]);
        res.json(data);
    } catch (err) {
        res.status(500).json({ success: false, error: err.message });
    }
});

// 8. Full Combined Trip Plan (Single City Focus, Exact TSP, INR)
app.post('/api/plan-trip', async (req, res) => {
    try {
        const {
            destId = 13,
            numDays = 3,
            numTravellers = 2,
            userBudget = 25000,
            travelStyle = 'Standard',
            selectedHotelId = -1,
            attrIds = []
        } = req.body;

        const attrCsv = attrIds && attrIds.length > 0 ? attrIds.join(',') : '_NONE_';

        const data = await runCBackend([
            'full_trip_plan',
            String(destId),
            String(numDays),
            String(numTravellers),
            String(userBudget),
            String(travelStyle),
            String(selectedHotelId),
            attrCsv
        ]);
        res.json(data);
    } catch (err) {
        res.status(500).json({ success: false, error: err.message });
    }
});

// 9. Saved Trips CRUD
app.get('/api/trips', async (req, res) => {
    try {
        const data = await runCBackend(['trip_list']);
        res.json(data);
    } catch (err) {
        res.status(500).json({ success: false, error: err.message });
    }
});

app.post('/api/trips', async (req, res) => {
    try {
        const {
            tripName = 'City Expedition',
            startDate = '2026-11-25',
            destLocation = 'Pune',
            numDays = 3,
            numTravellers = 2,
            totalBudget = 25000,
            estimatedCost = 18500,
            totalDistanceKm = 40,
            travelStyle = 'Standard',
            notes = 'Created from Trip Planner Web App'
        } = req.body;

        const data = await runCBackend([
            'trip_add',
            tripName,
            startDate,
            destLocation,
            String(numDays),
            String(numTravellers),
            String(totalBudget),
            String(estimatedCost),
            String(totalDistanceKm),
            travelStyle,
            notes
        ]);
        res.json(data);
    } catch (err) {
        res.status(500).json({ success: false, error: err.message });
    }
});

app.get('/api/trips/:id', async (req, res) => {
    try {
        const id = req.params.id;
        const data = await runCBackend(['trip_get', String(id)]);
        res.json(data);
    } catch (err) {
        res.status(500).json({ success: false, error: err.message });
    }
});

app.put('/api/trips/:id', async (req, res) => {
    try {
        const id = req.params.id;
        const {
            tripName,
            startDate,
            destLocation,
            numDays,
            numTravellers,
            totalBudget,
            estimatedCost,
            totalDistanceKm,
            travelStyle,
            notes
        } = req.body;

        const data = await runCBackend([
            'trip_update',
            String(id),
            tripName || '',
            startDate || '',
            destLocation || 'Pune',
            String(numDays || 0),
            String(numTravellers || 0),
            String(totalBudget || 0),
            String(estimatedCost || 0),
            String(totalDistanceKm || 0),
            travelStyle || '',
            notes || ''
        ]);
        res.json(data);
    } catch (err) {
        res.status(500).json({ success: false, error: err.message });
    }
});

app.delete('/api/trips/:id', async (req, res) => {
    try {
        const id = req.params.id;
        const data = await runCBackend(['trip_delete', String(id)]);
        res.json(data);
    } catch (err) {
        res.status(500).json({ success: false, error: err.message });
    }
});

// Automated tests endpoint
app.get('/api/tests', (req, res) => {
    const binPath = getCExecutablePath();
    const rootCwd = path.join(__dirname, '..');
    execFile(binPath, ['--api', 'run_tests'], { cwd: rootCwd }, (err, stdout, stderr) => {
        if (err) {
            return res.status(500).json({ success: false, error: stderr || err.message });
        }
        res.type('text/plain').send(stdout);
    });
});

// Catch-all route to serve index.html
app.get('*', (req, res) => {
    res.sendFile(path.join(frontendDir, 'index.html'));
});

// Start Server
app.listen(PORT, '0.0.0.0', () => {
    console.log(`=======================================================`);
    console.log(` Trip Planner Web Application`);
    console.log(` URL: http://localhost:${PORT}`);
    console.log(`=======================================================`);
});
