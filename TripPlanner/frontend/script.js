/**
 * script.js - Frontend application controller for TripPlanner.
 * Includes Dark/Light theme switching, single-city planning,
 * Indian Rupee (INR ₹) formatting, Exact TSP itinerary rendering,
 * Leaflet.js interactive attractions map, and detailed saved trips view.
 */

// Application State
const state = {
  locations: [],
  attractions: [],
  hotels: [],
  savedTrips: [],
  currentPlan: null,
  selectedAttrIds: new Set(),
  activeSearchCategory: 'All',
  activeSearchCity: 'All',
  searchDebounceTimer: null,
  map: null,
  mapMarkers: []
};

// Helper: Format amount in Indian Rupees (INR)
function formatINR(amount) {
  if (amount === undefined || amount === null || isNaN(amount)) return '₹0';
  return '₹' + Math.round(amount).toLocaleString('en-IN');
}

// Helper: format HHMM to AM/PM time
function formatHHMM(hhmm) {
  const hours = Math.floor(hhmm / 100);
  const mins = hhmm % 100;
  const period = hours >= 12 ? 'PM' : 'AM';
  const displayHours = hours % 12 === 0 ? 12 : hours % 12;
  return `${String(displayHours).padStart(2, '0')}:${String(mins).padStart(2, '0')} ${period}`;
}

// DOM Elements
const elements = {
  themeToggleBtn: document.getElementById('theme-toggle-btn'),
  themeLabelText: document.getElementById('theme-label-text'),
  selectDest: document.getElementById('select-dest'),
  inputDays: document.getElementById('input-days'),
  inputTravellers: document.getElementById('input-travellers'),
  inputBudget: document.getElementById('input-budget'),
  attractionChipsContainer: document.getElementById('attraction-chips-container'),
  btnSelectAllAttrs: document.getElementById('btn-select-all-attrs'),
  btnClearAttrs: document.getElementById('btn-clear-attrs'),
  tripForm: document.getElementById('trip-form'),
  btnGeneratePlan: document.getElementById('btn-generate-plan'),
  planLoader: document.getElementById('plan-loader'),
  resultsSection: document.getElementById('results-section'),
  btnSaveTrip: document.getElementById('btn-save-trip'),
  btnPrintPlan: document.getElementById('btn-print-plan'),

  // Route metrics
  metricDist: document.getElementById('metric-dist'),
  metricTime: document.getElementById('metric-time'),
  metricStops: document.getElementById('metric-stops'),
  metricDays: document.getElementById('metric-days'),
  routeStatusAlert: document.getElementById('route-status-alert'),
  routeStatusText: document.getElementById('route-status-text'),
  routeReachableBadge: document.getElementById('route-reachable-badge'),
  hopsContainer: document.getElementById('hops-container'),

  // Day schedules
  daySchedulesList: document.getElementById('day-schedules-list'),
  totalScheduledBadge: document.getElementById('total-scheduled-badge'),

  // Budget
  budgetSpentLabel: document.getElementById('budget-spent-label'),
  budgetTotalLabel: document.getElementById('budget-total-label'),
  budgetProgressFill: document.getElementById('budget-progress-fill'),
  budgetRemainingVal: document.getElementById('budget-remaining-val'),
  budgetStatusPill: document.getElementById('budget-status-pill'),
  budgetWarningBox: document.getElementById('budget-warning-box'),
  budgetWarningIcon: document.getElementById('budget-warning-icon'),
  budgetWarningText: document.getElementById('budget-warning-text'),
  expHotelVal: document.getElementById('exp-hotel-val'),
  expHotelDetail: document.getElementById('exp-hotel-detail'),
  expFoodVal: document.getElementById('exp-food-val'),
  expTransportVal: document.getElementById('exp-transport-val'),
  expActivityVal: document.getElementById('exp-activity-val'),
  expBufferVal: document.getElementById('exp-buffer-val'),
  expTotalVal: document.getElementById('exp-total-val'),

  // Hotels
  savingsCallout: document.getElementById('savings-callout'),
  savingsCalloutText: document.getElementById('savings-callout-text'),
  topHotelsList: document.getElementById('top-hotels-list'),
  cheaperHotelsList: document.getElementById('cheaper-hotels-list'),
  hotelSortSelect: document.getElementById('hotel-sort-select'),

  // Search
  selectSearchCity: document.getElementById('select-search-city'),
  inputSearchQuery: document.getElementById('input-search-query'),
  btnSearchTrigger: document.getElementById('btn-search-trigger'),
  categoryPills: document.getElementById('category-pills'),
  searchCountLabel: document.getElementById('search-count-label'),
  searchResultsContainer: document.getElementById('search-results-container'),
  attractionsMap: document.getElementById('attractions-map'),
  mapCityLabel: document.getElementById('map-city-label'),

  // Saved Trips
  savedTripsTbody: document.getElementById('saved-trips-tbody'),
  savedCountNav: document.getElementById('saved-count-nav'),
  inputTripFilter: document.getElementById('input-trip-filter'),
  btnRefreshTrips: document.getElementById('btn-refresh-trips'),
  toastContainer: document.getElementById('toast-container'),

  // Modal
  savedTripModal: document.getElementById('saved-trip-modal'),
  modalTripName: document.getElementById('modal-trip-name'),
  modalTripMeta: document.getElementById('modal-trip-meta'),
  modalTripBody: document.getElementById('modal-trip-body'),
  btnCloseTripModal: document.getElementById('btn-close-trip-modal'),
  btnModalBack: document.getElementById('btn-modal-back')
};

/* -------------------------------------------------------------------------
   DARK / LIGHT THEME TOGGLE WITH LOCALSTORAGE PERSISTENCE
   ------------------------------------------------------------------------- */
