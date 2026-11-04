/**
 * script.js - Frontend application controller for TripPlanner.
 * Communicates with Node.js Express API -> C DSA Engine.
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

// DOM Elements
const elements = {
  selectSrc: document.getElementById('select-src'),
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
  btnSaveTripAvl: document.getElementById('btn-save-trip-avl'),
  btnPrintPlan: document.getElementById('btn-print-plan'),

  // Route metrics
  metricDist: document.getElementById('metric-dist'),
  metricTime: document.getElementById('metric-time'),
  metricTolls: document.getElementById('metric-tolls'),
  metricHops: document.getElementById('metric-hops'),
  routeStatusAlert: document.getElementById('route-status-alert'),
  routeStatusText: document.getElementById('route-status-text'),
  routeReachableBadge: document.getElementById('route-reachable-badge'),
  hopsContainer: document.getElementById('hops-container'),

  // TSP Comparison
  compGreedyDist: document.getElementById('comp-greedy-dist'),
  compGreedyTime: document.getElementById('comp-greedy-time'),
  compGreedyUs: document.getElementById('comp-greedy-us'),
  compExactDist: document.getElementById('comp-exact-dist'),
  compExactTime: document.getElementById('comp-exact-time'),
  compExactUs: document.getElementById('comp-exact-us'),
  compCommentary: document.getElementById('comp-commentary'),
  compRecommendation: document.getElementById('comp-recommendation'),

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

  // Unit Test Modal
  btnRunTests: document.getElementById('btn-run-tests'),
  testModal: document.getElementById('test-modal'),
  modalCloseBtn: document.getElementById('modal-close-btn'),
  modalOkBtn: document.getElementById('modal-ok-btn'),
  testOutputConsole: document.getElementById('test-output-console'),
  toastContainer: document.getElementById('toast-container')
};

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
  try {
    const [locRes, attrRes, hotelRes, tripsRes] = await Promise.all([
      fetch('/api/locations').then(r => r.json()),
      fetch('/api/attractions').then(r => r.json()),
      fetch('/api/hotels').then(r => r.json()),
      fetch('/api/trips').then(r => r.json())
    ]);

    if (locRes.success) {
      state.locations = locRes.locations;
      populateLocationDropdowns();
      document.getElementById('stat-locations').textContent = state.locations.length;
    }

    if (attrRes.success) {
      state.attractions = attrRes.attractions;
      document.getElementById('stat-attractions').textContent = state.attractions.length;
    }

    if (hotelRes.success) {
      state.hotels = hotelRes.hotels;
      document.getElementById('stat-hotels').textContent = state.hotels.length;
    }

    if (tripsRes.success) {
      state.savedTrips = tripsRes.trips;
      renderSavedTripsTable(state.savedTrips);
    }

    // Initial search load
    executeSearch();

  } catch (err) {
    console.error('Initialization error:', err);
    showToast('Failed to connect to C backend server. Please verify server is running.', 'error');
  }
}

function populateLocationDropdowns() {
  elements.selectSrc.innerHTML = '';
  elements.selectDest.innerHTML = '';

  state.locations.forEach(loc => {
    const optSrc = document.createElement('option');
    optSrc.value = loc.id;
    optSrc.textContent = `${loc.name} (${loc.state})`;
    elements.selectSrc.appendChild(optSrc);

    const optDest = document.createElement('option');
    optDest.value = loc.id;
    optDest.textContent = `${loc.name} (${loc.state})`;
    elements.selectDest.appendChild(optDest);
  });

  // Defaults: Delhi (0) to Agra (1)
  if (state.locations.length > 1) {
    elements.selectSrc.value = '0';
    elements.selectDest.value = '1';
    updateAttractionChipsForDestination(1);
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
      if (state.selectedAttrIds.has(id)) {
        state.selectedAttrIds.delete(id);
        chip.classList.remove('active');
      } else {
        state.selectedAttrIds.add(id);
        chip.classList.add('active');
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
  });
});

elements.btnClearAttrs.addEventListener('click', () => {
  const chips = elements.attractionChipsContainer.querySelectorAll('.chip-label');
  chips.forEach(chip => {
    chip.classList.remove('active');
  });
  state.selectedAttrIds.clear();
});

/* -------------------------------------------------------------------------
   TRIP PLAN SUBMISSION & RENDERING
   ------------------------------------------------------------------------- */
