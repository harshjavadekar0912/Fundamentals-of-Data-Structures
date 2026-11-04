/**
 * script.js - Frontend application controller for TripPlanner.
 * Includes Dark/Light theme switching, single-city planning,
 * Indian Rupee (INR ₹) formatting, and Exact TSP itinerary rendering.
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
  searchDebounceTimer: null
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
  inputSearchQuery: document.getElementById('input-search-query'),
  btnSearchTrigger: document.getElementById('btn-search-trigger'),
  categoryPills: document.getElementById('category-pills'),
  searchCountLabel: document.getElementById('search-count-label'),
  searchResultsContainer: document.getElementById('search-results-container'),

  // Saved Trips
  savedTripsTbody: document.getElementById('saved-trips-tbody'),
  savedCountNav: document.getElementById('saved-count-nav'),
  inputTripFilter: document.getElementById('input-trip-filter'),
  btnRefreshTrips: document.getElementById('btn-refresh-trips'),
  toastContainer: document.getElementById('toast-container')
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

  try {
    const [locRes, attrRes, hotelRes, tripsRes] = await Promise.all([
      fetch('/api/locations').then(r => r.json()),
      fetch('/api/attractions').then(r => r.json()),
      fetch('/api/hotels').then(r => r.json()),
      fetch('/api/trips').then(r => r.json())
    ]);

    if (locRes.success) {
      state.locations = locRes.locations || [];
      document.getElementById('stat-locations').textContent = state.locations.length;
    }

    if (attrRes.success) {
      state.attractions = attrRes.attractions || [];
      document.getElementById('stat-attractions').textContent = state.attractions.length + '+';
    }

    if (hotelRes.success) {
      state.hotels = hotelRes.hotels || [];
      document.getElementById('stat-hotels').textContent = state.hotels.length + '+';
    }

    if (tripsRes.success) {
      state.savedTrips = tripsRes.trips || [];
      renderSavedTripsTable(state.savedTrips);
    }

    // Populate dropdown and attraction chips only after both locations and attractions are loaded
    if (state.locations.length > 0) {
      populateLocationDropdown();
    }

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
  const chips = elements.attractionChipsContainer.querySelectorAll('.chip-label');
  chips.forEach(chip => {
    chip.classList.remove('active');
    const cb = chip.querySelector('input');
    if (cb) cb.checked = false;
  });
  state.selectedAttrIds.clear();
});

/* -------------------------------------------------------------------------
   TRIP PLAN SUBMISSION & RENDERING
   ------------------------------------------------------------------------- */
elements.tripForm.addEventListener('submit', async (e) => {
  e.preventDefault();

  const destId = parseInt(elements.selectDest.value, 10);
  const numDays = parseInt(elements.inputDays.value, 10);
  const numTravellers = parseInt(elements.inputTravellers.value, 10);
  const userBudget = parseFloat(elements.inputBudget.value);
  const travelStyle = document.querySelector('input[name="travelStyle"]:checked').value;

  const attrIds = Array.from(state.selectedAttrIds);

  elements.planLoader.style.display = 'inline-block';
  elements.btnGeneratePlan.disabled = true;

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

    if (!data.success || !data.plan) {
      showToast(data.error || 'Failed to generate trip plan.', 'error');
      return;
    }

    state.currentPlan = {
      ...data.plan,
      formParams: { destId, numDays, numTravellers, userBudget, travelStyle }
    };

    renderTripPlanDashboard(state.currentPlan);
    elements.resultsSection.classList.remove('hidden');
    elements.resultsSection.scrollIntoView({ behavior: 'smooth' });
    showToast('Your personalized itinerary has been created!', 'success');

  } catch (err) {
    console.error('Plan generation error:', err);
    showToast('Communication error with travel server.', 'error');
  } finally {
    elements.planLoader.style.display = 'none';
    elements.btnGeneratePlan.disabled = false;
  }
});