function initTheme() {
  const savedTheme = localStorage.getItem('tripPlannerTheme') || 'light';
  applyTheme(savedTheme);

  elements.themeToggleBtn.addEventListener('click', () => {
    const currentTheme = document.documentElement.getAttribute('data-theme') || 'light';
    const nextTheme = currentTheme === 'dark' ? 'light' : 'dark';
    applyTheme(nextTheme);
  });
}

function applyTheme(theme) {
  document.documentElement.setAttribute('data-theme', theme);
  localStorage.setItem('tripPlannerTheme', theme);
  if (theme === 'dark') {
    elements.themeLabelText.textContent = 'Light Mode';
  } else {
    elements.themeLabelText.textContent = 'Dark Mode';
  }
}

/* -------------------------------------------------------------------------
   NOTIFICATION TOASTS
   ------------------------------------------------------------------------- */
function showToast(message, type = 'info') {
  const toast = document.createElement('div');
  toast.className = `toast ${type}`;
  toast.innerHTML = `<span>${type === 'success' ? '✓' : type === 'error' ? '✕' : 'ℹ'}</span> <span>${message}</span>`;
  elements.toastContainer.appendChild(toast);
  setTimeout(() => {
    toast.style.opacity = '0';
    setTimeout(() => toast.remove(), 300);
  }, 4000);
}

/* -------------------------------------------------------------------------
   INITIAL DATA FETCHING
   ------------------------------------------------------------------------- */
async function initAppData() {
  initTheme();
  initLeafletMap();

  try {
    const [locRes, attrRes, hotelRes, tripsRes] = await Promise.all([
      fetch('/api/locations').then(r => r.json()),
      fetch('/api/attractions').then(r => r.json()),
      fetch('/api/hotels').then(r => r.json()),
      fetch('/api/trips').then(r => r.json())
    ]);

    if (locRes.success) {
      state.locations = locRes.locations || [];
      const statLoc = document.getElementById('stat-locations');
      if (statLoc) statLoc.textContent = state.locations.length;
    }

    if (attrRes.success) {
      state.attractions = attrRes.attractions || [];
      const statAttr = document.getElementById('stat-attractions');
      if (statAttr) statAttr.textContent = state.attractions.length + '+';
    }

    if (hotelRes.success) {
      state.hotels = hotelRes.hotels || [];
      const statHotel = document.getElementById('stat-hotels');
      if (statHotel) statHotel.textContent = state.hotels.length + '+';
    }

    if (tripsRes.success) {
      state.savedTrips = tripsRes.trips || [];
      renderSavedTripsTable(state.savedTrips);
    }

    // Populate dropdowns after data is loaded
    if (state.locations.length > 0) {
      populateLocationDropdown();
      populateSearchCityDropdown();
    }

    // Initial search
    executeSearch();

  } catch (err) {
    console.error('Initialization error:', err);
    showToast('Failed to connect to backend server. Please verify server is running.', 'error');
  }
}

function populateLocationDropdown() {
  elements.selectDest.innerHTML = '';

  // Sort locations so Pune and major cities are at the top
  const sortedLocations = [...state.locations].sort((a, b) => {
    if (a.name === 'Pune') return -1;
    if (b.name === 'Pune') return 1;
    return a.name.localeCompare(b.name);
  });

  sortedLocations.forEach(loc => {
    const optDest = document.createElement('option');
    optDest.value = loc.id;
    optDest.textContent = `${loc.name} (${loc.state})`;
    elements.selectDest.appendChild(optDest);
  });

  // Default to Pune (id: 13) or first location
  const puneLoc = state.locations.find(l => l.name.toLowerCase() === 'pune');
  if (puneLoc) {
    elements.selectDest.value = String(puneLoc.id);
    updateAttractionChipsForDestination(puneLoc.id);
  } else if (sortedLocations.length > 0) {
    elements.selectDest.value = String(sortedLocations[0].id);
    updateAttractionChipsForDestination(sortedLocations[0].id);
  }
}

function populateSearchCityDropdown() {
  if (!elements.selectSearchCity) return;
  elements.selectSearchCity.innerHTML = '<option value="All">All Cities</option>';

  const sortedLocations = [...state.locations].sort((a, b) => {
    if (a.name === 'Pune') return -1;
    if (b.name === 'Pune') return 1;
    return a.name.localeCompare(b.name);
  });

  sortedLocations.forEach(loc => {
    const opt = document.createElement('option');
    opt.value = loc.name;
    opt.textContent = `${loc.name}`;
    elements.selectSearchCity.appendChild(opt);
  });

  elements.selectSearchCity.addEventListener('change', (e) => {
    state.activeSearchCity = e.target.value;
    if (elements.mapCityLabel) {
      elements.mapCityLabel.textContent = state.activeSearchCity === 'All' ? 'All Cities' : state.activeSearchCity;
    }
    executeSearch();
  });
}

/* -------------------------------------------------------------------------
   LEAFLET MAP INTEGRATION
   ------------------------------------------------------------------------- */
function initLeafletMap() {
  if (typeof L === 'undefined') {
    console.warn('Leaflet library not loaded.');
    return;
  }
  if (state.map) return;

  const mapContainer = document.getElementById('attractions-map');
  if (!mapContainer) return;

  // Center on India (approx 20.5937, 78.9629)
  state.map = L.map('attractions-map', {
    scrollWheelZoom: false
  }).setView([18.5204, 73.8567], 11); // Default centered on Pune

  L.tileLayer('https://tile.openstreetmap.org/{z}/{x}/{y}.png', {
    maxZoom: 18,
    attribution: '&copy; <a href="https://www.openstreetmap.org/copyright" target="_blank" rel="noopener">OpenStreetMap</a>'
  }).addTo(state.map);
}

