// Customer portal logic and event handling
let currentUser = null;
let currentFilter = 'All';
let availableVehicles = [];

document.addEventListener('DOMContentLoaded', () => {
  currentUser = api.checkAuth(['Customer']);
  if (!currentUser) return;

  // Load initial data
  loadVehicles();
  loadRentalHistory();
  loadNotifications();

  // Set minimum date for date inputs to today
  const today = new Date().toISOString().split('T')[0];
  const startEl = document.getElementById('rentStartDate');
  const endEl = document.getElementById('rentEndDate');
  if (startEl) startEl.min = today;
  if (endEl) endEl.min = today;
});

// Switch between Available Vehicles and Rental History tabs
function switchCustomerTab(tab) {
  const panelVehicles = document.getElementById('tabPanelVehicles');
  const panelHistory = document.getElementById('tabPanelHistory');
  const tabVehicles = document.getElementById('tabNavVehicles');
  const tabHistory = document.getElementById('tabNavHistory');

  if (tab === 'vehicles') {
    panelVehicles.style.display = 'block';
    panelHistory.style.display = 'none';
    tabVehicles.classList.add('active');
    tabHistory.classList.remove('active');
    loadVehicles();
  } else {
    panelHistory.style.display = 'block';
    panelVehicles.style.display = 'none';
    tabHistory.classList.add('active');
    tabVehicles.classList.remove('active');
    loadRentalHistory();
  }
}

// Fetch available vehicles and live inventory counts
async function loadVehicles() {
  const res = await api.get(`/api/vehicles?type=${currentFilter === 'All' ? '' : currentFilter}`);
  const countsRes = await api.get('/api/vehicles/count');

  if (countsRes.success && countsRes.counts) {
    const c = countsRes.counts;
    const fleetEl = document.getElementById('fleetStatusText');
    if (fleetEl) {
      fleetEl.textContent = `Fleet: ${c.availableCars} Cars & ${c.availableMotobikes} Motorbikes available`;
    }
  }

  const grid = document.getElementById('vehicleGridList');
  if (!grid) return;
  grid.innerHTML = '';

  if (!res.success || !res.data || res.data.length === 0) {
    grid.innerHTML = `<div style="grid-column: 1/-1; text-align: center; padding: 3rem; color: var(--gray);">
      No vehicles in this category are currently available.
    </div>`;
    return;
  }

  availableVehicles = res.data;

  res.data.forEach(v => {
    const isCar = (v.type === 'Car');
    const card = document.createElement('div');
    card.className = 'vehicleCard';
    card.innerHTML = `
      <div class="cardTopBadges">
        <span class="vehicleTypeTag ${isCar ? 'tagCar' : 'tagMoto'}">
          ${isCar ? 'CAR' : 'MOTORBIKE'}
        </span>
        <span class="statusReadyBadge">Ready</span>
      </div>

      <h4 class="vehicleNameTitle">${v.brand} ${v.model}</h4>
      <div class="licensePlatePill">${v.licenseplate}</div>

      <div class="specDetailRow">
        <span>${isCar ? 'Seats:' : 'Engine capacity:'}</span>
        <strong>${isCar ? `${v.numseat} seats` : `${v.enginecapacity} cc`}</strong>
      </div>
      <div class="specDetailRow">
        <span>Status:</span>
        <span class="specDetailStatus">Available</span>
      </div>

      <div class="vehiclePriceDaily">
        ${api.formatMoney(v.priceperday)} <span>/ day</span>
      </div>

      <button class="rentThisVehicleBtn" onclick="openRentModal(${v.vehicleID})">
        Rent This Vehicle
      </button>
    `;
    grid.appendChild(card);
  });
}

// Filter vehicle display by category
function filterCustomerVehicles(type) {
  currentFilter = type;
  document.getElementById('filterAll').classList.toggle('active', type === 'All');
  document.getElementById('filterCar').classList.toggle('active', type === 'Car');
  document.getElementById('filterMoto').classList.toggle('active', type === 'Motobike');
  loadVehicles();
}