elements.tripForm.addEventListener('submit', async (e) => {
  e.preventDefault();

  const srcId = parseInt(elements.selectSrc.value, 10);
  const destId = parseInt(elements.selectDest.value, 10);
  const numDays = parseInt(elements.inputDays.value, 10);
  const numTravellers = parseInt(elements.inputTravellers.value, 10);
  const userBudget = parseFloat(elements.inputBudget.value);
  const travelStyle = document.querySelector('input[name="travelStyle"]:checked').value;
  const algoMode = document.querySelector('input[name="algoMode"]:checked').value;
  const preferExact = (algoMode === 'greedy') ? 0 : 1;

  const attrIds = Array.from(state.selectedAttrIds);

  elements.planLoader.style.display = 'inline-block';
  elements.btnGeneratePlan.disabled = true;

  try {
    const response = await fetch('/api/plan-trip', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        srcId,
        destId,
        numDays,
        numTravellers,
        userBudget,
        travelStyle,
        preferExact,
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
      formParams: { srcId, destId, numDays, numTravellers, userBudget, travelStyle }
    };

    renderTripPlanDashboard(state.currentPlan);
    elements.resultsSection.classList.remove('hidden');
    elements.resultsSection.scrollIntoView({ behavior: 'smooth' });
    showToast('Complete trip blueprint computed successfully in C!', 'success');

  } catch (err) {
    console.error('Plan generation error:', err);
    showToast('Communication error with C backend API.', 'error');
  } finally {
    elements.planLoader.style.display = 'none';
    elements.btnGeneratePlan.disabled = false;
  }
});

function formatHHMM(hhmm) {
  const hours = Math.floor(hhmm / 100);
  const mins = hhmm % 100;
  const period = hours >= 12 ? 'PM' : 'AM';
  const displayHours = hours % 12 === 0 ? 12 : hours % 12;
  return `${String(displayHours).padStart(2, '0')}:${String(mins).padStart(2, '0')} ${period}`;
}