function updateMapMarkers(attractions) {
  if (!state.map || typeof L === 'undefined') return;

  // Clear existing markers
  state.mapMarkers.forEach(m => m.remove());
  state.mapMarkers = [];

  const validAttractions = (attractions || []).filter(a => a.latitude && a.longitude && (a.latitude !== 0 || a.longitude !== 0));

  if (validAttractions.length === 0) return;

  const latLngs = [];

  validAttractions.forEach(a => {
    const marker = L.marker([a.latitude, a.longitude]);
    const feeText = a.ticketCost > 0 ? formatINR(a.ticketCost) : 'Free Entry';
    const popupContent = `
      <div class="map-popup">
        <div class="map-popup-title">${a.name}</div>
        <div class="map-popup-meta">
          <span class="map-popup-badge">${a.category}</span>
          <span>📍 ${a.locationName || ''}</span>
        </div>
        <div style="font-size:0.8rem; margin-bottom: 5px; color: var(--text-muted);">${(a.description || '').substring(0, 100)}...</div>
        <div class="map-popup-price">Ticket: ${feeText} &bull; Rating: ⭐ ${a.rating.toFixed(1)}</div>
      </div>
    `;
    marker.bindPopup(popupContent);
    marker.addTo(state.map);
    marker.attractionId = a.id;
    state.mapMarkers.push(marker);
    latLngs.push([a.latitude, a.longitude]);
  });

  if (latLngs.length > 0) {
    if (latLngs.length === 1) {
      state.map.setView(latLngs[0], 13);
    } else {
      const bounds = L.latLngBounds(latLngs);
      state.map.fitBounds(bounds, { padding: [35, 35], maxZoom: 14 });
    }
  }

  setTimeout(() => {
    if (state.map) state.map.invalidateSize();
  }, 250);
}

/* -------------------------------------------------------------------------
   ATTRACTION SELECTION CHIPS
   ------------------------------------------------------------------------- */
function updateAttractionChipsForDestination(destId) {
  const destAttrs = state.attractions.filter(a => a.locationId === parseInt(destId, 10));
  elements.attractionChipsContainer.innerHTML = '';
  state.selectedAttrIds.clear();

  if (destAttrs.length === 0) {
    elements.attractionChipsContainer.innerHTML = '<div class="placeholder-text">No attractions registered for this destination yet.</div>';
    return;
  }

  destAttrs.forEach(a => {
    state.selectedAttrIds.add(a.id); // default select all for destination

    const chip = document.createElement('label');
    chip.className = 'chip-label active';
    chip.dataset.id = a.id;
    chip.innerHTML = `
      <input type="checkbox" checked value="${a.id}">
      <span>${a.name}</span>
      <span class="chip-cat">(${a.category})</span>
    `;

    chip.addEventListener('click', (e) => {
      e.preventDefault();
      const id = a.id;
      const cb = chip.querySelector('input');
      if (state.selectedAttrIds.has(id)) {
        state.selectedAttrIds.delete(id);
        chip.classList.remove('active');
        if (cb) cb.checked = false;
      } else {
        state.selectedAttrIds.add(id);
        chip.classList.add('active');
        if (cb) cb.checked = true;
      }
    });

    elements.attractionChipsContainer.appendChild(chip);
  });
}

elements.selectDest.addEventListener('change', (e) => {
  updateAttractionChipsForDestination(e.target.value);
});

elements.btnSelectAllAttrs.addEventListener('click', () => {
  const chips = elements.attractionChipsContainer.querySelectorAll('.chip-label');
  chips.forEach(chip => {
    const id = parseInt(chip.dataset.id, 10);
    state.selectedAttrIds.add(id);
    chip.classList.add('active');
    const cb = chip.querySelector('input');
    if (cb) cb.checked = true;
  });
});

elements.btnClearAttrs.addEventListener('click', () => {
  state.selectedAttrIds.clear();
  const chips = elements.attractionChipsContainer.querySelectorAll('.chip-label');
  chips.forEach(chip => {
    chip.classList.remove('active');
    const cb = chip.querySelector('input');
    if (cb) cb.checked = false;
  });
});

/* -------------------------------------------------------------------------
   TRIP PLAN GENERATION (Exact TSP, INR Budget, Route Hops)
   ------------------------------------------------------------------------- */
elements.tripForm.addEventListener('submit', async (e) => {
  e.preventDefault();

  const destId = parseInt(elements.selectDest.value, 10);
  const numDays = parseInt(elements.inputDays.value, 10);
  const numTravellers = parseInt(elements.inputTravellers.value, 10);
  const userBudget = parseFloat(elements.inputBudget.value);
  const travelStyle = document.querySelector('input[name="travelStyle"]:checked').value;
  const attrIds = Array.from(state.selectedAttrIds);

  if (isNaN(destId)) {
    showToast('Please select a destination city.', 'error');
    return;
  }

  // Loader state
  elements.btnGeneratePlan.disabled = true;
  elements.planLoader.style.display = 'block';

  try {
    const response = await fetch('/api/plan-trip', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        destId,
        numDays,
        numTravellers,
        userBudget,
        travelStyle,
        attrIds
      })
    });

    const data = await response.json();

    if (data.success && data.plan) {
      state.currentPlan = data.plan;
      state.currentPlan.formParams = { destId, numDays, numTravellers, userBudget, travelStyle };

      renderTripResults(data.plan);
      elements.resultsSection.classList.remove('hidden');
      elements.resultsSection.scrollIntoView({ behavior: 'smooth' });
      showToast('Optimal trip plan generated successfully!', 'success');
    } else {
      showToast(data.error || 'Failed to generate plan. Please try again.', 'error');
    }

  } catch (err) {
    console.error('Trip plan generation error:', err);
    showToast('Network error while planning trip.', 'error');
  } finally {
    elements.btnGeneratePlan.disabled = false;
    elements.planLoader.style.display = 'none';
  }
});