// Open rental modal and populate vehicle details
function openRentModal(vehId) {
  const v = availableVehicles.find(item => item.vehicleID === vehId);
  if (!v) return;

  document.getElementById('rentVehId').value = v.vehicleID;
  document.getElementById('rentVehPrice').value = v.priceperday;
  document.getElementById('rentModalTitle').textContent = `Rent Vehicle: ${v.brand} ${v.model}`;
  document.getElementById('rentVehName').textContent = `${v.brand} ${v.model} (${v.type === 'Car' ? 'Car' : 'Motorbike'})`;
  document.getElementById('rentVehPlate').textContent = v.licenseplate;
  document.getElementById('rentVehPriceText').textContent = api.formatMoney(v.priceperday);

  if (currentUser.cccd) document.getElementById('rentCccd').value = currentUser.cccd;
  if (currentUser.driverLicense) document.getElementById('rentLicense').value = currentUser.driverLicense;

  const today = new Date();
  const tomorrow = new Date();
  tomorrow.setDate(today.getDate() + 2);

  document.getElementById('rentStartDate').value = today.toISOString().split('T')[0];
  document.getElementById('rentEndDate').value = tomorrow.toISOString().split('T')[0];

  const depInput = document.getElementById('rentDeposit');
  if (depInput) {
    depInput.value = '';
    depInput.dataset.auto = '1';
  }

  calcCustomerEstimatedTotal();

  document.getElementById('rentModalAlert').style.display = 'none';
  document.getElementById('rentModal').classList.add('active');
}

// Close rental booking modal
function closeRentModal() {
  document.getElementById('rentModal').classList.remove('active');
}

// Calculate estimated rental cost and required deposit
function calcCustomerEstimatedTotal() {
  const price = parseFloat(document.getElementById('rentVehPrice').value) || 0;
  const start = document.getElementById('rentStartDate').value;
  const end = document.getElementById('rentEndDate').value;

  if (start && end) {
    const d1 = new Date(start);
    const d2 = new Date(end);
    let diffDays = Math.ceil((d2 - d1) / (1000 * 60 * 60 * 24));
    if (diffDays <= 0) diffDays = 1;

    const total = diffDays * price;
    const deposit = Math.round(total * 0.3); // 30% deposit
    document.getElementById('rentDaysText').textContent = `${diffDays} days`;
    document.getElementById('rentEstimatedTotal').textContent = api.formatMoney(total);

    const depInput = document.getElementById('rentDeposit');
    if (depInput && (!depInput.value || depInput.dataset.auto === '1')) {
      depInput.value = deposit;
      depInput.dataset.auto = '1';
    }
  }
}

// Submit customer rental booking request
async function submitCustomerRental(event) {
  event.preventDefault();
  const vehId = parseInt(document.getElementById('rentVehId').value);
  const cccd = document.getElementById('rentCccd').value.trim();
  const license = document.getElementById('rentLicense').value.trim();
  const start = document.getElementById('rentStartDate').value;
  const end = document.getElementById('rentEndDate').value;
  const deposit = parseFloat(document.getElementById('rentDeposit').value) || 0;
  const alertBox = document.getElementById('rentModalAlert');

  if (new Date(end) < new Date(start)) {
    alertBox.textContent = 'End date cannot be earlier than start date!';
    alertBox.style.display = 'block';
    return;
  }

  const btn = document.getElementById('btnConfirmRent');
  btn.disabled = true;
  btn.textContent = 'Processing rental...';

  const res = await api.post('/api/rentals/create', {
    customerId: currentUser.userId,
    vehicleId: vehId,
    cccd: cccd,
    driverLicense: license,
    startDate: start,
    endDate: end,
    depositAmount: deposit
  });

  btn.disabled = false;
  btn.textContent = 'Confirm Rental';

  if (res.success) {
    currentUser.cccd = cccd;
    currentUser.driverLicense = license;
    api.setUser(currentUser);

    closeRentModal();
    alert('Rental booking successful! Your request is currently pending approval by the manager.');
    loadVehicles();
    loadNotifications();
    switchCustomerTab('history');
  } else {
    alertBox.textContent = res.message || 'Error occurred while booking rental!';
    alertBox.style.display = 'block';
  }
}