function renderTripPlanDashboard(plan) {
  const { route, itinerary, budget, formParams } = plan;

  const srcLoc = state.locations.find(l => l.id === formParams.srcId);
  const destLoc = state.locations.find(l => l.id === formParams.destId);

  document.getElementById('plan-title-heading').textContent =
    `${srcLoc ? srcLoc.name : 'Origin'} to ${destLoc ? destLoc.name : 'Destination'} Expedition`;
  document.getElementById('plan-sub-heading').textContent =
    `${formParams.numDays} Days • ${formParams.numTravellers} Travelers • ${formParams.travelStyle} Style • Fully Optimized in C`;

  /* 1. Route Section */
  elements.metricDist.textContent = `${route.totalDistanceKm.toFixed(1)} km`;
  const hrs = Math.floor(route.totalTimeMin / 60);
  const mins = route.totalTimeMin % 60;
  elements.metricTime.textContent = `${hrs}h ${mins}m`;
  elements.metricTolls.textContent = `$${route.totalTollCost.toFixed(2)}`;
  elements.metricHops.textContent = route.hops.length;

  elements.routeStatusText.textContent = route.statusMessage;

  if (route.reachable) {
    elements.routeReachableBadge.textContent = 'Path Found (Dijkstra)';
    elements.routeReachableBadge.className = 'badge';
    elements.routeStatusAlert.className = 'alert-box success';
  } else {
    elements.routeReachableBadge.textContent = 'Unreachable Destination';
    elements.routeReachableBadge.className = 'badge danger';
    elements.routeStatusAlert.className = 'alert-box danger';
  }

  // Render hops
  elements.hopsContainer.innerHTML = '';
  if (route.hops.length === 0) {
    elements.hopsContainer.innerHTML = `<div class="hop-item"><span>Intra-city journey or single vertex route.</span></div>`;
  } else {
    route.hops.forEach((hop, i) => {
      const hopItem = document.createElement('div');
      hopItem.className = 'hop-item';
      const hopH = Math.floor(hop.travelTimeMin / 60);
      const hopM = hop.travelTimeMin % 60;
      hopItem.innerHTML = `
        <div class="hop-route">
          <span>Stop ${i + 1}: ${hop.fromName}</span>
          <span class="hop-arrow">➔</span>
          <span>${hop.toName}</span>
        </div>
        <div class="hop-stats">
          <span>${hop.distanceKm.toFixed(1)} km</span>
          <span>${hopH > 0 ? `${hopH}h ` : ''}${hopM}m</span>
        </div>
      `;
      elements.hopsContainer.appendChild(hopItem);
    });
  }

  /* 2. TSP Algorithmic Comparison */
  const comp = itinerary.comparison;
  elements.compGreedyDist.textContent = `${comp.greedyKm.toFixed(1)} km`;
  elements.compExactDist.textContent = `${comp.exactKm.toFixed(1)} km`;
  elements.compGreedyUs.textContent = `~12 μs`;
  elements.compExactUs.textContent = `~35 μs`;

  elements.compCommentary.textContent = comp.comment;
  elements.compRecommendation.textContent = `Optimal Choice: ${comp.recommendedAlgorithm}`;

  /* 3. Day-Wise Itinerary */
  elements.totalScheduledBadge.textContent = `${itinerary.totalAttractionsScheduled} Attractions Scheduled`;
  elements.daySchedulesList.innerHTML = '';

  itinerary.days.forEach(day => {
    const dayCard = document.createElement('div');
    dayCard.className = 'day-card';

    const activeH = Math.floor(day.totalActiveMinutes / 60);
    const activeM = day.totalActiveMinutes % 60;

    let itemsHtml = '';
    if (day.items.length === 0) {
      itemsHtml = '<div class="timeline-item"><div class="item-content">Rest & leisure day, or free exploration.</div></div>';
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
            <div class="item-cost">$${item.ticketCost.toFixed(2)}</div>
          </div>
        `;
      });
    }

    dayCard.innerHTML = `
      <div class="day-card-header">
        <h4>Day ${day.dayNumber} Itinerary</h4>
        <div class="day-summary-badges">
          <span class="day-badge">Active: ${activeH}h ${activeM}m</span>
          <span class="day-badge">Tickets: $${day.totalTicketCost.toFixed(2)}</span>
          <span class="day-badge">${day.items.length} Stops</span>
        </div>
      </div>
      <div class="day-items-timeline">${itemsHtml}</div>
    `;

    elements.daySchedulesList.appendChild(dayCard);
  });

  /* 4. Budget Breakdown */
  elements.budgetSpentLabel.textContent = `$${budget.totalExpense.toFixed(2)}`;
  elements.budgetTotalLabel.textContent = `$${budget.userBudget.toFixed(2)}`;
  elements.budgetRemainingVal.textContent = `$${budget.remainingBudget.toFixed(2)}`;

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

  elements.expHotelVal.textContent = `$${budget.hotelExpense.toFixed(2)}`;
  elements.expHotelDetail.textContent = `${budget.selectedHotel.name} (${budget.selectedHotel.tier})`;
  elements.expFoodVal.textContent = `$${budget.foodExpense.toFixed(2)}`;
  elements.expTransportVal.textContent = `$${budget.transportExpense.toFixed(2)}`;
  elements.expActivityVal.textContent = `$${budget.activityExpense.toFixed(2)}`;
  elements.expBufferVal.textContent = `$${budget.emergencyBuffer.toFixed(2)}`;
  elements.expTotalVal.textContent = `$${budget.totalExpense.toFixed(2)}`;

  /* 5. Hotels & Min-Heap Savings */
  renderHotelsList(budget.topRecommendations, budget.cheaperAlternatives);

  if (budget.potentialSavings > 0) {
    elements.savingsCallout.classList.remove('hidden');
    elements.savingsCalloutText.textContent =
      `You could save up to $${budget.potentialSavings.toFixed(2)} on accommodation by choosing one of the Min-Heap cheaper alternatives below!`;
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
      <span class="hotel-price">$${hotel.pricePerNight.toFixed(2)}/nt</span>
    </div>
    <div class="hotel-meta">
      <span>⭐ ${hotel.rating.toFixed(1)} / 5.0</span>
      <span>•</span>
      <span>🏷️ ${hotel.tier}</span>
      <span>•</span>
      <span>📍 ${hotel.distFromCenterKm ? `${hotel.distFromCenterKm.toFixed(1)} km to center` : 'Center'}</span>
    </div>
    <div class="hotel-amenities">
      ${isCheaperAlt ? '<strong style="color: #059669;">✨ Min-Heap Budget Saver</strong>' : 'Comfort & service guaranteed'}
    </div>
  `;
  return card;
}

/* -------------------------------------------------------------------------
   SAVING TRIPS TO AVL TREE (C BACKEND)
   ------------------------------------------------------------------------- */
elements.btnSaveTripAvl.addEventListener('click', async () => {
  if (!state.currentPlan) return;

  const { formParams, budget, route } = state.currentPlan;
  const srcLoc = state.locations.find(l => l.id === formParams.srcId);
  const destLoc = state.locations.find(l => l.id === formParams.destId);

  const defaultTripName = `${destLoc ? destLoc.name : 'Destination'} Expedition`;
  const tripName = prompt('Enter a title for this trip record to store in the AVL Tree:', defaultTripName) || defaultTripName;

  const today = new Date().toISOString().split('T')[0];
  const startDate = prompt('Enter planned start date (YYYY-MM-DD):', today) || today;

  try {
    const res = await fetch('/api/trips', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        tripName,
        startDate,
        sourceLocation: srcLoc ? srcLoc.name : 'Origin',
        destLocation: destLoc ? destLoc.name : 'Destination',
        numDays: formParams.numDays,
        numTravellers: formParams.numTravellers,
        totalBudget: budget.userBudget,
        estimatedCost: budget.totalExpense,
        totalDistanceKm: route.totalDistanceKm,
        travelStyle: formParams.travelStyle,
        notes: `Saved via C DSA AVL Engine. Shortest path: ${route.totalDistanceKm.toFixed(1)} km.`
      })
    });

    const data = await res.json();
    if (data.success) {
      showToast(`Trip #${data.tripId} saved to AVL Tree successfully!`, 'success');
      loadSavedTrips();
    } else {
      showToast(data.error || 'Failed to save trip.', 'error');
    }
  } catch (err) {
    showToast('Failed to save trip to AVL Tree.', 'error');
  }
});