/* -------------------------------------------------------------------------
   RENDER TRIP BLUEPRINT
   ------------------------------------------------------------------------- */
function renderTripResults(plan) {
  const { destinationCity, route, itinerary, budget } = plan;

  /* 1. Route Section */
  elements.metricDist.textContent = `${route.totalDistanceKm.toFixed(1)} km`;
  const hrs = Math.floor(route.totalTimeMin / 60);
  const mins = route.totalTimeMin % 60;
  elements.metricTime.textContent = `${hrs}h ${mins}m`;
  elements.metricStops.textContent = itinerary.totalAttractionsScheduled;
  elements.metricDays.textContent = `${itinerary.numDays} Days`;

  elements.routeStatusText.textContent = route.statusMessage;
  elements.routeReachableBadge.textContent = route.reachable ? 'Route Verified' : 'Unreachable';

  // Render hops timeline
  elements.hopsContainer.innerHTML = '';
  if (route.hops && route.hops.length > 0) {
    route.hops.forEach((h, idx) => {
      const hop = document.createElement('div');
      hop.className = 'hop-item';
      hop.innerHTML = `
        <div class="hop-badge">Leg ${idx + 1}</div>
        <div class="hop-content">
          <div class="hop-names">
            <span class="from-name">${h.fromName}</span>
            <span class="arrow">➔</span>
            <span class="to-name">${h.toName}</span>
          </div>
          <div class="hop-meta">
            <span>Day ${h.day}</span> &bull;
            <span>${h.distanceKm.toFixed(1)} km</span> &bull;
            <span>${h.travelTimeMin} mins transit</span>
          </div>
        </div>
      `;
      elements.hopsContainer.appendChild(hop);
    });
  } else {
    elements.hopsContainer.innerHTML = `<div class="placeholder-text">All scheduled sights are conveniently located within central ${destinationCity}.</div>`;
  }

  /* 2. Day-by-Day Itinerary Schedule */
  elements.totalScheduledBadge.textContent = `${itinerary.totalAttractionsScheduled} Sights Scheduled`;
  elements.daySchedulesList.innerHTML = '';

  if (itinerary.days && itinerary.days.length > 0) {
    itinerary.days.forEach(day => {
      const dayCard = document.createElement('div');
      dayCard.className = 'day-card';

      let itemsHtml = '';
      if (day.items && day.items.length > 0) {
        day.items.forEach(it => {
          itemsHtml += `
            <div class="schedule-item">
              <div class="item-time">
                <span class="time-range">${formatHHMM(it.visitStartTime)} - ${formatHHMM(it.visitEndTime)}</span>
                <span class="item-duration">${it.visitDurationMin} mins</span>
              </div>
              <div class="item-details">
                <div class="item-header">
                  <span class="item-name">${it.name}</span>
                  <span class="badge info">${it.category}</span>
                </div>
                <div class="item-meta">
                  <span>Entry: ${it.ticketCost > 0 ? formatINR(it.ticketCost) : 'Free'}</span>
                  ${it.timingNote ? `<span class="timing-note">• ${it.timingNote}</span>` : ''}
                </div>
              </div>
            </div>
          `;
        });
      } else {
        itemsHtml = '<div class="placeholder-text">Leisure day for relaxation, shopping, and local culinary exploration.</div>';
      }

      dayCard.innerHTML = `
        <div class="day-header">
          <div class="day-title">Day ${day.dayNumber}</div>
          <div class="day-stats">
            <span>${day.itemCount} Stops</span> &bull;
            <span>${formatHHMM(day.dayStartTime)} to ${formatHHMM(day.dayEndTime)}</span> &bull;
            <span>Tickets: ${formatINR(day.totalTicketCost)}</span>
          </div>
        </div>
        <div class="day-items-list">${itemsHtml}</div>
      `;

      elements.daySchedulesList.appendChild(dayCard);
    });
  }

  /* 3. Budget Section (INR ₹) */
  elements.budgetSpentLabel.textContent = formatINR(budget.totalExpense);
  elements.budgetTotalLabel.textContent = formatINR(budget.userBudget);

  const spentRatio = Math.min(100, Math.max(0, (budget.totalExpense / budget.userBudget) * 100));
  elements.budgetProgressFill.style.width = `${spentRatio}%`;

  if (budget.remainingBudget >= 0) {
    elements.budgetRemainingVal.textContent = formatINR(budget.remainingBudget);
    elements.budgetRemainingVal.style.color = '#10b981';
  } else {
    elements.budgetRemainingVal.textContent = `-${formatINR(Math.abs(budget.remainingBudget))}`;
    elements.budgetRemainingVal.style.color = '#ef4444';
  }

  elements.budgetStatusPill.textContent = budget.status;
  if (budget.status === 'Within Budget') {
    elements.budgetStatusPill.className = 'badge success';
    elements.budgetWarningBox.className = 'alert-box success';
    elements.budgetWarningIcon.textContent = '✅';
  } else if (budget.status === 'Near Limit') {
    elements.budgetStatusPill.className = 'badge warning';
    elements.budgetWarningBox.className = 'alert-box warning';
    elements.budgetWarningIcon.textContent = '⚠️';
  } else {
    elements.budgetStatusPill.className = 'badge danger';
    elements.budgetWarningBox.className = 'alert-box danger';
    elements.budgetWarningIcon.textContent = '🚨';
  }
  elements.budgetWarningText.textContent = budget.warningMessage;

  elements.expHotelVal.textContent = formatINR(budget.hotelExpense);
  elements.expHotelDetail.textContent = `${budget.selectedHotel.name} (${budget.selectedHotel.tier})`;
  elements.expFoodVal.textContent = formatINR(budget.foodExpense);
  elements.expTransportVal.textContent = formatINR(budget.transportExpense);
  elements.expActivityVal.textContent = formatINR(budget.activityExpense);
  elements.expBufferVal.textContent = formatINR(budget.emergencyBuffer);
  elements.expTotalVal.textContent = formatINR(budget.totalExpense);

  /* 4. Hotels & Cost Savers */
  renderHotelsList(budget.topRecommendations, budget.cheaperAlternatives);

  if (budget.potentialSavings > 0) {
    elements.savingsCallout.classList.remove('hidden');
    elements.savingsCalloutText.textContent =
      `You could save up to ${formatINR(budget.potentialSavings)} on lodging by choosing one of the value hotel alternatives below!`;
  } else {
    elements.savingsCallout.classList.add('hidden');
  }
}