// Load customer personal rental history
async function loadRentalHistory() {
  const tbody = document.getElementById('historyTableBody');
  if (!tbody) return;

  const res = await api.get(`/api/rentals/history?customerId=${currentUser.userId}`);

  if (!res.success || !res.data || res.data.length === 0) {
    tbody.innerHTML = `<tr><td colspan="8" style="text-align:center; padding: 2rem; color: var(--gray);">
      You have no rental orders.
    </td></tr>`;
    return;
  }

  tbody.innerHTML = '';
  res.data.forEach(o => {
    let statusHtml = '';
    let actionHtml = '';

    if (o.status === 'Approved') {
      statusHtml = '<span class="statusBadge statusApproved">Currently Rented</span>';
      actionHtml = `<button class="btn btnSuccess btnSm" onclick="handleReturnVehicle(${o.orderID}, '${o.vehicleBrand} ${o.vehicleModel}')" title="Confirm Return Vehicle">
        Return Vehicle
      </button>`;
    } else if (o.status === 'Completed') {
      statusHtml = '<span class="statusBadge statusCompleted">Vehicle Returned</span>';
      actionHtml = '<span style="color:#059669; font-size:12px; font-weight:600;">Completed</span>';
    } else if (o.status === 'Rejected') {
      statusHtml = '<span class="statusBadge statusRejected">Rejected</span>';
      actionHtml = '<span style="color:var(--gray); font-size:12px;">Declined</span>';
    } else {
      statusHtml = '<span class="statusBadge statusPending">Pending</span>';
      actionHtml = '<span style="color:var(--gray); font-size:12px;">Pending Approval</span>';
    }

    const tr = document.createElement('tr');
    tr.innerHTML = `
      <td><strong>#${o.orderID}</strong></td>
      <td><strong>${o.vehicleBrand} ${o.vehicleModel}</strong></td>
      <td><span class="licensePlatePill">${o.licensePlate}</span></td>
      <td>${o.vehicleType === 'Car' ? 'Car' : 'Motorbike'}</td>
      <td>${o.startDate} - ${o.endDate}</td>
      <td style="font-weight:700; color:var(--primary);">${api.formatMoney(o.totalAmount)}</td>
      <td>${statusHtml}</td>
      <td style="text-align:center;">${actionHtml}</td>
    `;
    tbody.appendChild(tr);
  });
}

// Process vehicle return request
async function handleReturnVehicle(orderId, vehName) {
  if (!confirm(`Are you sure you want to confirm returning the vehicle "${vehName}" (Order #${orderId})?`)) {
    return;
  }

  const res = await api.post('/api/rentals/return', {
    orderId: orderId,
    customerId: currentUser.userId
  });

  if (res.success) {
    alert(res.message);
    loadRentalHistory();
    loadVehicles(); 
    loadNotifications();
  } else {
    alert(res.message || 'Error occurred while returning the vehicle!');
  }
}

// Fetch and display customer notifications
async function loadNotifications() {
  const res = await api.get(`/api/notifications?customerId=${currentUser.userId}`);
  const badge = document.getElementById('notifBadge');
  const container = document.getElementById('notifListContainer');

  if (!res.success || !res.data || res.data.length === 0) {
    if (badge) badge.style.display = 'none';
    if (container) {
      container.innerHTML = '<div style="text-align:center; padding:1.5rem; color:var(--gray);">No notifications yet.</div>';
    }
    return;
  }

  const unreadCount = res.data.filter(n => !n.isRead && n.is_read != 1).length;
  if (badge) {
    if (unreadCount > 0) {
      badge.textContent = unreadCount;
      badge.style.display = 'flex';
    } else {
      badge.style.display = 'none';
    }
  }

  if (container) {
    container.innerHTML = '';
    res.data.forEach(n => {
      const card = document.createElement('div');
      card.className = 'notifCard';
      const isUnread = (!n.isRead && n.is_read != 1);
      card.innerHTML = `
        <div class="notifHeader">
          <span>Notification #${n.id || n.notification} (Order #${n.orderId || n.orderID || 'N/A'})</span>
          <span>${isUnread ? '<span style="color:#ef4444;">● New</span>' : 'Read'}</span>
        </div>
        <div>${n.message}</div>
      `;
      container.appendChild(card);
    });
  }
}

// Open notification modal
function openNotifModal() {
  document.getElementById('notifModal').classList.add('active');
  api.post('/api/notifications/mark-read', { customerId: currentUser.userId }).then(() => {
    const badge = document.getElementById('notifBadge');
    if (badge) badge.style.display = 'none';
  });
}

// Close notification modal
function closeNotifModal() {
  document.getElementById('notifModal').classList.remove('active');
}

// Dismiss modal when clicking outside content area
function handleCustomerOverlay(event, modalId) {
  if (event.target.id === modalId) {
    document.getElementById(modalId).classList.remove('active');
  }
}

// Escape key listener to close modals
document.addEventListener('keydown', (event) => {
  if (event.key === 'Escape') {
    closeRentModal();
    closeNotifModal();
  }
});