function renderTripPlanDashboard(plan) {
  const { route, itinerary, budget, formParams } = plan;
  const destLoc = state.locations.find(l => l.id === formParams.destId);
  const cityName = destLoc ? destLoc.name : (plan.destinationCity || 'Destination');

  document.getElementById('plan-title-heading').textContent =
    `${cityName} City Vacation Blueprint`;
  document.getElementById('plan-sub-heading').textContent =
    `${formParams.numDays} Days • ${formParams.numTravellers} Guests • ${formParams.travelStyle} Style • Fully Optimized`;

  /* 1. Route Section */
  elements.metricDist.textContent = `${route.totalDistanceKm.toFixed(1)} km`;
  const hrs = Math.floor(route.totalTimeMin / 60);
  const mins = route.totalTimeMin % 60;
  elements.metricTime.textContent = `${hrs > 0 ? `${hrs}h ` : ''}${mins}m`;
  elements.metricStops.textContent = itinerary.totalAttractionsScheduled;
  elements.metricDays.textContent = `${formParams.numDays} Days`;

  elements.routeStatusText.textContent = route.statusMessage;
  elements.routeReachableBadge.textContent = `${cityName} Tour`;
  elements.routeReachableBadge.className = 'badge';
  elements.routeStatusAlert.className = 'alert-box success';

  // Render hops
  elements.hopsContainer.innerHTML = '';
  if (route.hops.length === 0) {
    elements.hopsContainer.innerHTML = `<div class="hop-item"><span>Scenic walking & central transit route mapped for ${cityName}.</span></div>`;
  } else {
    route.hops.forEach((hop, i) => {
      const hopItem = document.createElement('div');
      hopItem.className = 'hop-item';
      hopItem.innerHTML = `
        <div class="hop-route">
          <span>Day ${hop.day || 1} &bull; ${hop.fromName}</span>
          <span class="hop-arrow">➔</span>
          <span>${hop.toName}</span>
        </div>
        <div class="hop-stats">
          <span>${hop.distanceKm.toFixed(1)} km</span>
          <span>${hop.travelTimeMin} mins</span>
        </div>
      `;
      elements.hopsContainer.appendChild(hopItem);
    });
  }

  /* 2. Day-Wise Itinerary */
  elements.totalScheduledBadge.textContent = `${itinerary.totalAttractionsScheduled} Attractions Scheduled`;
  elements.daySchedulesList.innerHTML = '';

  itinerary.days.forEach(day => {
    const dayCard = document.createElement('div');
    dayCard.className = 'day-card';

    const activeH = Math.floor(day.totalActiveMinutes / 60);
    const activeM = day.totalActiveMinutes % 60;

    let itemsHtml = '';
    if (day.items.length === 0) {
      itemsHtml = '<div class="timeline-item"><div class="item-content">Rest & relaxation day, or free local shopping.</div></div>';
    } else {
      day.items.forEach(item => {
        itemsHtml += `
          <div class="timeline-item">
            <div class="time-slot">${formatHHMM(item.visitStartTime)} - ${formatHHMM(item.visitEndTime)}</div>
            <div class="item-content">
              <h5>${item.name}</h5>
              <div class="item-meta">
                <span>📂 ${item.category}</span>
                <span>⏱️ Duration: ${item.visitDurationMin} mins</span>
                <span>🚗 Transit: ${item.arrivalTime < item.visitStartTime ? 'Early wait' : 'Direct'}</span>
              </div>
              <div class="item-note">💡 ${item.timingNote}</div>
            </div>
            <div class="item-cost">${item.ticketCost > 0 ? formatINR(item.ticketCost) : 'Free Entry'}</div>
          </div>
        `;
      });
    }

    dayCard.innerHTML = `
      <div class="day-card-header">
        <h4>Day ${day.dayNumber} Itinerary</h4>
        <div class="day-summary-badges">
          <span class="day-badge">Active: ${activeH}h ${activeM}m</span>
          <span class="day-badge">Tickets: ${formatINR(day.totalTicketCost)}</span>
          <span class="day-badge">${day.items.length} Sights</span>
        </div>
      </div>
      <div class="day-items-timeline">${itemsHtml}</div>
    `;

    elements.daySchedulesList.appendChild(dayCard);
  });

  /* 3. Budget Breakdown in INR (₹) */
  elements.budgetSpentLabel.textContent = formatINR(budget.totalExpense);
  elements.budgetTotalLabel.textContent = formatINR(budget.userBudget);
  elements.budgetRemainingVal.textContent = formatINR(budget.remainingBudget);

  const percentUsed = Math.min(100, Math.max(0, (budget.totalExpense / budget.userBudget) * 100));
  elements.budgetProgressFill.style.width = `${percentUsed}%`;

  elements.budgetStatusPill.textContent = budget.status;
  if (budget.status === 'Within Budget') {
    elements.budgetStatusPill.className = 'badge';
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

  const defaultTripName = `${cityName} Expedition`;
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
   SAVED TRIPS TABLE
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
      <td>📍 ${t.destLocation || t.sourceLocation}</td>
      <td>${t.numDays}d / ${t.numTravellers} guests</td>
      <td><strong style="color: #10b981;">${formatINR(t.estimatedCost)}</strong></td>
      <td><span class="badge info">${t.travelStyle}</span></td>
      <td>
        <button class="btn-icon-del" data-id="${t.tripId}" title="Remove saved trip">🗑️ Delete</button>
      </td>
    `;

    row.querySelector('.btn-icon-del').addEventListener('click', async () => {
      if (confirm(`Delete trip #${t.tripId} ("${t.tripName}")?`)) {
        await deleteTrip(t.tripId);
      }
    });

    elements.savedTripsTbody.appendChild(row);
  });
}

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
   ATTRACTION SEARCH
   ------------------------------------------------------------------------- */
async function executeSearch() {
  const query = elements.inputSearchQuery.value.trim();
  const category = state.activeSearchCategory;

  try {
    const url = `/api/search?query=${encodeURIComponent(query)}&category=${encodeURIComponent(category)}`;
    const res = await fetch(url);
    const data = await res.json();

    if (data.success) {
      renderSearchResults(data.results);
      elements.searchCountLabel.textContent = data.count;
    }
  } catch (err) {
    console.error('Search error:', err);
  }
}

function renderSearchResults(results) {
  elements.searchResultsContainer.innerHTML = '';

  if (!results || results.length === 0) {
    elements.searchResultsContainer.innerHTML =
      '<div class="placeholder-text">No attractions found matching your search. Try another city or keyword.</div>';
    return;
  }

  results.forEach(a => {
    const card = document.createElement('div');
    card.className = 'attraction-search-card';
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
  });
});

/* Initialize application on load */
window.addEventListener('DOMContentLoaded', initAppData);