function renderHotelsList(topHotels, cheaperHotels) {
  elements.topHotelsList.innerHTML = '';
  elements.cheaperHotelsList.innerHTML = '';

  if (topHotels && topHotels.length > 0) {
    topHotels.forEach(h => {
      const card = createHotelCard(h, false);
      elements.topHotelsList.appendChild(card);
    });
  } else {
    elements.topHotelsList.innerHTML = '<div class="placeholder-text">No matching hotels found.</div>';
  }

  if (cheaperHotels && cheaperHotels.length > 0) {
    cheaperHotels.forEach(h => {
      const card = createHotelCard(h, true);
      elements.cheaperHotelsList.appendChild(card);
    });
  } else {
    elements.cheaperHotelsList.innerHTML = '<div class="placeholder-text">Selected hotel is already at the lowest available price point.</div>';
  }
}

function createHotelCard(hotel, isCheaperAlt) {
  const card = document.createElement('div');
  card.className = 'hotel-item-card';
  card.innerHTML = `
    <div class="hotel-header">
      <span class="hotel-name">${hotel.name}</span>
      <span class="hotel-price">${formatINR(hotel.pricePerNight)}/night</span>
    </div>
    <div class="hotel-meta">
      <span>⭐ ${hotel.rating.toFixed(1)} / 5.0</span>
      <span>•</span>
      <span>🏷️ ${hotel.tier}</span>
      <span>•</span>
      <span>📍 ${hotel.distFromCenterKm ? `${hotel.distFromCenterKm.toFixed(1)} km to center` : 'Center'}</span>
    </div>
    <div class="hotel-amenities">
      ${isCheaperAlt ? '<strong style="color: #10b981;">✨ Budget-Saver Option</strong>' : (hotel.amenities || 'Comfortable stay')}
    </div>
  `;
  return card;
}

// Client-side hotel sorting
elements.hotelSortSelect.addEventListener('change', (e) => {
  if (!state.currentPlan || !state.currentPlan.budget) return;
  const mode = e.target.value;
  const top = [...state.currentPlan.budget.topRecommendations];
  const cheaper = [...state.currentPlan.budget.cheaperAlternatives];

  const sortFn = (a, b) => {
    if (mode === 'price_asc') return a.pricePerNight - b.pricePerNight;
    if (mode === 'rating') return b.rating - a.rating;
    if (mode === 'distance') return a.distFromCenterKm - b.distFromCenterKm;
    return 0;
  };

  top.sort(sortFn);
  cheaper.sort(sortFn);
  renderHotelsList(top, cheaper);
});

/* -------------------------------------------------------------------------
   SAVING TRIPS
   ------------------------------------------------------------------------- */
if (elements.btnSaveTrip) {
  elements.btnSaveTrip.addEventListener('click', async () => {
    if (!state.currentPlan) return;

    const { formParams, budget, route } = state.currentPlan;
    const destLoc = state.locations.find(l => l.id === formParams.destId);
    const cityName = destLoc ? destLoc.name : (state.currentPlan.destinationCity || 'Vacation');

    const defaultTripName = `${cityName} Tour`;
    const tripName = prompt('Enter a title for this trip itinerary:', defaultTripName) || defaultTripName;

    const today = new Date().toISOString().split('T')[0];
    const startDate = prompt('Enter planned start date (YYYY-MM-DD):', today) || today;

    try {
      const res = await fetch('/api/trips', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          tripName,
          startDate,
          destLocation: cityName,
          numDays: formParams.numDays,
          numTravellers: formParams.numTravellers,
          totalBudget: budget.userBudget,
          estimatedCost: budget.totalExpense,
          totalDistanceKm: route.totalDistanceKm,
          travelStyle: formParams.travelStyle,
          notes: `Trip to ${cityName}. Planned transit: ${route.totalDistanceKm.toFixed(1)} km.`
        })
      });

      const data = await res.json();
      if (data.success) {
        showToast(`Trip #${data.tripId} saved successfully!`, 'success');
        loadSavedTrips();
      } else {
        showToast(data.error || 'Failed to save trip.', 'error');
      }
    } catch (err) {
      showToast('Failed to save trip.', 'error');
    }
  });
}

elements.btnPrintPlan.addEventListener('click', () => {
  window.print();
});

/* -------------------------------------------------------------------------
   SAVED TRIPS TABLE & DETAILED VIEW
   ------------------------------------------------------------------------- */
async function loadSavedTrips() {
  try {
    const res = await fetch('/api/trips');
    const data = await res.json();
    if (data.success) {
      state.savedTrips = data.trips;
      renderSavedTripsTable(state.savedTrips);
    }
  } catch (err) {
    console.error('Error loading trips:', err);
  }
}