elements.btnPrintPlan.addEventListener('click', () => {
  window.print();
});

/* -------------------------------------------------------------------------
   SAVED TRIPS TABLE & AVL TREE OPERATIONS
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
    console.error('Error loading trips from AVL:', err);
  }
}

function renderSavedTripsTable(trips) {
  elements.savedTripsTbody.innerHTML = '';
  elements.savedCountNav.textContent = trips.length;

  if (trips.length === 0) {
    elements.savedTripsTbody.innerHTML = `<tr><td colspan="8" style="text-align:center; padding: 2rem;">No trips saved in the AVL Tree yet.</td></tr>`;
    return;
  }

  trips.forEach(t => {
    const row = document.createElement('tr');
    row.innerHTML = `
      <td><strong>#${t.tripId}</strong></td>
      <td><strong>${t.tripName}</strong></td>
      <td><code>${t.startDate}</code></td>
      <td>${t.sourceLocation} ➔ ${t.destLocation}</td>
      <td>${t.numDays}d / ${t.numTravellers}p</td>
      <td><strong style="color: #059669;">$${t.estimatedCost.toFixed(2)}</strong></td>
      <td><span class="badge info">${t.travelStyle}</span></td>
      <td>
        <button class="btn-icon-del" data-id="${t.tripId}" title="Delete node from AVL Tree">🗑️ Delete</button>
      </td>
    `;

    row.querySelector('.btn-icon-del').addEventListener('click', async () => {
      if (confirm(`Delete Trip #${t.tripId} ("${t.tripName}") from AVL Tree?`)) {
        await deleteTripFromAvl(t.tripId);
      }
    });

    elements.savedTripsTbody.appendChild(row);
  });
}

async function deleteTripFromAvl(tripId) {
  try {
    const res = await fetch(`/api/trips/${tripId}`, { method: 'DELETE' });
    const data = await res.json();
    if (data.success) {
      showToast(`Trip #${tripId} deleted from AVL Tree and rebalanced.`, 'info');
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
    t.sourceLocation.toLowerCase().includes(term) ||
    t.destLocation.toLowerCase().includes(term) ||
    String(t.tripId).includes(term)
  );
  renderSavedTripsTable(filtered);
});

elements.btnRefreshTrips.addEventListener('click', () => {
  loadSavedTrips();
  showToast('Reloaded trips from AVL Tree.', 'info');
});

/* -------------------------------------------------------------------------
   HASH TABLE SEARCH (MODULE 4)
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
      '<div class="placeholder-text">No attractions found matching the Hash Table query. Try a different term or category.</div>';
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
        <div class="attr-fee">${a.ticketCost > 0 ? `$${a.ticketCost.toFixed(2)}` : 'Free Entry'}</div>
      </div>
    `;
    elements.searchResultsContainer.appendChild(card);
  });
}

// Live search input with debounce
elements.inputSearchQuery.addEventListener('input', () => {
  clearTimeout(state.searchDebounceTimer);
  state.searchDebounceTimer = setTimeout(executeSearch, 250);
});

elements.btnSearchTrigger.addEventListener('click', executeSearch);

// Category filter pills
elements.categoryPills.addEventListener('click', (e) => {
  const pill = e.target.closest('.cat-pill');
  if (!pill) return;

  elements.categoryPills.querySelectorAll('.cat-pill').forEach(p => p.classList.remove('active'));
  pill.classList.add('active');
  state.activeSearchCategory = pill.dataset.cat;
  executeSearch();
});

/* -------------------------------------------------------------------------
   UNIT TEST RUNNER MODAL
   ------------------------------------------------------------------------- */
elements.btnRunTests.addEventListener('click', async () => {
  elements.testModal.classList.remove('hidden');
  elements.testOutputConsole.textContent = 'Invoking C executable with --api run_tests...\n';

  try {
    const res = await fetch('/api/tests');
    const text = await res.text();
    elements.testOutputConsole.textContent = text;
  } catch (err) {
    elements.testOutputConsole.textContent = `Error executing tests: ${err.message}`;
  }
});

elements.modalCloseBtn.addEventListener('click', () => {
  elements.testModal.classList.add('hidden');
});

elements.modalOkBtn.addEventListener('click', () => {
  elements.testModal.classList.add('hidden');
});

// Close modal on click outside
elements.testModal.addEventListener('click', (e) => {
  if (e.target === elements.testModal) {
    elements.testModal.classList.add('hidden');
  }
});

/* Initialize application on load */
window.addEventListener('DOMContentLoaded', initAppData);