function renderSavedTripsTable(trips) {
  elements.savedTripsTbody.innerHTML = '';
  elements.savedCountNav.textContent = trips.length;

  if (trips.length === 0) {
    elements.savedTripsTbody.innerHTML = `<tr><td colspan="8" style="text-align:center; padding: 2rem;">No trips saved yet. Configure and save a trip above!</td></tr>`;
    return;
  }

  trips.forEach(t => {
    const row = document.createElement('tr');
    row.innerHTML = `
      <td><strong>#${t.tripId}</strong></td>
      <td><strong>${t.tripName}</strong></td>
      <td><code>${t.startDate}</code></td>
      <td class="dest-col" style="white-space: nowrap;">📍 ${t.destLocation || t.sourceLocation}</td>
      <td>${t.numDays}d / ${t.numTravellers} guests</td>
      <td><strong style="color: #10b981;">${formatINR(t.estimatedCost)}</strong></td>
      <td><span class="badge info">${t.travelStyle}</span></td>
      <td>
        <div class="trip-action-group">
          <button class="btn-view-trip" data-id="${t.tripId}" title="View Complete Trip Details">View Trip</button>
          <button class="btn-icon-del" data-id="${t.tripId}" title="Remove saved trip">🗑️</button>
        </div>
      </td>
    `;

    row.querySelector('.btn-view-trip').addEventListener('click', () => {
      viewTripDetails(t.tripId);
    });

    row.querySelector('.btn-icon-del').addEventListener('click', async () => {
      if (confirm(`Delete trip #${t.tripId} ("${t.tripName}")?`)) {
        await deleteTrip(t.tripId);
      }
    });

    elements.savedTripsTbody.appendChild(row);
  });
}

/* -------------------------------------------------------------------------
   VIEW SAVED TRIP DETAILS MODAL
   ------------------------------------------------------------------------- */
async function viewTripDetails(tripId) {
  const trip = state.savedTrips.find(t => t.tripId === tripId);
  if (!trip) {
    showToast('Trip not found.', 'error');
    return;
  }

  elements.modalTripName.textContent = trip.tripName;
  elements.modalTripMeta.textContent = `Trip #${trip.tripId} • ${trip.destLocation || 'Destination'} • Departure: ${trip.startDate}`;
  elements.modalTripBody.innerHTML = '<div style="text-align:center; padding:2rem;"><div class="loader" style="display:inline-block;"></div><p style="margin-top:10px;">Generating comprehensive trip itinerary...</p></div>';
  elements.savedTripModal.classList.remove('hidden');

  try {
    // Resolve destination city ID
    const destName = (trip.destLocation || trip.sourceLocation || 'Pune').toLowerCase();
    const matchedLoc = state.locations.find(l => l.name.toLowerCase() === destName);
    const destId = matchedLoc ? matchedLoc.id : 13;

    // Fetch optimized plan from C backend
    const res = await fetch('/api/plan-trip', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        destId,
        numDays: trip.numDays,
        numTravellers: trip.numTravellers,
        userBudget: trip.totalBudget,
        travelStyle: trip.travelStyle || 'Standard'
      })
    });

    const data = await res.json();
    if (!data.success || !data.plan) {
      elements.modalTripBody.innerHTML = `<div class="alert-box danger">Failed to load detailed itinerary: ${data.error || 'Server error'}</div>`;
      return;
    }

    const { route, itinerary, budget } = data.plan;

    // Render comprehensive modal view
    let daysHtml = '';
    itinerary.days.forEach(day => {
      let itemsHtml = '';
      if (day.items && day.items.length > 0) {
        day.items.forEach(it => {
          itemsHtml += `
            <div class="modal-item-row">
              <div>
                <strong>${it.name}</strong>
                <span class="badge info" style="font-size:0.7rem; margin-left:6px;">${it.category}</span>
                <div style="font-size:0.78rem; color:var(--text-muted); margin-top:2px;">
                  ${formatHHMM(it.visitStartTime)} - ${formatHHMM(it.visitEndTime)} (${it.visitDurationMin} mins)
                  ${it.timingNote ? `• ${it.timingNote}` : ''}
                </div>
              </div>
              <div style="font-weight:700; color:var(--text-main);">
                ${it.ticketCost > 0 ? formatINR(it.ticketCost) : 'Free'}
              </div>
            </div>
          `;
        });
      } else {
        itemsHtml = '<div style="font-size:0.85rem; color:var(--text-muted);">Leisure sightseeing and local relaxation.</div>';
      }

      daysHtml += `
        <div class="modal-itinerary-day">
          <div class="modal-day-title">
            <span>Day ${day.dayNumber} Sightseeing</span>
            <span>${day.itemCount} Stops &bull; ${formatINR(day.totalTicketCost)} Tickets</span>
          </div>
          <div>${itemsHtml}</div>
        </div>
      `;
    });

    elements.modalTripBody.innerHTML = `
      <!-- Stat Highlights -->
      <div class="modal-stat-grid">
        <div class="modal-stat-card">
          <div class="modal-stat-label">Destination</div>
          <div class="modal-stat-val">📍 ${trip.destLocation}</div>
        </div>
        <div class="modal-stat-card">
          <div class="modal-stat-label">Starting Location</div>
          <div class="modal-stat-val">Pune (Hub)</div>
        </div>
        <div class="modal-stat-card">
          <div class="modal-stat-label">Duration & Guests</div>
          <div class="modal-stat-val">${trip.numDays} Days / ${trip.numTravellers} Guests</div>
        </div>
        <div class="modal-stat-card">
          <div class="modal-stat-label">Allocated Budget</div>
          <div class="modal-stat-val">${formatINR(trip.totalBudget)}</div>
        </div>
        <div class="modal-stat-card">
          <div class="modal-stat-label">Estimated Spend</div>
          <div class="modal-stat-val" style="color:#10b981;">${formatINR(budget.totalExpense)}</div>
        </div>
        <div class="modal-stat-card">
          <div class="modal-stat-label">Remaining Buffer</div>
          <div class="modal-stat-val" style="color:${budget.remainingBudget >= 0 ? '#10b981' : '#ef4444'};">
            ${formatINR(budget.remainingBudget)}
          </div>
        </div>
      </div>

      <!-- Route & Transit -->
      <div style="background:var(--bg-subtle); padding:1rem; border-radius:var(--radius-md); border:1px solid var(--border-color);">
        <div class="modal-section-title">🧭 Sightseeing Circuit & Route</div>
        <p style="font-size:0.86rem; color:var(--text-muted); margin-bottom:0.5rem;">
          ${route.statusMessage}
        </p>
        <div style="font-size:0.84rem; display:flex; gap:1.25rem;">
          <span><strong>Total Distance:</strong> ${route.totalDistanceKm.toFixed(1)} km</span>
          <span><strong>Est. Transit Time:</strong> ${Math.floor(route.totalTimeMin / 60)}h ${route.totalTimeMin % 60}m</span>
          <span><strong>Planned Stops:</strong> ${itinerary.totalAttractionsScheduled}</span>
        </div>
      </div>

      <!-- Day-by-Day Itinerary Schedule -->
      <div>
        <div class="modal-section-title">📆 Day-by-Day Sightseeing Schedule (Exact TSP Optimized)</div>
        ${daysHtml}
      </div>

      <!-- Accommodation -->
      <div style="border:1px solid var(--border-color); border-radius:var(--radius-md); padding:1rem; background:var(--bg-card);">
        <div class="modal-section-title">🏨 Recommended Accommodation</div>
        <div style="display:flex; justify-content:space-between; align-items:center;">
          <div>
            <strong style="font-size:0.95rem;">${budget.selectedHotel.name}</strong>
            <div style="font-size:0.8rem; color:var(--text-muted); margin-top:2px;">
              ⭐ ${budget.selectedHotel.rating.toFixed(1)} / 5.0 &bull; Tier: ${budget.selectedHotel.tier} &bull; ${budget.selectedHotel.distFromCenterKm ? `${budget.selectedHotel.distFromCenterKm.toFixed(1)} km from center` : 'Central'}
            </div>
          </div>
          <div style="font-size:1rem; font-weight:800; color:var(--text-main);">
            ${formatINR(budget.selectedHotel.pricePerNight)}<span style="font-size:0.75rem; font-weight:400; color:var(--text-muted);">/night</span>
          </div>
        </div>
      </div>

      <!-- Itemized Budget Breakdown -->
      <div style="background:var(--bg-subtle); padding:1.25rem; border-radius:var(--radius-md); border:1px solid var(--border-color);">
        <div class="modal-section-title">💰 Itemized Budget Breakdown (INR ₹)</div>
        <div style="display:grid; grid-template-columns: repeat(auto-fit, minmax(170px, 1fr)); gap:0.75rem; margin-top:0.75rem;">
          <div style="background:var(--bg-card); padding:0.65rem 0.85rem; border-radius:var(--radius-sm); border:1px solid var(--border-color);">
            <div style="font-size:0.75rem; color:var(--text-muted);">Hotel Lodging</div>
            <div style="font-weight:700; margin-top:2px;">${formatINR(budget.hotelExpense)}</div>
          </div>
          <div style="background:var(--bg-card); padding:0.65rem 0.85rem; border-radius:var(--radius-sm); border:1px solid var(--border-color);">
            <div style="font-size:0.75rem; color:var(--text-muted);">Food & Dining</div>
            <div style="font-weight:700; margin-top:2px;">${formatINR(budget.foodExpense)}</div>
          </div>
          <div style="background:var(--bg-card); padding:0.65rem 0.85rem; border-radius:var(--radius-sm); border:1px solid var(--border-color);">
            <div style="font-size:0.75rem; color:var(--text-muted);">Local Transit</div>
            <div style="font-weight:700; margin-top:2px;">${formatINR(budget.transportExpense)}</div>
          </div>
          <div style="background:var(--bg-card); padding:0.65rem 0.85rem; border-radius:var(--radius-sm); border:1px solid var(--border-color);">
            <div style="font-size:0.75rem; color:var(--text-muted);">Attractions & Passes</div>
            <div style="font-weight:700; margin-top:2px;">${formatINR(budget.activityExpense)}</div>
          </div>
          <div style="background:var(--bg-card); padding:0.65rem 0.85rem; border-radius:var(--radius-sm); border:1px solid var(--border-color);">
            <div style="font-size:0.75rem; color:var(--text-muted);">Emergency Buffer</div>
            <div style="font-weight:700; margin-top:2px;">${formatINR(budget.emergencyBuffer)}</div>
          </div>
          <div style="background:var(--primary-light); padding:0.65rem 0.85rem; border-radius:var(--radius-sm); border:1px solid var(--primary);">
            <div style="font-size:0.75rem; color:var(--primary); font-weight:700;">Grand Total</div>
            <div style="font-weight:800; font-size:1.1rem; color:var(--primary); margin-top:2px;">${formatINR(budget.totalExpense)}</div>
          </div>
        </div>
      </div>
    `;

  } catch (err) {
    console.error('Error rendering trip details:', err);
    elements.modalTripBody.innerHTML = '<div class="alert-box danger">Failed to fetch trip details.</div>';
  }
}

function closeTripModal() {
  elements.savedTripModal.classList.add('hidden');
}

if (elements.btnCloseTripModal) {
  elements.btnCloseTripModal.addEventListener('click', closeTripModal);
}
if (elements.btnModalBack) {
  elements.btnModalBack.addEventListener('click', closeTripModal);
}
if (elements.savedTripModal) {
  elements.savedTripModal.addEventListener('click', (e) => {
    if (e.target === elements.savedTripModal) closeTripModal();
  });
}
window.addEventListener('keydown', (e) => {
  if (e.key === 'Escape' && elements.savedTripModal && !elements.savedTripModal.classList.contains('hidden')) {
    closeTripModal();
  }
});

async function deleteTrip(tripId) {
  try {
    const res = await fetch(`/api/trips/${tripId}`, { method: 'DELETE' });
    const data = await res.json();
    if (data.success) {
      showToast(`Trip #${tripId} removed.`, 'info');
      loadSavedTrips();
    } else {
      showToast(data.message || 'Delete failed', 'error');
    }
  } catch (err) {
    showToast('Failed to delete trip.', 'error');
  }
}

elements.inputTripFilter.addEventListener('input', (e) => {
  const term = e.target.value.toLowerCase().trim();
  if (!term) {
    renderSavedTripsTable(state.savedTrips);
    return;
  }
  const filtered = state.savedTrips.filter(t =>
    t.tripName.toLowerCase().includes(term) ||
    t.startDate.includes(term) ||
    (t.destLocation && t.destLocation.toLowerCase().includes(term)) ||
    String(t.tripId).includes(term)
  );
  renderSavedTripsTable(filtered);
});

elements.btnRefreshTrips.addEventListener('click', () => {
  loadSavedTrips();
  showToast('Reloaded saved trips.', 'info');
});

/* -------------------------------------------------------------------------
   ATTRACTION SEARCH WITH COMBINED CITY & CATEGORY FILTERS
   ------------------------------------------------------------------------- */
async function executeSearch() {
  const query = elements.inputSearchQuery ? elements.inputSearchQuery.value.trim() : '';
  const category = state.activeSearchCategory;
  const city = state.activeSearchCity;

  try {
    const url = `/api/search?query=${encodeURIComponent(query)}&category=${encodeURIComponent(category)}&city=${encodeURIComponent(city)}`;
    const res = await fetch(url);
    const data = await res.json();

    if (data.success) {
      renderSearchResults(data.results);
      if (elements.searchCountLabel) {
        elements.searchCountLabel.textContent = data.count;
      }
      updateMapMarkers(data.results);
    }
  } catch (err) {
    console.error('Search error:', err);
  }
}

function renderSearchResults(results) {
  elements.searchResultsContainer.innerHTML = '';

  if (!results || results.length === 0) {
    elements.searchResultsContainer.innerHTML =
      '<div class="placeholder-text">No attractions found matching your search. Try another city or category.</div>';
    return;
  }

  results.forEach(a => {
    const card = document.createElement('div');
    card.className = 'attraction-search-card';
    card.style.cursor = 'pointer';
    card.title = 'Click to focus on map';

    card.innerHTML = `
      <div>
        <div class="attr-card-header">
          <span class="attr-name">${a.name}</span>
          <span class="attr-category-tag">${a.category}</span>
        </div>
        <p class="attr-desc">${a.description}</p>
      </div>
      <div class="attr-footer">
        <div>
          <span>⭐ ${a.rating.toFixed(1)}</span> &bull; 
          <span>📍 ${a.locationName || 'City'}</span>
        </div>
        <div class="attr-fee">${a.ticketCost > 0 ? formatINR(a.ticketCost) : 'Free Entry'}</div>
      </div>
    `;

    // Click attraction card to zoom to its pin on the Leaflet map
    card.addEventListener('click', () => {
      if (state.map && a.latitude && a.longitude) {
        state.map.setView([a.latitude, a.longitude], 14, { animate: true });
        const targetMarker = state.mapMarkers.find(m => m.attractionId === a.id);
        if (targetMarker) {
          targetMarker.openPopup();
        }
        const mapEl = document.getElementById('attractions-map');
        if (mapEl) {
          mapEl.scrollIntoView({ behavior: 'smooth', block: 'center' });
        }
      }
    });

    elements.searchResultsContainer.appendChild(card);
  });
}

elements.inputSearchQuery.addEventListener('input', () => {
  clearTimeout(state.searchDebounceTimer);
  state.searchDebounceTimer = setTimeout(executeSearch, 250);
});

elements.inputSearchQuery.addEventListener('keydown', (e) => {
  if (e.key === 'Enter') {
    e.preventDefault();
    executeSearch();
  }
});

elements.btnSearchTrigger.addEventListener('click', executeSearch);

elements.categoryPills.addEventListener('click', (e) => {
  const pill = e.target.closest('.cat-pill');
  if (!pill) return;

  elements.categoryPills.querySelectorAll('.cat-pill').forEach(p => p.classList.remove('active'));
  pill.classList.add('active');
  state.activeSearchCategory = pill.dataset.cat;
  executeSearch();
});

/* Navigation link active state switching */
document.querySelectorAll('.nav-link').forEach(link => {
  link.addEventListener('click', () => {
    document.querySelectorAll('.nav-link').forEach(l => l.classList.remove('active'));
    link.classList.add('active');
    setTimeout(() => {
      if (state.map) state.map.invalidateSize();
    }, 200);
  });
});

/* Invalidate Leaflet map size on window resize */
window.addEventListener('resize', () => {
  if (state.map) state.map.invalidateSize();
});

/* Initialize application on load */
window.addEventListener('DOMContentLoaded', initAppData);
